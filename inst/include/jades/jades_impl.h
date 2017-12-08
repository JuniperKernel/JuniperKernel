#ifndef jades_jades_jades_impl_H
#define jades_jades_jades_impl_H

#include <iostream>
#include <jades/jades.hpp>
#include <xeus/xguid.hpp>

namespace JadesInterpreter {
  JadesInterpreter(zmq::context_t& ctx):
    _ctx(&ctx),
    _jk("package:JadesKernel"),
    _connected(0)
      {
        // these are internal routing sockets that push messages (e.g.
        // poison pills, results, etc.) to the heartbeat thread and
        // iopub thread.
        _inproc_pub = listen_on(ctx, INPROC_PUB, zmq::socket_type::pub);
        _inproc_sig = listen_on(ctx, INPROC_SIG, zmq::socket_type::pub);

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
        _stream_out = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
        _stream_err = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
        _stream_out_port = read_port(_stream_out);
        _stream_err_port = read_port(_stream_err);
        std::thread sout = stream_thread(_stream_out, true, _connected );
        std::thread serr = stream_thread(_stream_err, false, _connected);
        sout.detach();
        serr.detach();
    }

  ~JadesInterpreter() {
    _inproc_sig->setsockopt(ZMQ_LINGER, 0); delete _inproc_sig;
    _inproc_pub->setsockopt(ZMQ_LINGER, 0); delete _inproc_pub;
  }
  void JadesInterpreter::configure_impl() {
    auto handle_comm_opened = [](xeus::xcomm&& comm, const xeus::xmessage&) {
      Rcpp::Rcout << "Comm opened for target: " << comm.target().name() << std::endl;
    };
    comm_manager().register_comm_target("jades_target", handle_comm_opened);
    using function_type = std::function<void(xeus::xcomm&&, const xeus::xmessage&)>;
  }

  xjson R_perform(const std::string& request_type, xjson& req) {
    req["stream_out_port"] = _stream_out_port;  // stitch the stdout port into the client request
    req["stream_err_port"] = _stream_err_port;  // stitch the stderr into the client request
    Rcpp::Function handler = _jk[request_type];
    Rcpp::Function do_request = _jk["doRequest"];
    Rcpp::List res = do_request(Rcpp::wrap(handler), from_json_r(req));
    xjson jres = from_list_r(res);
    while( _connected.load() ){}
    return jres;
  }

  xjson JadesInterpreter::execute_request_impl(
    int execution_counter,
    const std::string& code,
    bool silent,
    bool store_history,
    const xjson_node* /* user_expressions */,
    bool allow_stdin) {
      xjson req;
      req["code"] = code;
      req["execution_counter"] = execution_counter;
      return R_perform("execute_request", req);
  }

  xjson JadesInterpreter::complete_request_impl(
    const std::string& code,
    int cursor_pos) {
      xjson req;
      req["code"] = code;
      req["cursor_pos"] = cursor_pos;
      return R_perform("complete_request", req);
  }

  xjson JadesInterpreter::inspect_request_impl(
    const std::string& code,
    int cursor_pos,
    int detail_level) {
      xjson req;
      req["code"] = code;
      req["cursor_pos"] = cursor_pos;
      req["detail_level"] = detail_level;
      return R_perform("inspect_request", req);
  }

  xjson JadesInterpreter::history_request_impl(const xhistory_arguments& args) {
    xjson req;
    Rcpp::Rcout << "history request is unimpl." << std::endl;
    return R_perform("history_request", req);
  }

  xjson JadesInterpreter::is_complete_request_impl(const std::string& code) {
    xjson req;
    req["code"] = code;
    return R_perform("is_complete_request", req);
  }

  xjson JadesInterpreter::kernel_info_request_impl() {
    xjson req;
    return R_perform("kernel_info_request", req);
  }

  void JadesInterpreter::input_reply_impl(const std::string& value) {
    Rcpp::Rcout << "Received input_reply" << std::endl;
    Rcpp::Rcout << "value: " << value << std::endl;
  }

  std::thread stream_thread(zmq::socket_t* sock, const bool out, std::atomic<int>& connected) const {
    const JadesInterpreter& js = *this;
    std::thread out_thread([&js, sock, out, &connected]() {  // full fence membar
      short conn=false;
      std::function<bool()> handlers[] = {
        [sock, &js, out, &connected, &conn]() {
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
          if( msg[1].size()==0 ) {
            if( !conn++ )
              connected.fetch_add(1);
              else {
                conn=false;
                connected.fetch_sub(1);
              }
            return true;
          }
          // read the message and publish to stdout
          if( out ) js.stream_out(msg_t_to_string(msg[1]));
          else      js.stream_err(msg_t_to_string(msg[1]));
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
}

#endif // ifndef jades_jades_jades_impl_H