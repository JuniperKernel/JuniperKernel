# perform a "safe" kernel shutdown
kernel_shutdown <- function() {
  payload <- '{"content":{"restart":false},"header":{"date":"2017-12-23T00:43:53.209162Z","msg_id":"fe404469-709bb2c562e92af8d5c2a9cc","msg_type":"shutdown_request","session":"76159402-4d8ca86da95021a5cc2a4fd3","username":"username","version":"5.3"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))

  if( is.null(reply) || is.null(reply$header) || is.null(reply$header$msg_type) ) {
    message("could not shutdown kernel")
    return(FALSE)
  }

  if( reply$header$msg_type!="shutdown_reply" ) {
    message(paste0("expected `shutdown_reply`, but got ", reply$header$msg_type))
    return(FALSE)
  }
  TRUE
}