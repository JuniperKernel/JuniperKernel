# global package environment
.jkern <- new.env(parent=emptyenv())

.home <- function() {
  p <- path.expand('~')
  if( .Platform$OS!='unix' )
  p <- dirname(p)
  p
}

# get the jupyter config path
.config <- function() {
  if( !is.na(Sys.getenv()['JUPYTER_CONFIG_DIR']) )
  return(Sys.getenv()['JUPYTER_CONFIG_DIR'])
  return(file.path(.home(), '.jupyter'))
}

# Get the Jupyter data files config directory. Will use the
# environment variable JUPYTER_DATA_DIR if defined.
.data <- function() {
  if( !is.na(Sys.getenv()['JUPYTER_DATA_DIR']) )
  return(Sys.getenv()['JUPYTER_DATA_DIR'])

  sysname <- Sys.info()['sysname'][[1L]]

  if( sysname == 'Darwin' )
  return(file.path(.home(), 'Library', 'Jupyter'))

  if( sysname == 'NT' ) {
    appdata <- Sys.getenv()['APPDATA']
    if( !is.na(appdata) )
    return(file.path(appdata, 'jupyter'))
    return(file.path(.config(), 'data'))
  }
  xdg <- Sys.getenv()['XDG_DATA_HOME']
  if( is.na(xdg) )
  xdg <- file.path(.home(), '.local', 'share')
  file.path(xdg, 'jupyter')
}



.onAttach <- function(pkg, lib) {}
.onLoad   <- function(pkg, lib) {

  # setup system JUPYTER/CONFIG paths that are used throughout
  sysname <- Sys.info()["sysname"][[1]]
  if( sysname!='Windows' ) {
    if( sysname=='Darwin') {
      jupyterpath <- file.path(.home(), 'Library', 'Jupyter')
    } else {
      jupyterpath <- "/usr/local/share/jupyter"
    }
    assign('SYSTEM_JUPYTER_PATH', c(jupyterpath, "/usr/share/jupyter"), envir=.jkern)
    assign('SYSTEM_CONFIG_PATH', c("/usr/local/etc/jupyter", "/etc/jupyter"), envir=.jkern)
  } else {
    programdata <- Sys.getenv()['PROGRAMDATA']
    if( !is.na(programdata) ) {
      assign('SYSTEM_JUPYTER_PATH', file.path(programdata, 'jupyter'), envir=.jkern)
      assign('SYSTEM_CONFIG_PATH', file.path(programdata, 'jupyter'), envir=.jkern)
    } else {
      assign('SYSTEM_JUPYTER_PATH', file.path(.libPaths()[1L], 'JuniperKernel', 'jupyter'), envir=.jkern)
      assign('SYSTEM_CONFIG_PATH', NULL, envir=.jkern)
    }
  }
  assign('ENV_JUPYTER_PATH', file.path(.libPaths()[1L], 'JuniperKernel', 'jupyter'), envir=.jkern)
  assign('ENV_CONFIG_PATH' , file.path(.libPaths()[1L], 'JuniperKernel', 'etc', 'jupyter'), envir=.jkern)
}