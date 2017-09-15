#' Handler for the inspect_request Message Type
#'
#' @title Inspect Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{inspect_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{inspect_reply}, which corresponds
#'   to the \code{inspect_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{inspect_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#introspection}
#' @export
comm_close <- function(request_msg) {
  message("unimpl")
  list(msg_type = "inspect_reply", content = list(status="ok", found=FALSE))
}
