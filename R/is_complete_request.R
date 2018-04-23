# Copyright (C) 2017-2018  Spencer Aiello
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

#' Handler for the is_complete_request Message Type
#'
#' @title Is Complete Handler
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
#'
#' @examples
#' \dontrun{
#'   request_msg <- list(code="print(")  'incomplete'
#'   is_complete_request(request_msg)
#' }
#'
#' \dontrun{
#'   request_msg <- list(code="print(5)")  # 'complete'
#'   is_complete_request(request_msg)
#' }
#'
#' @export
is_complete_request <- function(request_msg) {
  code <- request_msg$content$code
  status <- tryCatch(
    {parse(text=code);'complete'},
    error=function(e) {
      if( grepl("INCOMPLETE_STRING"      , e$message) )      "incomplete"  # unmatched quote
      else if( grepl("unexpected end of input", e$message) ) "incomplete"  # unmatched (
      else "invalid"
    }
  )
  content <- list(status=status)
  if( status=='incomplete') content <- list(status=status, indent='')
  list(msg_type = "is_complete_reply", content=content)
}
