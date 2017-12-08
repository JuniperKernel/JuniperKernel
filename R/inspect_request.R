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

#' Handler for the inspect_request Message Type
#'
#' @title Inspect Handler
#' @param request_msg
#'   A list passed in from \code{doRequest} representing the
#'   deserialized \code{inspect_request} message JSON.
#'
#' @return
#'   A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is \code{inspect_reply}, which corresponds
#'   to the \code{inspect_request} message. The \code{content} field
#'   complies with the Jupyter wire message protocol specification
#'   for \code{inspect_reply} messages.
#'
#' @author Spencer Aiello
#' @references \url{http://jupyter-client.readthedocs.io/en/latest/messaging.html#introspection}
#' @importFrom methods is
#' @export
inspect_request <- function(request_msg) {
  code <- request_msg$code
  notFound <- list(msg_type="inspect_reply", content=list(status="ok", found=FALSE, metadata=list()))

  if( !nzchar(code) )
    return(notFound)

  code <- gsub("\n", ";", code)
  start <- cursor <- request_msg$cursor_pos

  # The token we try to perform an inspect upon consists
  # of the characters from position P up to and including the current cursor
  # position. Position "P" is the special "beginning of token" position that
  # is defined by the guesser's regex (see below).
  #
  # Roll the cursor forward until the guesser declares "end of token"
  # signaled by a returning empty string.
  #
  # Note: guesser differrs from utils:::.guessTokenFromLine by allowing for '%'
  while( (cursor <= nchar(code)) && (.guessToken(code, cursor)!="") )
    cursor <- cursor+1L  # roll cursor forward
  token <- .guessToken(code, cursor-1L)  # rolled off the end of the token, so subtract 1

  if( !nzchar(token) )
    return(notFound)

  # try to get an R object for the parsed token
  Robj <- NULL
  if( token %in% .reserved )
    Robj <- .reserved[[token]]

  if( is.null(Robj) )
    Robj <- tryCatch(eval(parse(text=token), envir=.GlobalEnv), error=function(.){})

  if( is.null(Robj) )
    Robj <- tryCatch(get(token), error=function(.){})

  # break out the :: and ::: case
  # otherwise try "?token"
  # if these fail try "??token"
  # if all fails, return NULL
  helpObj <-
    tryCatch(
      tryCatch(
        {
          # token has form <pkg>::<fun> or <pkg>:::<fun>
          if( grepl("::", token) ) {
            toks <- unlist(strsplit(token, ":"))
            package <- toks[1L]
            topic   <- toks[2L]
            utils::help(topic=topic, package=package)
          } else {
            eval(parse(text=paste0('?', token)))
          }
        }, error=function(e) eval(parse(text=paste0("??", token)))
      )
    , error=function(.){NULL}
    )

  classInfo <- .mimeBundle(class(Robj))
  if( is(Robj, "data.frame") )
    Robj <- utils::head(Robj, 50L)  # only print first 50 rows
  printInfo <- .mimeBundle(Robj)
  helpInfo  <- .mimeBundle(helpObj)

  .addPageSection <- function(bundle, title, content) {
    templates <- list('text/plain' = '%s\n', 'text/html' = '<h1>%s:</h1>\n')
    for( mime in names(templates) ) {
      data <- content[[mime]]
      if( !is.null(data) ) {
        title <- sprintf(templates[[mime]], title)
        bundle[[mime]] <- paste0(bundle[[mime]], title, data, '\n', sep='\n')
      }
    }
    bundle
  }
  # bundle up the info
  data <- {
    if( is(Robj, 'function') )
      helpInfo
    else {
      res <- list()
      res <- .addPageSection(res, "Class", classInfo)
      res <- .addPageSection(res, "Printed (data frames are truncated)", printInfo)
      res <- .addPageSection(res, "Help", helpInfo)
      res
    }
  }

  list(msg_type = "inspect_reply", content = list(status="ok", found=length(data)!=0L, data=data, metadata=list()))
}

.reserved <- c( 'if' = `if`
              , 'else'  = NULL
              , 'repeat' = `repeat`
              , 'while' = `while`
              , 'function' = `function`
              , 'for' = `for`
              , 'in' = NULL
              , 'next' = `next`
              , 'break' = `break`
              , 'TRUE' = TRUE
              , 'FALSE' = FALSE
              , 'NULL' = NULL
              , 'Inf' = Inf
              , 'NaN' = NaN
              , 'NA' = NA
              , 'NA_integer_' = NA_integer_
              , 'NA_real_' = NA_real_
              , 'NA_complex_' = NA_complex_
              , 'NA_character_' = NA_character_
              )

.guessToken <- function(buf, end) {
  cc <- getNamespace('utils')
  h <- cc$head.default
  t <- cc$tail.default
  buf <- substr(buf, 1L, end)
  insideQuotes <- {
    l <- h(unlist(strsplit(buf, "")), end)
    ((sum(l == "'")%%2 == 1) || (sum(l == "\"")%%2 == 1))
  }
  start <-
    if( insideQuotes ) suppressWarnings(gregexpr("['\"]", buf, perl = TRUE))[[1L]]
    else               suppressWarnings(gregexpr("[^\\.\\w:?$@[\\]%]+", buf, perl = TRUE))[[1L]]

  start <-
    if( all(start<0L) ) 0L
    else                t(start + attr(start, "match.length"), 1L) - 1L

  substr(buf, start + 1L - insideQuotes, end)
}
