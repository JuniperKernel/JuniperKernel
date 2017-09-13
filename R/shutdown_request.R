#' Response to the \code{shutdown_request} Message Type
#'
#' @title Kernel Info Request Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{shutdown_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{shutdown_reply}, which corresponds
#'   to the \code{shutdown_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{shutdown_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#kernel-shutdown}
#' @export
shutdown_request <- function(request_msg) {
  list( msg_type="shutdown_reply", content=list(restart=request_msg$content$restart))
}