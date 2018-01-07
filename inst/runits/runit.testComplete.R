test.complete1 <- function() {
  # tab complete on "libr<tab><tab>"
  payload <- '{"content":{"code":"libr","cursor_pos":4},"header":{"msg_id":"AE07104CC69C42CA83CD3618FD4A6E62","msg_type":"complete_request","session":"3CF9177FB45047B39E1B998A55B4D66C","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("complete_request")

  checkTrue(reply$content$cursor_end==4)
  checkTrue(reply$content$cursor_start==0)
  checkTrue(all(reply$content$matches==c("library", "library.dynam", "library.dynam.unload")))
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$header$msg_type=="complete_reply")
}


test.complete2 <- function() {
  # tab complete on "libr<tab><tab>"
  payload <- '{"content":{"code":"library(foo)","cursor_pos":11},"header":{"msg_id":"8D9202723357443C80FFD1566FA4960E","msg_type":"complete_request","session":"3CF9177FB45047B39E1B998A55B4D66C","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("complete_request")

  checkTrue(reply$content$cursor_end==11)
  checkTrue(reply$content$cursor_start==8)
  checkTrue(length(reply$content$matches)==0)
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$header$msg_type=="complete_reply")
}
