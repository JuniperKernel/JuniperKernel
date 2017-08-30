#ifndef juniper_juniper_jupyter_H
#define juniper_juniper_jupyter_H

#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Rcpp.h>

// Here's where we read and write messages using the
// jupyter protocol. Since the protocol is actually
// serialized python dicts (a.k.a. JSON), we can just
// parse the message data with our favorite json parser!
// The message contains all of the routing information, use
// it to invoke callbacks in R to handle the actual processing
// of the request.
class RequestServer {
  public:
    static void serve(const zmq::multipart_t& request, const zmq::socket_t& channel) {

    }

  private:
    struct `
    // Keep a hash-table for routing requests
    using const_zmulti_t = const zmq::multipart_t&;
    typedef std::map<std::string, void(*)(const_zmulti_t msg)> request_map_t;
    request_map_t _handlers = {
      {"kernel_request_info", [](const_zmulti_t msg){}}
    };
};

#endif // ifndef juniper_juniper_jupyter_H
