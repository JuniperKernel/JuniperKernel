#ifndef juniper_juniper_juniper_H
#define juniper_juniper_juniper_H

#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <json.hpp>
#include <juniper/conf.h>
#include <juniper/sockets.h>
#include <juniper/background.h>
#include <juniper/requests.h>
#include <juniper/external.h>

class JuniperKernel {
  public:
    const RequestServer* _request_server;
    JuniperKernel(const config& conf):
      _ctx(new zmq::context_t(1)),

      // these are the 3 incoming Jupyter channels
      _stdin(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
      _hbport(conf.hb_port),
      _ioport(conf.iopub_port),
      _shellport(conf.shell_port),
      _cntrlport(conf.control_port),
      _key(conf.key),
      _sig_scheme(conf.signature_scheme) {
        _request_server = new RequestServer(*_ctx, _key);
        char sep = (conf.transport=="tcp") ? ':' : '-';
        _endpoint = conf.transport + "://" + conf.ip + sep;

        // socket setup
        init_socket(_stdin, _endpoint + conf.stdin_port);
    }

    static JuniperKernel* make(const std::string& connection_file) {
      config conf = config::read_connection_file(connection_file);
      JuniperKernel* jk = new JuniperKernel(conf);
      return jk;
    }

    // start the background threads
    // called as part of the kernel boot sequence
    void start_bg_threads() {
      _hbthread = start_hb_thread(*_ctx, _endpoint + _hbport);
      _iothread = start_io_thread(*_ctx, _endpoint + _ioport);
    }

    // runs in the main the thread, polls shell and controller
     void run() const {
       zmq::socket_t* cntrl = listen_on(*_ctx, _endpoint + _cntrlport, zmq::socket_type::router);
       zmq::socket_t* shell = listen_on(*_ctx, _endpoint + _shellport, zmq::socket_type::router);
       const RequestServer& server = *_request_server;
       const std::string key = _key;
       std::function<bool()> handlers[] = {
         [&cntrl, &key, &server]() {
           zmq::multipart_t msg;
           msg.recv(*cntrl);
           server.serve(msg, *cntrl);
           return true;
         },
         [&shell, &key, &server]() {
           zmq::multipart_t msg;
           msg.recv(*shell);
           server.serve(msg, *shell);
           return true;
         }
       };
       poll(*_ctx, std::move((zmq::socket_t* []){cntrl, shell}), handlers, 2);
     }

    ~JuniperKernel() {
      // set linger to 0 on all sockets; destroy sockets; finally destoy ctx
      _request_server->shutdown(); // try to send a signal out again
      _hbthread.join();
      _iothread.join();

      if( _request_server )
        delete _request_server;

      if( _ctx ) {
        _stdin     ->setsockopt(ZMQ_LINGER, 0); delete _stdin;
        delete _ctx;
      }
    }

  private:
    // context is shared by all threads, cause there
    // ain't no GIL to stop us now! ...we can build this thing together!
    zmq::context_t* const _ctx;

    // jupyter stdin
    zmq::socket_t*  const _stdin;

    //misc
    std::string _endpoint;
    const std::string _hbport;
    const std::string _ioport;
    const std::string _shellport;
    const std::string _cntrlport;
    const std::string _key;
    const std::string _sig_scheme;

    std::thread _hbthread;
    std::thread _iothread;
};

#endif // ifndef juniper_juniper_juniper_H
