## Juniper Kernel Installation
##
## Requires jupyter and installs the kernel via
## `jupyter kernelspec install`.

.argv <- function() {
  exec <- file.path(R.home('bin'), 'R')
  c(exec, '--slave', '-e', 'JuniperKernel::launch()', '--args', '{connection_file}')
}

.stopIfJupyterMissing <- function() {
  rc <- system2("jupyter", c("kernelspec", "--version"), FALSE, FALSE)
  if( rc )
    stop("jupyter not found; ensure jupyter is installed.")
}

.writeSpec <- function(displayName) {
  tmpPath <- tempfile()
  kernelspec <- file.path(tmpPath, 'kernelspec')
  dir.create(kernelspec, recursive=TRUE)
  specPath <- file.path(kernelspec, "kernel.json")
  spec <- list( argv = .argv()
              , display_name = displayName
              , language = 'R'
              )
  fc <- file(specPath)
  writeLines(jsonlite::toJSON(spec, pretty=TRUE, auto_unbox=TRUE), specPath)
  close(fc)
  tmpPath
}

.installSpec <- function(user, prefix, kernelName, tmpPath) {
  kernelspec <- file.path(tmpPath, 'kernelspec')
  args <- c( "kernelspec"
           , "install"
           , "--replace"
           , "--name"
           , kernelName
           # --user flag
           , ifelse(user, '--user', '')
           # --prefix prefix
           , ifelse(nzchar(prefix), '--prefix', '')
           , ifelse(nzchar(prefix), prefix, '')
           # path to kernelspec
           , kernelspec
           )
  rc <- system2("jupyter", args)
  unlink(tmpPath, recursive = TRUE)
  rc
}

#' List Kernels
#'
#' @title List Installed Jupyter Kernels
#' @details
#' Prints the currently installed kernels and their install locations.
#' @export
listKernels <- function() {
  .stopIfJupyterMissing()
  invisible(system2('jupyter', c('kernelspec', 'list')))
}

#' Create Kernel Name
#'
#' @title Create the Default Kernel Name
#' @seealso \code{\link{installJuniper}}.
#' @export
kernelName <- function() {
  s <- utils::sessionInfo()
  paste0('juniper_r', s$R.version$major, '.', s$R.version$minor)
}

#' Create Kernel Display Name
#'
#' @title Create the Default Kernel Display Name
#' @seealso \code{\link{installJuniper}}.
#' @export
displayName <- function() {
  s <- utils::sessionInfo()
  paste0('R ', s$R.version$major, '.', s$R.version$minor, ' (Juniper)')
}

#' Install Juniper Kernel
#'
#' @title Install the Juniper Kernel for Jupyter
#'
#' @details
#' Use this method to install the Juniper Kernel. After a successful invocation
#' of this method, Juniper will be an available kernel for all Jupyter front-end
#' clients (e.g., the dropdown selector in the Notebook interface). This method
#' is essentially a wrapper on the function call \code{jupyter kernelspec install}
#' with some extra configuration options. These options are detailed as the parameters
#' below. One important note to make is that the kernel will depend the \code{R} environment
#' that doing the invoking. In this way a user may install kernels for different versions
#' of R by invoking this \code{installJuniper} method from each respective R. The defaults
#' for \code{kernelName} and \code{displayName} are good for avoiding namespacing issues
#' between versions of R, but installs having the same kernel name replace an existing
#' kernel.
#'
#' @param user
#' If \code{TRUE}, install the kernel in a user-local fashion. For macOS,
#' the user-local directory is \code{~/Library/Jupyter/kernels}. For Windows, the
#' user-local directory is \code{\%APPDATA\%\\jupyter\\kernels}. For Linux this directory is
#' \code{~/.local/share/jupyter/kernels}.
#' If \code{FALSE}, the kernel is installed system-wide. For unix-based machines,
#' the system-level directory is \code{/usr/share/jupyter/kernels} or
#' \code{/usr/local/share/jupyter/kernels}. For Windows, the location is
#' \code{\%PROGRAMDATA\%\\jupyter\\kernels}.
#' If the \code{prefix} argument is specified, then the \code{user} parameter is ignored.
#'
#' @param kernelName
#' A character string representing the location of the kernel. This is required
#' to be made up of alphanumeric and \code{.}, \code{_}, \code{-} characters only.
#' This is enforced with a check against this \code{^[a-zA-Z_][a-zA-Z0-9_.-]*$} regex.
#' The case of this argument is always ignored and is \code{tolower}ed; so while it's
#' allowed to have mixed-case characters, the resulting location will not be. A warning
#' will be issued if there is mixed-case characters. The default for this
#' \code{juniper_r} concatenated with the \code{major.minor} version of R. For example,
#' for R 3.4.0, the default would be \code{juniper_r3.4.0}.
#'
#' @param displayName
#' A character string representing the name of the kernel in a client. There are no
#' restrictions on the display name. The default for R 3.4.0 is \code{R 3.4.0 (Juniper)}.
#'
#' @param prefix
#' A character string specifying the \code{virtual env} that this kernel should be installed
#' to. The install location will be \code{prefix/share/jupyter/kernels}.
#'
#' @examples
#' \dontrun{
#'   installJuniper()  # install into user-local directory
#' }
#' @export
installJuniper <- function (user = TRUE, kernelName = kernelName(), displayName = displayName(), prefix='') {
  .stopIfJupyterMissing()

  if( regexpr("^[a-zA-Z_][a-zA-Z0-9_.-]*$", kernelName)[1L] == -1L )
    stop("`kernelName` must match the regex ^[a-zA-Z_][a-zA-Z0-9_.-]*$")

  name <- tolower(kernelName)
  if( name!=kernelName )
    warning("Mixed case characters are ignored: ", kernelName, " -> ", name)

  # write the kernels.json file
  tmpPath <- .writeSpec(displayName)

  if( user && nzchar(prefix) ) {
    warning("`user` and `prefix` specifed. `user` is ignored.")
    user <- FALSE
  }

  # install the kernel for jupyter to see
  invisible(.installSpec(user, prefix, name, tmpPath))
}