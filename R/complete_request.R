#' Handler for the complete_request Message Type
#'
#' @title Complete Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{complete_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{complete_reply}, which corresponds
#'   to the \code{complete_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{complete_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#completion}
#' @export
complete_request <- function(request_msg) {
  code <- request_msg$content$code
  code <- gsub("\n", ";", code)
  cursor <- request_msg$content$cursor_pos

  cc <- getNamespace("utils")
  # see ?rcompgen Unexported API
  cc$.assignLinebuffer(code)
  cc$.assignEnd(cursor)
  cc$.guessTokenFromLine()
  cc$.completeToken()

  completions <- cc$.CompletionEnv$comps
  if( length(completions) > 0L ) completions <- c(completions, "__juniper_vec_ignore_hack__")  # hack to get single item vecs to parse as vecs and not scalars
  guess <- cc$.guessTokenFromLine(update = FALSE)
  matches <- unique(completions)
  content <- list(status="ok", matches = matches[1L:min(50L, length(matches))], cursor_start=guess$start, cursor_end=guess$start + nchar(guess$token))
  list(msg_type = "complete_reply", content=content)
}
