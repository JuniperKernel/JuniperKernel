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
    # start a new jupyter client
    jtc <- JuniperKernel:::run_client()
    print("test client started")
    # write the connection file
    connInfo <- JuniperKernel:::client_info(jtc)
    tmpPath <- tempfile()
    dir.create(tmpPath, recursive=TRUE)
    test.env$connectionFile <- connectionFile <- file.path(tmpPath, "connection_file.json")
    test.env$fc <- fc <- file(connectionFile)
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
    unlink(connectionFile)
    if( subprocess::process_state(jkp)!="running" ) {
      txt <- paste0( "Process did not correctly start.\n"
                   , "  Exit code:", subprocess::process_return_code(jkp), "\n"
                   , "  stdout: \n", paste0(subprocess::process_read(jkp)$stdout, collapse="\n"), "\n"
                   , "  stderr: \n", paste0(subprocess::process_read(jkp)$stderr, collapse="\n"), "\n"
                   , "  proc info: \n", paste0(jkp, collapse="\n")
                   )
      stop(txt)
    }


    # DEFINE TEST SUITE
    testSuite <- defineTestSuite( name="JuniperKernel unit tests"
                                , dirs=system.file("runits", package = "JuniperKernel")
                                , testFuncRegexp = "^[Tt]est.+"
                                )

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
  , finally = function() {
      if( subprocess::process_state(test.env$juniperKernelProc)=="running" ) {
        tryCatch(if(!subprocess::process_terminate(test.env$juniperKernelProc)) stop("proc terminate did not return TRUE"), error=function(e) print("Could not terminate the process", e))
      }
      tryCatch(unlink(test.env$connectionFile), error=function(e) stop("could not delete temp connection file: ", test.env$connectionFile))
      JuniperKernel:::.JUNIPER$jupyterTestClient <- NULL
      gc()
    }
)
