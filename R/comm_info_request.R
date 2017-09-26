#' Handler for the comm_info_request Message Type
#'
#' @title Inspect Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{comm_info_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{comm_info_reply}, which corresponds
#'   to the \code{comm_info_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{comm_info_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#comm-info}
#' @export
comm_info_request <- function(request_msg) {
  target <- ifelse(is.null(request_msg$target), "", request_msg$target_name)
  list(msg_type = "comm_info_reply", content = filter_comms(target))
}
