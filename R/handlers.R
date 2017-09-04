kernel_info_request <- function() {
  list( protocol_version = "5.2"
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
      )
}