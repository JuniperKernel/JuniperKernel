# Check that the 'busy' and 'idle' messages are published
# Also collect intermediate messages and return them
# in a sensible structure for individual test checks.
check_iopub_messages <- function(msg_type) {
  msgs <- recv_msgs()
  checkTrue(length(msgs)!=0L)

  busy <- jsonlite::fromJSON(msgs[1L])
  idle <- jsonlite::fromJSON(msgs[length(msgs)])

  if(busy$content$execution_state!="busy") {
    println("ACTUAL BUSY MESSAGE IS")
    print(busy)
    print(msgs)
  }

  checkTrue(busy$content$execution_state=="busy")
  checkTrue(busy$parent_header$msg_type==msg_type)

  if(idle$content$execution_state!="idle") {
    println("ACTUAL IDLE MESSAGE IS")
    print(idle)
    print(msgs)
  }
  checkTrue(idle$content$execution_state=="idle")
  checkTrue(idle$parent_header$msg_type==msg_type)

  invisible(msgs)
}


recv_msgs <- function() {
  msgs <- c()

  # spin until messages are available from iopub
  # timeout after ~5 seconds if no messages published...
  ntries <- 30L
  while( ntries >= 0L && (msg <- JuniperKernel:::iopub_recv(jclient))=="" ) {
    if( !ntries%%6L ) Sys.sleep(1L)  # sleep for 1 second every 6 tries
    ntries <- ntries - 1L
  }

  # if we didn't get any message, don't count it
  if( nzchar(msg) )
    msgs <- c(msgs, msg)

  # Optimistically poll for the idle message. iopub_recv races with all
  # message writers, so intermittently the queue will be empty and produce
  # a read resulting in "". Allow for ~3 seconds of polling if no idle is
  # reached so that we don't gum up the test suite.
  # Note that even if the previous poll produced no messages read, then
  # this loop tries One More Time. And, on each non-zero length message
  # the retry mechanism is reset.
  ntries <- 30L
  notIdle <- TRUE
  while( notIdle ) {
    if( ntries <= 0L ) break
    msg <- JuniperKernel:::iopub_recv(jclient)
    if( nzchar(msg) ) {
      ntries <- 30L  # reset
      msgs <- c(msgs, msg)
      # check if we got the idle message (which is the last message)
      jmsg <- jsonlite::fromJSON(msg)
      if( !is.null(jmsg$content$execution_state) &&
           jmsg$content$execution_state=="idle" ) {
        notIdle <- FALSE
      }
    } else {
      # Poll the iopub message queue until the "idle" message arrives.
      # we are racily reading messages from the iopub queue whilst writers
      # are in progress. An empty queue with no "idle" message published may
      # mean more messages are yet to come, or a more serious problem.
      # If no messages come after `ntries` attempts, then bail on this and
      # return what we have so far.
      #
      # Note that if we fail to recv the idle message, then all downstream
      # tests will be broken!
      ntries <- ntries - 1L
      println("waiting for idle...")
      Sys.sleep(.10)
    }
  }
  msgs
}