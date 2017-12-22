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

  # read all messages until reaching "" or idle
  # if no idle, then poll the queue for 3 seconds for more messages...
  # even if no messages were read above, this will try One More Time
  ntries <- 30L
  while( nzchar(msg <- JuniperKernel:::iopub_recv(jclient)) || ntries >=0L ) {
    if( nzchar(msg) ) {
      msgs <- c(msgs, msg)
      # check if we got the idle message (which is the last message)
      jmsg <- jsonlite::fromJSON(msg)
      if( !is.null(jmsg$content$execution_state) &&
           jmsg$content$execution_state=="idle" ) {
        break
      }
      ntries <- 30L  # reset
    } else {
      # Poll the iopub message queue until the "idle" message arrives.
      # we are racily reading messages from the iopub queue and we may
      # miss the idle if we aren't careful. The logic here is to continue
      # to poll until we get an "", which indicates an empty queue... BUT
      # more messages may be yet to come if we have not read an "idle".
      # However, we don't want to spin forever, so we poll for 3 seconds
      # and then let the tests continue running.
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