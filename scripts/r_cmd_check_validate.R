check_notes_helper <- function(details) {
  if( !is(details, "data.frame") )
  stop("expected the notes details to be a data.frame")

  whitelist =
    list(
      "CRAN incoming feasibility" =
        c( "Maintainer:"
         , "Checking URLs requires 'libcurl' support in the R build"
         , "Days since last update"
         , "Number of updates in past 6 months"
         , "New submission"
         )

     , "package dependencies" =
        c( "No repository set, so cyclic dependency check skipped"
         , "Package suggested but not available for checking"
         )

     , "installed package size" =
        c( "installed size is .*Mb"  # libs/include exceed some threshold
         , "sub-directories of 1Mb or more"
         , "include .*Mb"
         , "libs .*Mb"
         , "zmq .*Mb"
         )
     )
  e <- new.env(parent=emptyenv())  # holds a bit for pass/fail checks in the *ply calls
  e$status <- TRUE
  lapply(1:nrow(details), function(rowid, e) {
    row <- details[rowid,]
    check <- as.character(row$Check)
    checklist <- whitelist[[check]]
    if( is.null(checklist) ) {
      message("NOTE not whitelisted: ", check)
      message(row$Output)
      e$status <- FALSE
    } else {
      # loop over the lines of the Output and compare them to checklist
      lapply(strsplit(row$Output, '\n')[[1L]], function(line, e, checklist) {
        grepped <- sum(sapply(checklist, function(pattern) grepl(pattern,line)))
        if( nzchar(line) && !grepped ) {
          message("NOTE not whitelisted: ", check)
          message(row$Output)
          e$status <- FALSE
        }
      }, e=e, checklist=checklist)
    }
  }, e=e)
  e$status
}

# This script checks the output of a R CMD CHECK run
# This is run as part of the CI suite of tests for JuniperKernel.
#
# This step performs the following steps:
#   1. Read in the file JuniperKernel.Rcheck/00check.log
#   2. For each "check" (line beginning with "* chceking"), grep for status
#   3. The status may be one of [NOTE, WARNING, ERROR, OK]
#   4. Sometimes NOTE is ok, so check it against a list of allowed notes
#   5. WARNING status anywhere is an invalid R CMD CHECK
#   6. If R CMD CHECK has a status of FAIL ERROR or WARN, then it is invalid
#
# Lines look like this:
#     * checking R files for non-ASCII characters ... WARNING
#     * checking for code/documentation mismatches ... OK
#
#   On Windows, i386 and x64 architectures are checked:
#     * loading checks for arch 'i386'
#     ** checking whether the package can be loaded ... OK
#     ...
#     * loading checks for arch 'x64'
#     ** checking whether the package can be loaded ... OK
#     ...
#
# Luckily, the `tools` package has some handy unexported functionality
# that is used to preprocess this log file!
(function() {
  path <- file.path('.', 'JuniperKernel.Rcheck', '00check.log')
  if( !file.exists(path) )
  stop("00check.log not found.")

  chkResults <- tools:::check_packages_in_dir_results(logs=path)
  if( is.null(chkResults) || length(chkResults)==0L )
  stop("Invalid 00check.log")

  status <- chkResults$JuniperKernel$status
  if( status %in% c("FAIL", "ERROR", "WARN") ) {
    message(readLines(path, warn=FALSE), collapse='\n')
    stop("R CMD CHECK completed with status ", status, ".\n")
  }

  # check all notes
  if( status=="NOTE" ) {
    details <- tools:::check_packages_in_dir_details(logs=path)
    if( !check_notes_helper(details) )
    stop("NOTEs discovered in output that need further attention.", call.=FALSE)
    status <- "OK"
  }
  stopifnot(status=="OK")
  print("R CMD CHECK PASS")
})()
