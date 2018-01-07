test.iscomplete1 <- function() {
  # provide a complete line
  payload <- '{"content":{"code":"lm()"},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"is_complete_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("is_complete_request")

  checkTrue(reply$content$status=="complete")
  checkTrue(reply$header$msg_type=="is_complete_reply")
  checkTrue(reply$parent_header$msg_type=="is_complete_request")
}

test.iscomplete2 <- function() {
  # provide a complete line
  payload <- '{"content":{"code":"paste0(\'hello world)"},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"is_complete_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("is_complete_request")

  checkTrue(reply$content$status=="incomplete")
  checkTrue(reply$header$msg_type=="is_complete_reply")
  checkTrue(reply$parent_header$msg_type=="is_complete_request")
}

test.iscomplete3 <- function() {
  # provide a complete line
  payload <- '{"content":{"code":"paste0(\'hello\', \' world\'"},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"is_complete_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("is_complete_request")

  checkTrue(reply$content$status=="incomplete")
  checkTrue(reply$header$msg_type=="is_complete_reply")
  checkTrue(reply$parent_header$msg_type=="is_complete_request")
}

test.iscomplete4 <- function() {
  # provide a complete line
  payload <- '{"content":{"code":"\'hello"},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"is_complete_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("is_complete_request")

  checkTrue(reply$content$status=="incomplete")
  checkTrue(reply$header$msg_type=="is_complete_reply")
  checkTrue(reply$parent_header$msg_type=="is_complete_request")
}


test.iscomplete5 <- function() {
  # provide a complete line
  payload <- '{"content":{"code":"hello)"},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"is_complete_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("is_complete_request")

  checkTrue(reply$content$status=="invalid")
  checkTrue(reply$header$msg_type=="is_complete_reply")
  checkTrue(reply$parent_header$msg_type=="is_complete_request")
}
