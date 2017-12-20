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
#'
#' @examples
#' \dontrun{
#'   request_msg <- list("comm_id"="uniq_comm_id", "target_name"="my_comm", "data"=list())
#'   comm_open(request_msg)
#' }
#'
#' @export
comm_open <- function(request_msg) {
  comm_request("open")
  NULL
}
