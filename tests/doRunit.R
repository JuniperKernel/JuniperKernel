# runit test setup
# This file is *not* part of the JuniperKernel package
# NB: the actual set of runits are in inst/runits

# load RUnit and JuniperKernel packages
stopifnot(require(RUnit, quietly=TRUE))
stopifnot(require(JuniperKernel, quietly=TRUE))
stopifnot(require(subprocess, quietly=TRUE))
source("launch_kernel.R")
source("check_iopub.R")

# SETUP
ports <- floor((5+runif(5))*10000)  # randomize ports before seed is set
ports <- list(hbport=ports[1L], ioport=ports[2L], shport=ports[3L], ctport=ports[4L], inport=ports[5L])

set.seed(42)

test.env <- new.env(parent=emptyenv())
kernelProc <- launch_kernel(ports)
jclient <- JuniperKernel:::run_client(ports$hbport, ports$ioport, ports$shport, ports$ctport, ports$inport)

JuniperKernel:::wait_for_hb(jclient)
println("CLIENT<=>KERNEL connection established")

tryCatch(
  {
    # DEFINE TEST SUITE
    testSuite <- defineTestSuite( name="JuniperKernel unit tests"
                                , dirs="/Users/saiello8/repos/JuniperKernel/inst/runits"  # system.file("runits", package = "JuniperKernel")
                                , testFuncRegexp = "^[Tt]est.+"
                                )

    # RUN TESTS
    tests <- runTestSuite(testSuite)

    # PRINT RESULTS
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
      # attempt a kernel shutdown
      tryCatch(println(process_terminate(kernelProc)), error=function(.){})
      # tryCatch(process_kill(kernelProc), error=function(.){})
      tryCatch(print(kernelProc), error=function(.){})
    }
)