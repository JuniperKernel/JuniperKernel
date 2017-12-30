test.CommInfo <- function() {
  payload <- '{"content":{"target_name":"jupyter.widget"},"header":{"msg_id":"FF4875471ED74DAB89377F9169E1F43F","msg_type":"comm_info_request","session":"3CF9177FB45047B39E1B998A55B4D66C","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  reply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("comm_info_request")

  checkTrue(is.null(reply$content$comms))
  checkTrue(reply$content$status=="ok")
  checkTrue(reply$header$msg_type=="comm_info_reply")
}
