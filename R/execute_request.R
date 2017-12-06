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
  rebroadcast_input(.kernel(), content$code, cnt <- .getAndIncCnt())  # increase count no matter what
  status <- .tryEval(content$code, cnt)
  content <- list(status=status, execution_count=cnt)

  if( status=="ok" ) {
    content$payload = list()
    content$user_expressions=list()
  }

  list(msg_type = "execute_reply", content = content)
}

.tryEval <- function(code, cnt) {
  tryCatch(
    {
      res <- .chkDTVisible(withVisible(eval(parse(text=code), envir=.GlobalEnv)))
      .setGlobal(".Last.value", res$value, 1L)
      if( res$visible )
        .execute_result(res$value, cnt)
      return("ok")
    }
    , error=function(e) {
        message(e, "\n")
        return("error")
      }
    , interrupt = function(.) return("abort")
  )
}

# build and send the content of an execute_result iopub message.
.execute_result <- function(result, cnt) {
  content <- list(data=.mimeBundle(result), execution_count=cnt, metadata=list())
  execute_result(.kernel(), content)
}


.chkDTVisible <- function(result) {
  if( "data.table" %in% oldClass(result$value) )
    result$visible <- result$visible && data.table::shouldPrint(result$value)
  result
}

# global env set hack
.setGlobal <- function(key, val, pos) {
  assign(key, val, envir=as.environment(pos))
}