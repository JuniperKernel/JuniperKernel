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
inspect_request <- function(request_msg) {
  code <- request_msg$content$code
  code <- gsub("\n", ";", code)
  cursor <- request_msg$content$cursor_pos

  # cc <- getNamespace('utils')
  # # see ?rcompgen Unexported API
  # cc$.assignLinebuffer(code)
  # cc$.assignEnd(cursor)
  # tok <- cc$.guessTokenFromLine()
  # print(paste0("guessed token: ", tok))
  #
  # if( nchar(tok) ) {
  #   o <- tryCatch(eval(parse(text=code), envir=.GlobalEnv), error=function(.)NULL)
  #   print(paste0("o= ", o))
  #   help_struct <- eval(parse(text = paste0('?', tok)))
  # }

  # data <- lapply(repr::mime2repr, function(mimeFun) mimeFun(result))
  # keep non-NULL results (NULL when no such mime repr exists)
  # content <- list(data=Filter(Negate(is.null), data), execution_count=cnt)


  list(msg_type = "inspect_reply", content = list(status="ok", found=FALSE))
}
