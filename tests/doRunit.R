# Copyright (C) 2017  Spencer Aiello
#
# This file is part of JuniperKernel.
#
# JuniperKernel is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# JuniperKernel is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with JuniperKernel.  If not, see <http://www.gnu.org/licenses/>.

# JuniperKernel RUnit Utilities
# Utility: print function
println <- function(...) {
  cat(paste(..., "\n"))
}

# Utility: kernel launcher in forked process
launch_kernel <- function(ports) {
  require(subprocess)
  connectionFile <- write_connection_file(ports)
  rbin <- normalizePath(Sys.which("R"))
  args <- paste( "--slave"
  , "-e"
  , "JuniperKernel::bootKernel()"
  , "--args"
  , connectionFile
  )
  kernelProc <- spawn_process(rbin, args)
  if( subprocess::process_state(kernelProc)!="running" ) {
    stdouterr <- process_read(kernelProc)
    txt <- paste0( "Process did not correctly start.\n"
    , "  Exit code:", process_return_code(kernelProc), "\n"
    , "  stdout: \n", paste0(stdouterr$stdout, collapse="\n"), "\n"
    , "  stderr: \n", paste0(stdouterr$stderr, collapse="\n"), "\n"
    , "  proc info: \n", paste0(kernelProc, collapse="\n")
    )
    stop(txt)
  }
  kernelProc
}

write_connection_file <- function(ports) {
  connInfo <- list( "control_port"    = ports$ctport
  , "hb_port"         = ports$hbport
  , "iopub_port"      = ports$ioport
  , "ip"              = "127.0.0.1"
  , "key"             = "cc496d37-59a9-4c61-8900-d826985f564d"
  , "shell_port"      = ports$shport
  , "signature_scheme"= "hmac-sha256"
  , "stdin_port"      = ports$inport
  , "transport"       = "tcp"
  )
  connInfo <- jsonlite:::toJSON(connInfo, pretty=TRUE, auto_unbox=TRUE)
  tmpPath <- tempfile()
  dir.create(tmpPath, recursive=TRUE)
  connectionFile <- file.path(tmpPath, "connection_file.json")
  println("CONNECTION_INFO: ", connInfo)
  fc <- file(connectionFile)
  writeLines(connInfo, fc)
  close(fc)
  println("connection file written")
  connectionFile
}

# Check that the 'busy' and 'idle' messages are published
# Also collect intermediate messages and return them
# in a sensible structure for individual test checks.
check_iopub_messages <- function(msg_type) {
  msgs <- recv_msgs()
  checkTrue(length(msgs)!=0L)

  busy <- jsonlite::fromJSON(msgs[1L])
  idle <- jsonlite::fromJSON(msgs[length(msgs)])

  if(busy$content$execution_state!="busy") {
    println("ACTUAL BUSY MESSAGE IS")
    print(busy)
    print(msgs)
  }

  checkTrue(busy$content$execution_state=="busy")
  checkTrue(busy$parent_header$msg_type==msg_type)

  if(idle$content$execution_state!="idle") {
    println("ACTUAL IDLE MESSAGE IS")
    print(idle)
    print(msgs)
  }
  checkTrue(idle$content$execution_state=="idle")
  checkTrue(idle$parent_header$msg_type==msg_type)

  invisible(msgs)
}

recv_msgs <- function() {
  msgs <- c()

  # spin until messages are available from iopub
  # timeout after ~5 seconds if no messages published...
  ntries <- 30L
  while( ntries >= 0L && (msg <- JuniperKernel:::iopub_recv(jclient))=="" ) {
    if( !ntries%%6L ) Sys.sleep(1L)  # sleep for 1 second every 6 tries
    ntries <- ntries - 1L
  }

  # if we didn't get any message, don't count it
  if( nzchar(msg) )
  msgs <- c(msgs, msg)

  # Optimistically poll for the idle message. iopub_recv races with all
  # message writers, so intermittently the queue will be empty and produce
  # a read resulting in "". Allow for ~3 seconds of polling if no idle is
  # reached so that we don't gum up the test suite.
  # Note that even if the previous poll produced no messages read, then
  # this loop tries One More Time. And, on each non-zero length message
  # the retry mechanism is reset.
  ntries <- 30L
  notIdle <- TRUE
  while( notIdle ) {
    if( ntries <= 0L ) break
    msg <- JuniperKernel:::iopub_recv(jclient)
    if( nzchar(msg) ) {
      ntries <- 30L  # reset
      msgs <- c(msgs, msg)
      # check if we got the idle message (which is the last message)
      jmsg <- jsonlite::fromJSON(msg)
      if( !is.null(jmsg$content$execution_state) &&
      jmsg$content$execution_state=="idle" ) {
        notIdle <- FALSE
      }
    } else {
      # Poll the iopub message queue until the "idle" message arrives.
      # we are racily reading messages from the iopub queue whilst writers
      # are in progress. An empty queue with no "idle" message published may
      # mean more messages are yet to come, or a more serious problem.
      # If no messages come after `ntries` attempts, then bail on this and
      # return what we have so far.
      #
      # Note that if we fail to recv the idle message, then all downstream
      # tests will be broken!
      ntries <- ntries - 1L
      println("waiting for idle...")
      Sys.sleep(.10)
    }
  }
  msgs
}

