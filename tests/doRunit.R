# runit test setup
# This file is *not* part of the JuniperKernel package
# NB: the actual set of runits are in inst/runits

# load RUnit and JuniperKernel packages
stopifnot(require(RUnit, quietly=TRUE))
stopifnot(require(JuniperKernel, quietly=TRUE))
stopifnot(require(subprocess, quietly=TRUE))
library(JuniperKernel)

# SETUP
set.seed(42)

test.env <- new.env(parent=emptyenv())
tryCatch(
  {
    # write the connection file

    connInfo <- list(
      "control_port"    = 53959,
      "hb_port"         = 53960,
      "iopub_port"      = 53961,
      "ip"              = "127.0.0.1",
      "key"             = "cc496d37-59a9-4c61-8900-d826985f564d",
      "shell_port"      = 53957,
      "signature_scheme"= "hmac-sha256",
      "stdin_port"      = 53958,
      "transport"       = "tcp")
    connInfo <- jsonlite:::toJSON(connInfo, pretty=TRUE, auto_unbox=TRUE)

    tmpPath <- tempfile()
    dir.create(tmpPath, recursive=TRUE)
    connectionFile <- file.path(tmpPath, "connection_file.json")
    print(paste0("CONNECTION_INFO: ", connInfo))
    fc <- file(connectionFile)
    writeLines(connInfo, fc)
    close(fc)
    print("connection file written")

    rbin <- normalizePath(Sys.which("R"))
    args <- paste( "--slave"
                 , "-e"
                 , "JuniperKernel::bootKernel()"
                 , "--args"
                 , connectionFile
                 )
    test.env$juniperKernelProc <- jkp <- subprocess::spawn_process(rbin, args)
    if( subprocess::process_state(jkp)!="running" ) {
      txt <- paste0( "Process did not correctly start.\n"
                   , "  Exit code:", subprocess::process_return_code(jkp), "\n"
                   , "  stdout: \n", paste0(subprocess::process_read(jkp)$stdout, collapse="\n"), "\n"
                   , "  stderr: \n", paste0(subprocess::process_read(jkp)$stderr, collapse="\n"), "\n"
                   , "  proc info: \n", paste0(jkp, collapse="\n")
                   )
      stop(txt)
    }
    while( TRUE ) {
      outerr <- paste0(subprocess::process_read(jkp), collapse='\n')
      if( outerr!="" ) {
        print(outerr)
        break
      }
    }
    print("process is running...")
    print(paste0(jkp, collapse="\n"))
    Sys.sleep(10)


    print(paste0(subprocess::process_read(jkp), collapse='\n'))



    # start a new jupyter client
    jtc <- JuniperKernel:::run_client()
    print("test client started")

    payload <- '{"content":{},"header":{"date":"2017-12-17T00:30:33.180360Z","msg_id":"1a7186aa-1d2fcbab990a326ebb6747d4","msg_type":"kernel_info_request","session":"944E351AF3AD44B78054DF4DAB6FA0E5","username":"spencer","version":"5.2"},"metadata":{},"parent_header":{}}'
    kinfo <- JuniperKernel:::client_exec_request(jtc, payload)

    print(kinfo)

    Sys.sleep(2)
    outerr <- paste0(subprocess::process_read(jkp), collapse='\n')


    # DEFINE TEST SUITE
    testSuite <- defineTestSuite( name="JuniperKernel unit tests"
                                , dirs=system.file("runits", package = "JuniperKernel")
                                , testFuncRegexp = "^[Tt]est.+"
                                )
    print(paste0(subprocess::process_read(jkp), collapse='\n'))


    Sys.sleep(10)
    print(paste0(subprocess::process_read(jkp), collapse='\n'))
    JuniperKernel:::.setGlobal("JTC", jtc, 1L)

    # RUN TESTS
    tests <- runTestSuite(testSuite)

    # PRINT RESULTS
    printTextProtocol(tests)

    # Return success or failure to R CMD CHECK
    errs <- getErrors(tests)
    if( errs$nFail     > 0 ) stop("TEST FAILED!")
    if( errs$nErr      > 0 ) stop("TEST HAD ERRORS!")
    if( errs$nTestFunc < 1 ) stop("NO TEST FUNCTIONS RUN!")

    print("ALL TESTS PASSED")
  }
  , error = function(e) {
      print("ERROR")
      print(e)
    }
  , finally = {
      jtc <- NULL
      gc()
      print(paste0(subprocess::process_read(jkp), collapse='\n'))
      print(subprocess::process_terminate(jkp))
      print(jkp)
    }
)
