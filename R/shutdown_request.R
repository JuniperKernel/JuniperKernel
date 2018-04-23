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

#' Response to the \code{shutdown_request} Message Type
#'
#' @title Kernel Info Request Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{shutdown_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{shutdown_reply}, which corresponds
#'   to the \code{shutdown_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{shutdown_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#kernel-shutdown}
#'
#' @examples
#' \dontrun{
#'   request_msg <- list(restart=FALSE)
#'   shutdown_request(request_msg)
#' }
#'
#' @export
shutdown_request <- function(request_msg) {
  list(msg_type="shutdown_reply", content=list(restart=request_msg$content$restart))
}