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

# TODO: awaiting PR from @snoweye: https://github.com/snoweye/pbdZMQ/blob/master/R/overwrite_shpkg.r
.onLoad   <- function(lib, pkg) {
  ### For osx only.
  if(Sys.info()[['sysname']] == "Darwin") {
    cmd.int <- system("which install_name_tool", intern = TRUE)
    cmd.ot <- system("which otool", intern = TRUE)

    ### Get rpath from JuniperKernel shared library.
    fn.so <- paste(lib, "/", pkg, "/libs/JuniperKernel.so", sep = "")
    rpath <- system(paste(cmd.ot, " -L ", fn.so, sep = ""), intern = TRUE)

    ### Get dylib file from rpath
    pattern <- paste("^\\t(.*/pbdZMQ/libs/libzmq.*\\.dylib) .*$", sep = "")
    id <- grep(pattern, rpath)
    if( length(id)==0L ) {
      # zeromq from pbdZMQ not found; check for external zeromq
      pattern <- paste("^\\t(.*/libzmq.*\\.dylib) .*$", sep = "")
      id <- grep(pattern, rpath)
    }
    fn.dylib <- gsub(pattern, "\\1", rpath[id])

    ### Do nothing if the dylib file exists at which rpath points.
    ### Overwrite with new one if the dylib file does not exist.
    if(!file.exists(fn.dylib)) {
      pbdZMQ <- system.file("./libs", package="pbdZMQ")
      dn <- tools::file_path_as_absolute(pbdZMQ)
      fn <- list.files(path = dn, pattern = "libzmq(.*)\\.dylib")
      new.fn.dylib <- paste(dn, "/", fn, sep = "")

      cmd <- paste(cmd.int,
                   " -change ",
                   fn.dylib,
                   " ",
                   new.fn.dylib,
                   " ",
                   fn.so,
                   sep = "")
      system(cmd)
    }
  }

  ### Load "JuniperKernel.so".
  library.dynam("JuniperKernel", pkg, lib)

  invisible()
} # End of .onLoad().