#' Response to the \code{kernel_info_request} Message Type
#'
#' @title Kernel Info Request Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{kernel_info_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{kernel_info_reply}, which corresponds
#'   to the \code{kernel_info_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{kernel_info_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#kernel-info}
#' @export
kernel_info_request <- function(request_msg) {
  linfo <- list ( name = "R"
  , codemirror_mode = "r"
  , pygments_lexer = "r"
  , mimetype = "text/x-r-source"
  , file_extension = ".R"
  , version = paste(version$major, version$minor, sep=".")
  )
  content <- list( protocol_version = "5.2"
  , implementation   = "JuniperKernel"
  , implementation_version = as.character(packageVersion("JuniperKernel"))
  , language_info = linfo
  , banner = version$version.string
  )

  list(msg_type = "kernel_info_reply", content = content)
}