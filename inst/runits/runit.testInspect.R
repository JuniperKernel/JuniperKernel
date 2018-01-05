test.inspect1 <- function() {
  # inspect lm()
  payload <- '{"content":{"code":"lm()","cursor_pos":1,"detail_level":0},"header":{"date":"2017-12-23T00:38:26.553010Z","msg_id":"C58866E8121C4292896D777DCC7DB239","msg_type":"inspect_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("inspect_request")
  checkTrue(reply$content$found)
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$parent_header$msg_type=="inspect_request")

  checkTrue(all(names(reply$content$data) == c("text/html",  "text/latex", "text/plain")))
  checkTrue(grepl("<h2>Fitting Linear Models</h2>", reply$content$data$`text/html`))
  checkTrue(grepl("\\\\HeaderA\\{lm\\}\\{Fitting Linear Models\\}\\{lm\\}\n\\\\keyword\\{regression\\}\\{lm\\}", reply$content$data$`text/latex`))
  checkTrue(grepl("lm                    package:stats                    R Documentation", reply$content$data$`text/plain`))
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
  checkTrue(reply$content$found)
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$parent_header$msg_type=="inspect_request")

  checkTrue(all(names(reply$content$data) == c("text/html",  "text/latex", "text/plain")))
  checkTrue(grepl("<h2>Concatenate Strings</h2>", reply$content$data$`text/html`))
  checkTrue(grepl("\\\\HeaderA\\{paste\\}\\{Concatenate Strings\\}\\{paste\\}", reply$content$data$`text/latex`))
  checkTrue(grepl("paste                   package:base                   R Documentation", reply$content$data$`text/plain`))
}

test.inspect3 <- function() {
  # inspect undefined 'asdf'
  payload <- '{"content":{"code":"asdf","cursor_pos":3,"detail_level":0},"header":{"date":"2017-12-23T00:41:28.207465Z","msg_id":"9EEFD98E06C94432805D2056851FDC4B","msg_type":"inspect_request","session":"B4A85B30E41542B58F6ECD2074C9D2D6","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("inspect_request")
  checkTrue(reply$content$found)
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$parent_header$msg_type=="inspect_request")

  checkTrue(all(names(reply$content$data) == c("text/html", "text/plain")))
  # checkTrue(gsub('`', '\'', reply$content$data$`text/html`)=="<h1>Class\n:</h1>\n'NULL'\n\n<h1>Help\n:</h1>\nNo documentation for 'asdf' in specified packages and libraries:\nyou could try '??asdf'\n\n")
  # checkTrue(gsub('`', '\'', reply$content$data$`text/plain`)=="Class\n[1] \"NULL\"\n\nPrinted (data frames are truncated)\nNULL\n\nHelp\nNo documentation for 'asdf' in specified packages and libraries:\nyou could try '??asdf'\n\n")
}