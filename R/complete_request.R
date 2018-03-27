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
#'
#' @examples
#' \dontrun{
#'   request_msg <- list("code"="print(\"hello\")", cursor_pos=4)
#'   complete_request(request_msg)
#' }
#'
#' @export
complete_request <- function(request_msg) {
  code <- request_msg$code
  code <- gsub("\n", ";", code)
  cursor <- request_msg$cursor_pos

  cc <- getNamespace("utils")
  # see ?rcompgen Unexported API
  cc$.assignLinebuffer(code)
  cc$.assignEnd(cursor)
  cc$.guessTokenFromLine()
  cc$.completeToken()

  completions <- cc$.CompletionEnv$comps
  guess <- cc$.guessTokenFromLine(update = FALSE)
  matches <- unique(completions)
  if( length(matches) > 0L ) matches <- matches[1L:min(50L, length(matches))]
  if( length(matches)==1L  ) matches <- list(matches)
  content <- list(status="ok", matches=matches, cursor_start=guess$start, cursor_end=guess$start + nchar(guess$token))
  jsonlite::toJSON(content, auto_unbox=TRUE)
}
