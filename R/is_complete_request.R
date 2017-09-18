#' Handler for the is_complete_request Message Type
#'
#' @title Complete Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{is_complete_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{is_complete_reply}, which corresponds
#'   to the \code{is_complete_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{is_complete_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#code-completeness}
#' @export
is_complete_request <- function(request_msg) {
  code <- request_msg$content$code
  status <- tryCatch(
    {parse(text=code);'complete'},
    error=function(e) {
      if( grepl("INCOMPLETE_STRING"      , e$message) ) "incomplete"  # unmatched quote
      if( grepl("unexpected end of input", e$message) ) "incomplete"  # unmatched (
      else "invalid"
    }
  )
  content <- list(status=status)
  if( status=='incomplete') content <- list(status=status, indent='')
  list(msg_type = "is_complete_reply", content=content)
}
