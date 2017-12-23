test.KernelInfo <- function() {
  payload <- '{"content":{},"header":{"date":"2017-12-17T00:30:33.180360Z","msg_id":"1a7186aa-1d2fcbab990a326ebb6747d4","msg_type":"kernel_info_request","session":"944E351AF3AD44B78054DF4DAB6FA0E5","username":"spencer","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  kinfo <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  check_iopub_messages("kernel_info_request")

  content <- kinfo$content
  checkTrue(content$banner=="R version 3.4.3 (2017-11-30)")
  checkTrue(content$implementation=="JuniperKernel")
  checkTrue(content$language_info$codemirror_mode=="r")
  checkTrue(content$language_info$file_extension==".R")
  checkTrue(content$language_info$mimetype=="text/x-r-source")
  checkTrue(content$language_info$name=="R")
  checkTrue(content$language_info$pygments_lexer=="r")
  checkTrue(content$protocol_version=="5.2")

  header <- kinfo$header
  checkTrue(header$msg_type=="kernel_info_reply")
  checkTrue(header$version=="5.2")

  parent_header <- kinfo$parent_header
  checkTrue(parent_header$msg_type=="kernel_info_request")
  checkTrue(parent_header$version=="5.2")
}