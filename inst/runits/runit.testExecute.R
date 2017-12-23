test.testExecute <- function() {

  # executes "1" in R
  payload <- '{"content":{"allow_stdin":true,"code":"1","silent":false,"stop_on_error":true,"store_history":true,"user_expressions":{}},"header":{"date":"2017-12-21T22:17:43.016427Z","msg_id":"6D380F5E23EA4014B212A7B8E0B8DBB1","msg_type":"execute_request","session":"231E20AF0A8240E2821ABF1E266EC729","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  execReply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  msgs <- check_iopub_messages("execute_request")

  checkTrue(execReply$content$status=="ok")
  checkTrue(execReply$header$msg_type=="execute_reply")
  checkTrue(execReply$parent_header$msg_type=="execute_request")


  # check the iopub messages
  # expect 4 messages:
  #  + 1 busy
  #  + 1 re-broadcast
  #  + 1 result
  #  + 1 idle
  checkTrue(length(msgs)==4L)

  rebroad <- jsonlite::fromJSON(msgs[2L])
  checkTrue(rebroad$content$code=="1")
  checkTrue(rebroad$header$msg_type=="execute_input")
  checkTrue(rebroad$parent_header$msg_type=="execute_request")

  result <- jsonlite::fromJSON(msgs[3L])
  checkTrue(result$content$data$`text/html`=="1")
  checkTrue(result$content$data$`text/markdown`=="1")
  checkTrue(result$content$data$`text/plain`=="[1] 1")
  checkTrue(result$header$msg_type=="execute_result")
  checkTrue(result$parent_header$msg_type=="execute_request")
}


test.testExecute2 <- function() {
  # execute "library(data.table)" in R
  payload <- '{"content":{"allow_stdin":true,"code":"library(data.table)","silent":false,"stop_on_error":true,"store_history":true,"user_expressions":{}},"header":{"date":"2017-12-21T22:36:20.723216Z","msg_id":"333A106131E24C908E329735E95F33E8","msg_type":"execute_request","session":"4CA10DA81FC54D108B6B61AD24761A8E","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  execReply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  msgs <- check_iopub_messages("execute_request")
  checkTrue(execReply$content$status=="ok")
  checkTrue(execReply$header$msg_type=="execute_reply")
  checkTrue(execReply$parent_header$msg_type=="execute_request")
}

test.testExecute3 <- function() {
  # execute 'print("hello world")' in R
  payload <- '{"content":{"allow_stdin":true,"code":"print(\'hello world\')","silent":false,"stop_on_error":true,"store_history":true,"user_expressions":{}},"header":{"date":"2017-12-21T22:37:12.426326Z","msg_id":"82D9D776FC6343BD9BFF702CB93C5D53","msg_type":"execute_request","session":"4CA10DA81FC54D108B6B61AD24761A8E","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  execReply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  msgs <- check_iopub_messages("execute_request")

  checkTrue(execReply$content$status=="ok")
  checkTrue(execReply$header$msg_type=="execute_reply")
  checkTrue(execReply$parent_header$msg_type=="execute_request")

  # this time there should be at least 4 messages:
  #  + 1 busy
  #  + 1 re-broadcast
  #  + n more messages for each stdout message
  #     - R will sink stdout to a TCP socket; and a single
  #       "print" statement may end up being split over
  #       multiple iopub messages
  #     - As a corollary, multiple "print"s may be combined
  #       into a single TCP send from R.
  #  + 1 idle
  checkTrue(length(msgs)>=4L)
  rebroad <- jsonlite::fromJSON(msgs[2L])
  checkTrue(rebroad$content$code=="print('hello world')")
  checkTrue(rebroad$header$msg_type=="execute_input")
  checkTrue(rebroad$parent_header$msg_type=="execute_request")

  # the next "n" messages must all be stdout; recombine
  # them into the single print that was executed
  e <- new.env(parent=emptyenv())
  e$txt <- ""
  invisible(lapply(3L:(length(msgs)-1L),
    function(i, e) {
      outmsg <- jsonlite::fromJSON(msgs[i])
      checkTrue(outmsg$content$name=="stdout")
      checkTrue(outmsg$parent_header$msg_type=="execute_request")
      e$txt <- paste0(e$txt, outmsg$content$text)
    },e=e))
  checkTrue(e$txt=="[1] \"hello world\"\n")
}


test.testExecute4 <- function() {
  # execute 'for(i in 1:50) print(paste0("hello ", i))' in R
  payload <- '{"content":{"allow_stdin":true,"code":"for(i in 1:10) {\\n print(paste0(\'hello \', i)) \\n }","silent":false,"stop_on_error":true,"store_history":true,"user_expressions":{}},"header":{"date":"2017-12-21T22:38:45.127809Z","msg_id":"A0AEA79A8901419383F45E8CBCA9CBDE","msg_type":"execute_request","session":"4CA10DA81FC54D108B6B61AD24761A8E","username":"username","version":"5.2"},"metadata":{},"parent_header":{}}'
  JuniperKernel:::client_exec_request(jclient, payload)
  execReply <- jsonlite::fromJSON(JuniperKernel:::client_exec_reply(jclient))
  msgs <- check_iopub_messages("execute_request")

  checkTrue(execReply$content$status=="ok")
  checkTrue(execReply$header$msg_type=="execute_reply")
  checkTrue(execReply$parent_header$msg_type=="execute_request")

  # this time there should be at least 4 messages:
  #  + 1 busy
  #  + 1 re-broadcast
  #  + n more messages for each stdout message
  #     - R will sink stdout to a TCP socket; and a single
  #       "print" statement may end up being split over
  #       multiple iopub messages
  #     - As a corollary, multiple "print"s may be combined
  #       into a single TCP send from R.
  #  + 1 idle
  checkTrue(length(msgs)>=4L)
  rebroad <- jsonlite::fromJSON(msgs[2L])
  checkTrue(rebroad$content$code=="for(i in 1:10) {\n print(paste0('hello ', i)) \n }")
  checkTrue(rebroad$header$msg_type=="execute_input")
  checkTrue(rebroad$parent_header$msg_type=="execute_request")

  # the next "n" messages must all be stdout; recombine
  # them into the single print that was executed
  e <- new.env(parent=emptyenv())
  e$txt <- ""
  lapply(3L:(length(msgs)-1L),
  function(i, e) {
    outmsg <- jsonlite::fromJSON(msgs[i])
    checkTrue(outmsg$content$name=="stdout")
    checkTrue(outmsg$parent_header$msg_type=="execute_request")
    e$txt <- paste0(e$txt, outmsg$content$text)
  },e=e)

  txt <- strsplit(e$txt, '\n')[[1L]]
  checkTrue(length(txt)==10L)
  invisible(lapply(1:10, function(i) {
    checkTrue(txt[i]==paste0("[1] \"hello ", i, "\""))
  }))
}