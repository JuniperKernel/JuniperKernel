# Copyright (C) 2017  Spencer Aiello
#
# This file is part of JuniperKernel.
#
# JuniperKernel is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# JuniperKernel is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with JuniperKernel.  If not, see <http://www.gnu.org/licenses/>.

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
  status <- .tryEval(request_msg$code, request_msg$execution_count)
  content <- list(status=status, execution_count=request_msg$execution_count)

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
  publish_execute_result(cnt, .mimeBundle(result))
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