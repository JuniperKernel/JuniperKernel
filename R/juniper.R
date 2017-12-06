#' JuniperKernel: An R Kernel for Jupyter
#'
#' @docType package
#' @name JuniperKernel
#' @import gdtools
#' @import Rcpp
#' @useDynLib JuniperKernel, .registration = TRUE
NULL


# global package environment
.JUNIPER <- new.env(parent=emptyenv())
.JUNIPER$execution_count <- 1L

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