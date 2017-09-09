#' Handler for the execute_request Message Type
#'
#' @title Execute Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{execute_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{execute_reply}, which corresponds
#'   to the \code{execute_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{execute_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#execution-results}
#' @export
execute_request <- function(request_msg) {
  content <- request_msg$content
  rebroadcast_input(.kernel(), content$code, cnt <- .exeCnt())

  exprs <- parse(text=content$code)
  .incCnt()
  list( msg_type = "execute_reply"
  , content = list(status="ok", execution_count = cnt)
  )
}