
# Creates a name like "R 3.4.0"
.kernelName <- function() {
  s <- sessionInfo()
  paste0('juniper_R', s$R.version$major, '.', s$R.version$minor)
}

.displayName <- function() {
  s <- sessionInfo()
  paste0('R ', s$R.version$major, '.', s$R.version$minor, ' (Juniper)')
}

.argv <- function(extra.args=NULL) {
  exec <- file.path(R.home('bin'), 'R')
  argv <- c(exec, '--slave', '-e', 'JuniperKernel::launch()', '--args', '{connection_file}')
  if( !is.null(extra.args) )
    argv <- c(argv, extra.args)
  argv
}

.chkJupyter <- function() {
  if( rc <- system2("jupyter", c("kernelspec", "--version"), FALSE, FALSE) )
  stop("jupyter has to be installed but `jupyter kernelspec --version` exited with code ", rc, ".\n")
}

.writeSpec <- function(displayName, overrides, extra.args) {
  tmpPath <- tempfile()
  kernelspec <- file.path(tmpPath, 'kernelspec')
  dir.create(kernelspec, recursive=TRUE)
  specPath <- file.path(kernelspec, "kernel.json")
  spec <- list( argv = .argv(extra.args)
              , display_name = displayName
              , language = 'R'
              )
  spec <- modifyList(spec, overrides)  # update the spec with any overrides
  fc <- file(specPath)
  writeLines(jsonlite::toJSON(spec, pretty=TRUE, auto_unbox=TRUE), specPath)
  close(fc)
  tmpPath
}

.installSpec <- function(user, kernelName, tmpPath) {
  kernelspec <- file.path(tmpPath, 'kernelspec')
  args <- c( "kernelspec"
           , "install"
           , "--replace"
           , "--name"
           , kernelName
           , ifelse(user, '--user', '')
           , kernelspec
           )
  rc <- system2("jupyter", args)
  unlink(tmpPath, recursive = TRUE)
  rc
}

#' Install Juniper Kernel
#'
#' Install the Juniper Kernel for Jupyter
installJuniper <- function (user = TRUE, kernelName = .kernelName(), displayName = .displayName(), profile = NULL) {
  .chkJupyter()

  # arg overrides
  if( kernelName!=.kernelName() && is.null(displayName) )
    displayName <- kernelName
  overrides = list()
  extra.args <- NULL
  if( !is.null(displayName) )
    overrides$display_name <- displayName
  if( !is.null(profile) ) {
    extra.args <- c("--profile", profile)
    if( is.null(displayName) )
    overrides$display_name <- paste0(kernelName, ' (profile=', profile, ')')
  }

  # write the kernels.json file
  tmpPath <- .writeSpec(displayName, overrides, extra.args)

  # install the kernel for jupyter to see
  invisible(.installSpec(user, kernelName, tmpPath))
}