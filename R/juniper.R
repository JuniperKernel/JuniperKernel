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

#' JuniperKernel: An R Kernel for Jupyter
#'
#' @docType package
#' @name JuniperKernel
#' @import gdtools
#' @import Rcpp
#' @import pbdZMQ
NULL


# global package environment
.JUNIPER <- new.env(parent=emptyenv())
.JUNIPER$execution_count <- 1L
.JUNIPER$jkdopts <- NULL

.kernel <- function() .JUNIPER$kernel
.getAndIncCnt <- function() {
  cnt <- .JUNIPER$execution_count
  .JUNIPER$execution_count <- cnt + 1L
  cnt
}

.mimeBundle <- function(Robj) {
  # from the docs (http://jupyter-client.readthedocs.io/en/latest/messaging.html#display-data):
  #     "A single message should contain all possible representations
  #     of the same information. Each representation should be a
  #     JSON'able data structure, and should be a valid MIME type"
  # loop over the available mimetypes using repr package
  data <- lapply(repr::mime2repr, function(mimeFun) mimeFun(Robj))
  # keep non-NULL results (NULL when no such mime repr exists)
  Filter(Negate(is.null), data)
}

.addPageSection <- function(bundle, title, content) {
  templates <- list('text/plain' = '%s\n', 'text/html' = '<h1>%s:</h1>\n')
  for( mime in names(templates) ) {
    data <- content[[mime]]
    if( !is.null(data) ) {
      title <- sprintf(templates[[mime]], title)
      bundle[[mime]] <- paste0(bundle[[mime]], title, data, '\n', sep='\n')
    }
  }
  bundle
}