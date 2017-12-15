# runit test setup
# This file is *not* part of the JuniperKernel package
# NB: the actual set of runits are in inst/unitTests

# load RUnit and JuniperKernel packages
stopifnot(require(RUnit, quietly=TRUE))
stopifnot(require(JuniperKernel, quietly=TRUE))
stopifnot(require(subprocess, quietly=TRUE))

# SETUP
set.seed(42)

tryCatch(
  {
    # start a new jupyter client
    jupyterTestClient <- JuniperKernel::run_client()
    # write the connection file
    connInfo <- JuniperKernel::client_info(jupyterTestClient)
    tmpPath <- tempfile()
    dir.create(tmpPath, recursive=TRUE)
    connectionFile <- file.path(tmpPath, "connection_file.json")
    fc <- file(connectionFile)
    writeLines(connInfo, fc)
    close(fc)

    rbin <- normalizePath(Sys.which("R"))
    args <- paste( "--slave"
                 , "-e"
                 , "JuniperKernel::bootKernel()"
                 , "--args"
                 , connectionFile
                 )
    handle <- subprocess::spawn_process(rbin, args)





    # DEFINE TEST SUITE
    testSuite <- defineTestSuite(
    name="rbql unit tests",
    dirs=system.file("unitTests", package = "rbql"),
    #testFileRegexp = "^runit\\.rbql\\.help\\.R$",  # use this to test just a single group
    testFuncRegexp = "^[Tt]est.+")

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
      JuniperKernel::stop_client(jupyterTestClient)

    }
)