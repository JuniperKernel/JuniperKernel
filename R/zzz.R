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

.onAttach <- function(lib, pkg) {}

.onLoad <- function(libname, pkgname){
  ### For osx only.
  if(Sys.info()[['sysname']] == "Darwin"){
    cmd.ot <- system("which otool", intern = TRUE)

    ### Get rpath from pkgname's shared library.
    fn.so <- paste(libname, "/", pkgname, "/libs/", pkgname, ".so", sep = "")
    rpath <- system(paste(cmd.ot, " -L ", fn.so, sep = ""), intern = TRUE)

    ### Get the installed dylib from pkgname's rpath.
    pattern <- paste("^\\t(.*/pbdZMQ/libs/libzmq.*\\.dylib) .*$", sep = "")
    i.rpath <- grep(pattern, rpath)
    fn.dylib <- gsub(pattern, "\\1", rpath[i.rpath])
    if(length(fn.dylib) == 1){

      ### The file of the installed dylib does not exist.
      if(!file.exists(fn.dylib)){
        cmd.int <- system("which install_name_tool", intern = TRUE)

        ### Search the dylib from R's library path.
        dn <- tools::file_path_as_absolute(
                system.file("./libs", package = "pbdZMQ")) 
        fn <- list.files(path = dn, pattern = "libzmq.*\\.dylib")

        ### Install the searched one to the pkgname's shared library.
        if(length(fn) == 1){
          new.fn.dylib <- paste(dn, "/", fn, sep = "")
          cmd <- paste(cmd.int, " -change ", fn.dylib, " ", new.fn.dylib,
                       " ", fn.so,
                       sep = "")
          system(cmd)
        } else{
          stop("Neither external nor internal ZMQ can be found.") 
        }
      }
    }
  }

  ### Load "pkgname.so".
  library.dynam("JuniperKernel", pkgname, libname)

  invisible()
} # End of .onLoad().

