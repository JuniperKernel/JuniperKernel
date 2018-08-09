### Modified from Rserve/src/install.libs.R
### For libs
files <- c("JuniperKernel.so", "JuniperKernel.so.dSYM", "JuniperKernel.dylib",
           "JuniperKernel.dll", "symbols.rds")
files <- files[file.exists(files)]
if(length(files) > 0){
  libsarch <- if (nzchar(R_ARCH)) paste("libs", R_ARCH, sep='') else "libs"
  dest <- file.path(R_PACKAGE_DIR, libsarch)
  dir.create(dest, recursive = TRUE, showWarnings = FALSE)
  print(paste("copying from", files, "TO", dest))
  file.copy(files, dest, overwrite = TRUE, recursive = TRUE)

  ### For Mac OSX:
  ### Overwrite RPATH from the shared library installed to the destination.
  if(Sys.info()[['sysname']] == "Darwin"){
    cmd.int <- system("which install_name_tool", intern = TRUE)
    cmd.ot <- system("which otool", intern = TRUE) 
    fn.JK.so <- file.path(dest, "JuniperKernel.so")

    if(file.exists(fn.JK.so)){
      ### For JuniperKernel.so
      rpath <- system(paste(cmd.ot, " -L ", fn.JK.so, sep = ""),
                      intern = TRUE)
      cat("\nBefore install_name_tool (in install.libs.R):\n")
      print(rpath)
    }
  }
}