# perform a "safe" kernel shutdown
kernel_shutdown <- function() {
  payload <- '{"content":{"restart":false},"header":{"date":"2017-12-23T00:43:53.209162Z","msg_id":"fe404469-709bb2c562e92af8d5c2a9cc","msg_type":"shutdown_request","session":"76159402-4d8ca86da95021a5cc2a4fd3","username":"username","version":"5.3"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))

  if( is.null(reply) || is.null(reply$header) || is.null(reply$header$msg_type) ) {
    message("could not shutdown kernel")
    return(FALSE)
  }

  if( reply$header$msg_type!="shutdown_reply" ) {
    message(paste0("expected `shutdown_reply`, but got ", reply$header$msg_type))
    return(FALSE)
  }
  TRUE
}
################################################################################################
################################################################################################
################################################################################################

# runit test setup
# This file is *not* part of the JuniperKernel package
# NB: the actual set of runits are in inst/runits

# load RUnit and JuniperKernel packages
stopifnot(require(RUnit, quietly=TRUE))
stopifnot(require(subprocess, quietly=TRUE))
stopifnot(require(jsonlite, quietly=TRUE))
stopifnot(require(JuniperKernel, quietly=TRUE))

println("=======================R UNIT SETUP=======================")
# SETUP
ports <- floor((5+runif(5))*10000)  # randomize ports before seed is set
ports <- list(hbport=ports[1L], ioport=ports[2L], shport=ports[3L], ctport=ports[4L], inport=ports[5L])

set.seed(42)

test.env <- new.env(parent=emptyenv())
kernelProc <- launch_kernel(ports)
jclient <- JuniperKernel:::run_client(ports$hbport, ports$ioport, ports$shport, ports$ctport, ports$inport)

JuniperKernel:::wait_for_hb(jclient)
println("CLIENT<=>KERNEL connection established")
println("=======================R UNIT SETUP DONE=======================")

println("")
println("")

tryCatch(
  {
    # DEFINE TEST SUITE
    testSuite <- defineTestSuite( name="JuniperKernel unit tests"
                                , dirs=system.file("runits", package = "JuniperKernel")
                                , testFuncRegexp = "^[Tt]est.+"
                                )

    # RUN TESTS
    println("=======================R UNIT TEST SUITE BEGIN=======================")
    println("")
    println("")
    tests <- runTestSuite(testSuite)


    # PRINT RESULTS
    println("=======================R UNIT TEST SUITE END=======================")
    println("")
    println("")

    printTextProtocol(tests)

    # Return success or failure to R CMD CHECK
    errs <- getErrors(tests)
    if( errs$nFail     > 0L ) stop("TEST FAILED!")
    if( errs$nErr      > 0L ) stop("TEST HAD ERRORS!")
    if( errs$nTestFunc < 1L ) stop("NO TEST FUNCTIONS RUN!")

    print("ALL TESTS PASSED")
  }
  , error = function(e) {
      println("ERROR")
      println(e)
    }
  , finally = {
      println("")
      println("")
      println("=======================R UNIT CLEANUP=======================")
      shutdown <- kernel_shutdown()
      if( !shutdown ) {
        println("Failed to shutdown...")
        tryCatch(println(process_terminate(kernelProc)), error=function(.){})
      }
      print("...proc wait...")
      Sys.sleep(3)      
      process_wait(kernelProc, timeout=3000L)  # timeout for the process to clean it self up after 3s
      tryCatch(print(kernelProc), error=function(.){})
      stopifnot(process_return_code(kernelProc)==0L)
      print("Successful kernel shutdown")
    }
)
