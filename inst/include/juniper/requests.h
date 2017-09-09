#ifndef juniper_juniper_requests_H
#define juniper_juniper_requests_H

#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Rcpp.h>
#include <juniper/sockets.h>
#include <juniper/juniper.h>
#include <juniper/jmessage.h>
#include <juniper/utils.h>


// Every incoming message has a type, which tells the server which handler
// the message should be passed to for further execution.
class RequestServer {
  public:
    mutable JMessage _cur_msg;  // messages are handled 1 at a time; keep a ref
    RequestServer(zmq::context_t& ctx, const std::string& key):
      _ctx(&ctx),
      _key(key),
      _jk("package:JuniperKernel")
        {
          // these are internal routing sockets that push messages (e.g.
          // poison pills, results, etc.) to the heartbeat thread and
          // iopub thread.
          _inproc_pub = listen_on(ctx, INPROC_PUB, zmq::socket_type::pub);
          _inproc_sig = listen_on(ctx, INPROC_SIG, zmq::socket_type::pub);
        }

    ~RequestServer() {
      _inproc_sig->setsockopt(ZMQ_LINGER, 0); delete _inproc_sig;
      _inproc_pub->setsockopt(ZMQ_LINGER, 0); delete _inproc_pub;
    }

    // handle all client requests here. Every request gets the same
    // treatment:
    //  1. deserialize the incoming message and validate
    //  2. 'busy' is published over iopub
    //  3. the message is passed to a handler
    //  4. a zmq::multipart_t is returned containing the 'content' and 'msg_type' fields
    //  5. send the multipart_t over the socket
    //  6. 'idle' is published over iopub
    void serve(zmq::multipart_t& request, zmq::socket_t& sock) const {
      _cur_msg = JMessage::read(request, _key);  // read and validate
      busy().handle(sock).idle();
    }

    void stream_stdout(const std::string& o) const { iopub("stream", {{"name", "stdout"}, {"text", o}}); }
    void stream_stderr(const std::string& e) const { iopub("stream", {{"name", "stderr"}, {"text", e}}); }
    void rebroadcast_input(const std::string& input, const int count) const {
       iopub("execute_input", {{"code", input}, {"execution_count", count}});
    }

  private:
    zmq::context_t* const _ctx;
    const std::string _key;   // hmac key
    zmq::socket_t* _inproc_pub;
    zmq::socket_t* _inproc_sig;
    const Rcpp::Environment _jk;

    // R functions are pulled out of the JuniperKernel package environment.
    // For each msg_type, there's a corresponding R function with that name.
    // For example, for msg_type "kernel_info_request", there's an R method
    // called "kernel_info_request" that's exported in the JuniperKernel.
    const RequestServer& handle(zmq::socket_t& sock) const {
      std::thread sout  = stdout_thread();
      json req = _cur_msg.get();
      Rcpp::Function handler = _jk[req["header"]["msg_type"]];
      Rcpp::Function do_request = _jk["doRequest"];
      Rcpp::List res = do_request(Rcpp::wrap(handler), from_json_r(req));
      sout.join();
      json jres = from_list_r(res);
      JMessage::reply(_cur_msg, jres["msg_type"], jres["content"]).send(sock);
      return *this;
    }

    std::thread stdout_thread() const {
      const RequestServer& rs = *this;
      std::thread out_thread([&rs]() {
        zmq::socket_t* sock = new zmq::socket_t(*rs._ctx, zmq::socket_type::stream);
        sock->connect("tcp://localhost:6011");
        int connected=false;
        std::function<bool()> handlers[] = {
          [&sock, &rs, &connected]() {
            zmq::multipart_t msg;
            msg.recv(*sock);
            rs.stream_stdout(msg.str());
            if( connected++>0 && msg[1].size()==0 )
              return false; // signal done
            return true;
          }
        };
        poll(*rs._ctx, (zmq::socket_t* []){sock}, handlers, 1);
      });
      return out_thread;
    }

    void iopub(const std::string& msg_type, const json& content) const {
      JMessage::reply(_cur_msg, msg_type, content).send(*_inproc_pub);
    }
    const RequestServer& busy() const { iopub("status", {{"execution_state", "busy"}}); return *this; }
    const RequestServer& idle() const { iopub("status", {{"execution_state", "idle"}}); return *this; }
};

#endif // ifndef juniper_juniper_requests_H
