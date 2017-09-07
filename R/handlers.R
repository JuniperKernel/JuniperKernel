# These methods build up the "content" that will be packaged
# in the corresponding reply. The full reply is managed by the
# c++ core kernel code.

doRequest <- function(request_msg, handler) {
  con <- socketConnection(server=TRUE, port=6011)
  sink(con)
  tryCatch(
   return(handler(request_msg))
  , finally={
      sink()
      close(con)
    }
  )
}


kernel_info_request <- function(request_msg) {
  list( msg_type = "kernel_info_reply"
      , content = list( protocol_version = "5.2"
                      , implementation   = "JuniperKernel"
                      , implementation_version = as.character(packageVersion("JuniperKernel"))
                      , language_info = list ( name = "R"
                                             , codemirror_mode = "r"
                                             , pygments_lexer = "r"
                                             , mimetype = "text/x-r-source"
                                             , file_extension = ".R"
                                             , version = paste(version$major, version$minor, sep=".")
                                             )
                      , banner = version$version.string
                      )
      )
}

execute_request <- function(request_msg) {
  content <- request_msg$content
  rebroadcast_input(.kernel(), content$code, cnt <- .exeCnt())

  exprs <- parse(text=content$code)
  list( msg_type = "execute_reply"
      , content = list(status="ok", execution_count = 1)
  )
}
