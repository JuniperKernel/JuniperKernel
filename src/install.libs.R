### Modified from Rserve/src/install.libs.R
### For libs
files <- c("libxeus.so", "libxeus.dll", "run", "JuniperKernel.so", "JuniperKernel.so.dSYM", "JuniperKernel.dylib",
           "JuniperKernel.dll", "symbols.rds")
files <- files[file.exists(files)]
if(length(files) > 0){
  libsarch <- if (nzchar(R_ARCH)) paste("libs", R_ARCH, sep='') else "libs"
  dest <- file.path(R_PACKAGE_DIR, libsarch)
  dir.create(dest, recursive = TRUE, showWarnings = FALSE)
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

    lxeus.path <- file.path(dest, "libxeus.so")
    cmd.lxeus <- paste(cmd.int, " -change ", " libxeus.so ", lxeus.path, fn.JK.so)
    print(cmd.lxeus)
    print(system(cmd.lxeus))
    rpath <- system(paste(cmd.ot, " -L ", fn.JK.so, sep = ""), intern = TRUE)
    cat("\nAfter install_name_tool (in install.libs.R):\n")
    print(rpath)

    cat("\nFix up run")
    run.so <- file.path(dest, "run")

    cmd.lxeus <- paste(cmd.int, " -change ", " libxeus.so ", lxeus.path, run.so)
    print(cmd.lxeus)
    print(system(cmd.lxeus))
  }
}
