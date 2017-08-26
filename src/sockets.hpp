#include <string>
#include <zmq.hpp>

zmq::socket_t subscribe_to(zmq::context_t& context, const std::string& inproc_topic);
void init_socket(zmq::socket_t* socket, std::string endpoint);
void poller(zmq::pollitem_t items[], std::function<bool()> handlers[], int n);