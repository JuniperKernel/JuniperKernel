#include <string>
#include <zmq.hpp>

void start_hb_thread(const zmq::context_t& ctx, const std::string endpoint, const std::string inproc_sig);
void start_io_thread(const zmq::context_t& ctx, const std::string endpoint, const std::string inproc_sig, const std::string inproc_pub);