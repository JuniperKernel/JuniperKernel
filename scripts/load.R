# Change this global variable to match your own system's path

spen.git.root <- "/Users/spencer/RicePlate/JuniperKernel"

PATH <- file.path(spen.git.root, 'R')

src <- function() {
  to_src <- c('init.R', 'kernelspec.R', 'utils.R')
  
  require(jsonlite);
  invisible(lapply(to_src,function(x){source(file.path(PATH,x))}))

  .onLoad()
  .onAttach()


  invisible(NULL)
}
src()
