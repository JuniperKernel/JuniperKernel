test.testWarning <- function() {

  payload <- '{"content":{"allow_stdin":true,"code":"warning(\'this is a warning\')","silent":false,"stop_on_error":true,"store_history":true,"user_expressions":{}},"header":{"date":"2017-12-21T22:17:43.016427Z","msg_id":"6D380F5E23EA4014B212A7B8E0B8DBB1","msg_type":"execute_request","session":"231E20AF0A8240E2821ABF1E266EC729","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  execReply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  msgs <- check_iopub_messages("execute_request")

  checkTrue(execReply$content$status=="ok")
  checkTrue(execReply$header$msg_type=="execute_reply")
  checkTrue(execReply$parent_header$msg_type=="execute_request")


  # expect 4 messages:
  #  + 1 busy
  #  + 1 re-broadcast
  #  + 1 result
  #  + 1 idle
  checkTrue(length(msgs)==4L)

  rebroad <- jsonlite::fromJSON(msgs[2L])
  checkTrue(rebroad$content$code=="warning('this is a warning')")
  checkTrue(rebroad$header$msg_type=="execute_input")
  checkTrue(rebroad$parent_header$msg_type=="execute_request")

  result <- jsonlite::fromJSON(msgs[3L])

  checkTrue(result$content$name=="stderr")
  checkTrue(result$content$text=="this is a warning\n\n")
  checkTrue(result$header$msg_type=="stream")
}
