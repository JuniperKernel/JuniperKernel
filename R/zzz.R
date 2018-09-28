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

.onAttach <- function(lib, pkg) {}

.onLoad <- function(lib, pkg) {
  pbdZMQ::overwrite.shpkg.rpath(mylib = lib, mypkg = pkg, linkingto = "pbdZMQ",
                                shlib = "zmq")
  library.dynam("JuniperKernel", pkg, lib)
  invisible()
} # End of .onLoad().


#' Fetch the Ld Flags for the JuniperKernel Library.
#' @title Fetch the Ld Flags for JuniperKernel.
#' @param arch Either /x64 or /i386 for windows; otherwise should be left empty.
#' @return Invisibly return the ldflags
#' @author Spencer Aiello
#' @export
JKLdFlags <- function(arch = '') {
  file.name <- paste("./libs", arch, "/", sep = "")
  dir.path <- tools::file_path_as_absolute(system.file(file.name, package = 'JuniperKernel'))
  ldflags <- {
    if( arch == "/i386" || arch == "/x64" )
      paste0("\"", dir.path, "/JuniperKernel.dll", "\"")
    else
      paste0("\"", dir.path, "/JuniperKernel.so" , "\"")
  }
  cat(ldflags)
  invisible(ldflags)
}