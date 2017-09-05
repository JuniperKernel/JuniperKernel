# These methods build up the "content" that will be packaged
# in the corresponding reply. The full reply is managed by the
# c++ core kernel code.

kernel_info_request <- function(request_msg) {
  list( msg_type = "kernel_info_reply"
      , content = list( protocol_version = "5.2"
                      , implementation   = "JuniperKernel"
                      , implementation_version = as.character(packageVersion("JuniperKernel"))
                      , language_info = list ( name = "R"
                                             , codemirror_mode = "r"
                                             , pygments_lexer = "r"
                                             , mimetype = "text/x-r-source"
                                             , file_extension = ".r"
                                             , version = paste(version$major, version$minor, sep=".")
                                             )
                      , banner = version$version.string
    , foo = TRUE, bazboo = c(TRUE, FALSE, FALSE, TRUE), kazoo=343.12312, kazoo2=c(314.1231, 341.231,123.2321, pi)
                      )
      )
}

execute_request <- function(request_msg) {

}
