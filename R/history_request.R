#' Handler for the history_request Message Type
#'
#' @title Inspect Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{history_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{history_reply}, which corresponds
#'   to the \code{history_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{history_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#history}
#' @export
history_request <- function(request_msg) {
  # history is effectively unimpl'd
  list(msg_type = "history_reply", content = list())
}
