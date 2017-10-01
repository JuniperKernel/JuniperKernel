#ifndef juniper_juniper_requests_H
#define juniper_juniper_requests_H

#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Rcpp.h>
#include <juniper/sockets.h>
#include <juniper/conf.h>
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

    void stream_out(const std::string& o) const { iopub("stream", {{"name", "stdout"}, {"text", o}}); }
    void stream_err(const std::string& e) const { iopub("stream", {{"name", "stderr"}, {"text", e}}); }
    void rebroadcast_input(const std::string& input, const int count) const {
       iopub("execute_input", {{"code", input}, {"execution_count", count}});
    }
    void execute_result(const json& data) const { iopub("execute_result", data); }
    void display_data(const json& data) const   { iopub("display_data"  , data); }
    void shutdown() const { zmq::message_t m(0); _inproc_sig->send(m); }
    void iopub(const std::string& msg_type, const json& content, const json& metadata=json({})) const {
      JMessage::reply(_cur_msg, msg_type, content, metadata).send(*_inproc_pub);
    }
    const RequestServer& busy() const { iopub("status", {{"execution_state", "busy"}}); return *this; }
    const RequestServer& idle() const { iopub("status", {{"execution_state", "idle"}}); return *this; }

  private:
    zmq::context_t* const _ctx;
    const std::string _key;   // hmac key
    zmq::socket_t* _inproc_pub;
    zmq::socket_t* _inproc_sig;
    const Rcpp::Environment _jk;

    // R functions are pulled out of the JuniperKernel package environment.
    // For each msg_type, there's a corresponding R function with that name.
    // Example: for msg_type "kernel_info_request", there's an R method
    // called "kernel_info_request" that's exported in the JuniperKernel.
    const RequestServer& handle(zmq::socket_t& sock) const {
      json req = _cur_msg.get();
      std::string msg_type = req["header"]["msg_type"];
//      Rcpp::Rcout << "Handling message type: " << msg_type << std::endl;
      // R sinks stdout/stderr to socketConnection, which is being listened
      // to on a ZMQ_STREAM socket. Polling of this connection happens in a
      // separate thread so that stdout/stderr can be streamed to IOPub 
      // asynchronously. This socket is created in the main thread and 
      // migrated to a polling thread. Why? R needs to know the port--which
      // is chosen by the OS--to form the socketConnection: because R is 
      // executing in the main thread there is the option to block the main
      // thread until it handshakes with the spawned thread via inproc 
      // transports; or migrate a socket to a new thread. Option 2 is  
      // simpler and totally legal according to the ZMQ docs since the socket
      // creation and port scraping happens before the polling thread is spawned.
      zmq::socket_t* stream_out = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
      zmq::socket_t* stream_err = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
      req["stream_out_port"] = read_port(stream_out);  // stitch the stdout port into the client request
      req["stream_err_port"] = read_port(stream_err);  // stitch the stderr into the client request
      Rcpp::Function handler = _jk[msg_type];
      Rcpp::Function do_request = _jk["doRequest"];
      // boot listener threads; execute request; join listeners
      std::thread sout = stream_thread(stream_out, true );
      std::thread serr = stream_thread(stream_err, false);
      Rcpp::List res = do_request(Rcpp::wrap(handler), from_json_r(req));
      serr.join();  // stream_err is now destroyed
      sout.join();  // stream_out is now destroyed
      json jres = from_list_r(res);
      // comms don't reply (except for comm_info_request)
      if( msg_type=="comm_open" || msg_type=="comm_close" || msg_type=="comm_msg" )
        return *this;
      JMessage::reply(_cur_msg, jres["msg_type"], jres["content"]).send(sock);
      if( msg_type.compare("shutdown_request")==0 ) {
        shutdown();
        Rcpp::Environment base("package:base");
        Rcpp::Function quit = base["quit"];
        quit("no");
      }
      return *this;
    }

    std::thread stream_thread(zmq::socket_t* sock, const bool out=true) const {
      const RequestServer& rs = *this;
      std::thread out_thread([&rs, sock, out]() {  // full fence membar
        short connected=false;
        std::function<bool()> handlers[] = {
          [sock, &rs, out, &connected]() {
            zmq::multipart_t msg;
            msg.recv(*sock);

            // connect/disconnect logic
            // in either case it's a two-frame multipart message:
            //   frame 1: identity
            //   frame 2: empty
            // the first time getting these two frames means we're
            // getting connected to by a socket having identity found in
            // frame1. the second time means the socket having that
            // identity disconnected. Our expectation is that we shall
            // only ever have a single connection; and when it dies, we also
            // die (AKA signal death to the poller by returning false).
            if( msg[1].size()==0 )
              return !connected++;

            // read the message and publish to stdout
            if( out ) rs.stream_out(msg_t_to_string(msg[1]));
            else      rs.stream_err(msg_t_to_string(msg[1]));
            return true;
          }
        };
        poll(*rs._ctx, std::move((zmq::socket_t* []){sock}), handlers, 1);
      });
      return out_thread;
    }
    static int read_port(zmq::socket_t* sock) {
      char endpoint[32];
      size_t sz = sizeof(endpoint); 
      sock->getsockopt(ZMQ_LAST_ENDPOINT, &endpoint, &sz);
      std::string ep(endpoint);
      std::string port(ep.substr(ep.find(":", ep.find(":")+1)+1));
      return stoi(port);
    }
};

#endif // ifndef juniper_juniper_requests_H
