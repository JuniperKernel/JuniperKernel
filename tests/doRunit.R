# runit test setup
# This file is *not* part of the JuniperKernel package
# NB: the actual set of runits are in inst/runits

# load RUnit and JuniperKernel packages
stopifnot(require(RUnit, quietly=TRUE))
stopifnot(require(subprocess, quietly=TRUE))
stopifnot(require(jsonlite, quietly=TRUE))
stopifnot(require(JuniperKernel, quietly=TRUE))

source("launch_kernel.R")
source("check_iopub.R")
source("shutdown.R")

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
                                , dirs=system.file("runits", package = "JuniperKernel")  # "C:/Users/spencer/repos/JuniperKernel/inst/runits" #system.file("runits", package = "JuniperKernel")  # "C:/Users/spencer/repos/JuniperKernel/inst/runits"
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
