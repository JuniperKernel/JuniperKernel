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

#' Handler for the history_request Message Type
#'
#' @title Inspect Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{history_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{history_reply}, which corresponds
#'   to the \code{history_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{history_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#history}
#' @export
history_request <- function(request_msg) {
  message("unimpl")
  list(msg_type = "history_reply", content = list())
}
