#ifndef juniper_juniper_requests_H
#define juniper_juniper_requests_H

#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Rcpp.h>
#include <juniper/sockets.h>
#include <juniper/juniper.h>
#include <juniper/jmessage.h>


// Every incoming message has a type, which tells the server which handler
// the message should be passed to for further execution.
class RequestServer {
  public:
    RequestServer(zmq::context_t& ctx, const std::string& key) {
      _key=key;
      // these are internal routing sockets that push messages (e.g.
      // poison pills, results, etc.) to the heartbeat thread and
      // iopub thread.
      _inproc_pub = listen_on(ctx, INPROC_PUB, zmq::socket_type::pub);
      _inproc_sig = listen_on(ctx, INPROC_SIG, zmq::socket_type::pub);
    }

    ~RequestServer() {
      Rcpp::Rcout << "request server shutdown" << std::endl;
      _inproc_sig->setsockopt(ZMQ_LINGER, 0); delete _inproc_sig;
      _inproc_pub->setsockopt(ZMQ_LINGER, 0); delete _inproc_pub;
    }

    void serve(zmq::multipart_t& request) {
      JMessage jm = JMessage::read(request, _key);
      busy(jm);
      // handle message
      info_request();
      idle(jm);
    }

  private:
    // hmac key
    std::string _key;
    // inproc sockets
    zmq::socket_t* _inproc_pub;
    zmq::socket_t* _inproc_sig;

    // Keep a hash-table for routing requests
    using const_zmulti_t = const zmq::multipart_t&;
    typedef std::map<std::string, void(*)(const_zmulti_t msg)> request_map_t;
    request_map_t _handlers = {
      {"kernel_info_request", [](const_zmulti_t msg){}}
    };

    void info_request() {
      Rcpp::Rcout << "Attempting to get the JuniperKernel env" << std::endl;
      Rcpp::Environment jk("package:JuniperKernel");
            Rcpp::Rcout << "CRUNK1" << std::endl;

      Rcpp::Function kernel_request_info = jk["kernel_info_request"];
            Rcpp::Rcout << "CRUNK2" << std::endl;

      Rcpp::List res = kernel_request_info();
      Rcpp::Rcout << "CRUNK3" << std::endl;
      json j = JMessage::from_list_r(res);
      Rcpp::Rcout << "res: " << j.dump() <<  std::endl;
    }

    void iopub(const JMessage& parent, const std::string& msg_type, const json& content) {
      JMessage::reply(parent, msg_type, content).send(*_inproc_pub);
    }
    void busy(const JMessage& parent) { iopub(parent, "status", {{"execution_state", "busy"}}); }
    void idle(const JMessage& parent) { iopub(parent, "status", {{"execution_state", "idle"}}); }
};

#endif // ifndef juniper_juniper_requests_H
