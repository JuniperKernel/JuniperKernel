#' Top-level Request Driver for R
#'
#' @title Handle Jupyter Requests
#' @param handler
#'   An R method that handles the message type of
#'   \code{request_msg}. This function is passed in
#'   by the \code{RequestServer}, which handles all of
#'   generic message handling such as validation and
#'   routing. This handler is one of c('kernel_info_request',
#'   'execute_request', 'inspect_request', 'complete_request',
#'   'history_request', 'is_complete_request', 'comm_info_request',
#'   'comm_open', 'comm_close', 'comm_msg', 'shutdown_request').
#'
#' @param request_msg
#'   A list passed in from \code{RequestServer} representing the
#'   deserialized message JSON.
#'
#' @return A list having names \code{msg_type} and \code{content}. The
#'   \code{msg_type} is the reply type corresponding to the
#'   \code{request_msg}'s message type. For example, a
#'   \code{kernel_info_request} message produces a list with
#'   \code{msg_type=kernel_info_reply}. The \code{content} field
#'   of this list is dictated by the Jupyter wire message protocol.
#'   Note that the full reply to a Jupyter client is managed by the
#'   RequestServer.
#'
#' @details
#'   All client requests are eventually funneled through this
#'   top-level request driver via the \code{RequestServer}. It is
#'   the job of the \code{RequestServer} to inspect messages and
#'   invoke \code{doRequest} with the appropriate \code{handler}.
#'   In other words, \code{doRequest} focuses purely on redirecting
#'   stdout/stderr and calling the \code{handler}. Message streams are
#'   detoured to a \code{socketConnection} hosted in the current thread
#'   and connected to by a separate thread polling on a ZMQ_STREAM socket.
#'   These details are all handled by the \code{RequestServer}, and all
#'   \code{doRequest} does is \code{sink} messages to the socket and perform
#'   cleanup. The port is passed as part of the \code{request_msg}, and is
#'   chosen randomly by the \code{RequestServer}.
#'
#' @author Spencer Aiello
#' @export
doRequest <- function(handler, request_msg) {
  out <- socketConnection("localhost", port=request_msg$stream_out_port)
  err <- socketConnection("localhost", port=request_msg$stream_err_port)
  sink(out, type="output")
  sink(err, type="message")
  aliases <- list(system=list(sans="Arial", serif="Times", mono="Courier", symbol="Symbol"), user=list())
  jk_device(.kernel(), "white", 10, 5, 12, FALSE, aliases)
  dev <- grDevices::dev.cur()
  tryCatch(
      return(handler(request_msg))
    , finally={
        sink(type="message"); close(err);
        sink(type="output" ); close(out);
        grDevices::dev.off(dev)
      }
  )
}
