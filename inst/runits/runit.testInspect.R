test.inspect1 <- function() {
  # inspect lm()
  payload <- '{"content":{"code":"lm()","cursor_pos":1,"detail_level":0},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"inspect_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("inspect_request")

}

test.inspect2 <- function() {
  # multi-line inspect on paste0:
  #   for(i in 1:50) {
  #     paste0("foo", "bar")
  #   }
  payload <- '{"content":{"code":"for(i in 1:50) {\\n    paste0(\'foo\',\'bar\')\\n}","cursor_pos":25,"detail_level":0},"header":{"date":"2017-12-23T00:40:20.039931Z","msg_id":"5AF1678C320A4C3F8C649DF82F44AFAE","msg_type":"inspect_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("inspect_request")

}

test.inspect3 <- function() {
  # inspect undefined 'asdf'
  payload <- '{"content":{"code":"asdf","cursor_pos":3,"detail_level":0},"header":{"date":"2017-12-23T00:41:28.207465Z","msg_id":"9EEFD98E06C94432805D2056851FDC4B","msg_type":"inspect_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("inspect_request")

}