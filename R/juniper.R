#' JuniperKernel: An R Kernel for Jupyter
#'
#' @docType package
#' @name JuniperKernel
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