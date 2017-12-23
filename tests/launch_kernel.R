println <- function(...) {
  cat(paste(..., "\n"))
}

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