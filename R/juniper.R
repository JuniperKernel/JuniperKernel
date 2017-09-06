#' JuniperKernel: An R Kernel for Jupyter
#'
#' @docType package
#' @name JuniperKernel
NULL


# global package environment
.JUNIPER <- new.env(parent=emptyenv())
.JUNIPER$execution_count <- 1L

.kernel <- function() .JUNIPER$kernel
.exeCnt <- function() .JUNIPER$execution_count
.incCnt <- function() .JUNIPER$execution_count <- .JUNIPER$execution_count + 1L