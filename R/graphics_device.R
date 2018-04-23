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

#' Set JuniperKernel's Graphic Device Settings
#'
#' @title Set JuniperKernel's Graphic Device Settings
#' @param background_color
#'    A character string of the color for the image background.
#'    Examples: "red", "blue", "green".
#'
#' @param width Width in inches.
#' @param height Height in inches.
#' @param pointsize Default point size.
#'
#' @param system_fonts
#'    Named list of font names to be aliased with fonts installed on your system. If
#'    unspecified, the R default families sans, serif, mono and symbol are aliased to
#'    the family returned by match_family().
#'
#' @param user_fonts
#'    Named list of fonts to be aliased with font files provided by the user rather
#'    than fonts properly installed on the system. The aliases can be fonts from the
#'    fontquiver package, strings containing a path to a font file, or a list containing
#'    name and file elements with name indicating the font alias in the SVG output
#'    and file the path to a font file.
#'
#' @param device_off
#'    Use this to toggle the JuniperKernel graphics device on and off. If you would like to
#'    to save plots to a pdf file, e.g., you would first turn off the JuniperKernel device
#'    here and then create a new pdf graphics device.
#'
#' @return Invisibly return a list of the settings.
#'
#' @details Use this method to set the graphics settings used by JuniperKernel.
#'
#' @author Spencer Aiello
#'
#' @export
jk_device_settings <- function(background_color="white", width = 10L, height = 5L,
                               pointsize = 12L, system_fonts=NULL, user_fonts=NULL, device_off=FALSE) {
  if( is.null(system_fonts) )
    system_fonts <- list(sans="Arial", serif="Times", mono="Courier", symbol="Symbol")

  if( is.null(user_fonts) )
    user_fonts <- list()

  aliases <- list(system=system_fonts, user=user_fonts)
  invisible(.JUNIPER$jkdopts <- list(bg = background_color, w=width, h=height, ps=pointsize, aliases=aliases, device_off=device_off))
}


#' Reset JuniperKernel's Graphic Settings
#'
#' @title Set JuniperKernel's Graphic Device Settings to Defaults
#' @return Invisibly return a list of the settings.
#' @details Use this method to set the graphics settings used by JuniperKernel.
#' @author Spencer Aiello
#' @export
jk_device_defaults <- function() {
  jk_device_settings()
}
