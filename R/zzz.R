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
.onLoad   <- function(lib, pkg) {

  ### Use default library.
  dn <- paste(lib, "/", pkg, "/libs/", sep = "")

  ### For Mac OSX.
  ### Overwrite RPATH from the shared library installed to the destination.
  if(Sys.info()[['sysname']] == "Darwin"){
    dest <- gsub("/$", "", dn)
    cmd.int <- system("which install_name_tool", intern = TRUE)
    fn.JK.so <- file.path(dest, "JuniperKernel.so")

    if(length(grep("install_name_tool", cmd.int)) > 0 &&
       file.exists(fn.JK.so)){

      ### Check original path.
      cmd.ot <- system("which otool", intern = TRUE)
      rpath <- system(paste(cmd.ot, " -L ", fn.JK.so, sep = ""),
                      intern = TRUE)
      cat("\nBefore install_name_tool (in zzz.R):\n")
      print(rpath)
    }
  }


  library.dynam("JuniperKernel", pkg, lib)
  invisible()
}
