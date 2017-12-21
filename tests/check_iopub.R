# Check that the 'busy' and 'idle' messages are published
# Also count up all intermediate messages and return them
# in a sensible structure for individual test checks.
check_status_messages <- function() {
  busy <- jsonlite::fromJSON(JuniperKernel:::iopub_recv(jclient))


}