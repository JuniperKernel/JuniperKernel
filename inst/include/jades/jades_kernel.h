#ifndef jades_jades_jades_kernel_H
#define jades_jades_jades_kernel_H

#include <xeus/xeus.hpp>
#include <xeus/xinterpreter.hpp>
#include <xeus/xkernel_configuration.hpp>
#include <xeus/xserver.hpp>
#include <memory>
#include <string>

namespace xeus {
  class XEUS_API JadesKernel: xkernel {
    zmq::context_t* const _ctx;
    JadesKernel(const xconfiguration& config,
                const std::string& user_name,
                interpreter_ptr interpreter,
                server_builder builder,
                zmq::context_t& ctx
                ):
      xkernel(config, user_name, interpreter, builder),
      _ctx(&ctx){}

    ~JadesKernel() {
      if( _ctx )
        delete _ctx;
    }

    void xkernel::start() {
      std::string kernel_id = new_xguid();
      std::string session_id = new_xguid();

      using authentication_ptr = xkernel_core::authentication_ptr;
      authentication_ptr auth = make_xauthentication(m_config.m_signature_scheme, m_config.m_key);

      zmq::multipart_t start_msg;
      build_start_msg(auth, kernel_id, m_user_name, session_id, start_msg);

      server_ptr server = m_builder(*_ctx, m_config);

      xkernel_core core(kernel_id, m_user_name, session_id,
                        std::move(auth), server.get(), p_interpreter.get());

      p_interpreter->configure();
      server->start(start_msg);
    }
  }
}

#endif // #ifndef jades_jades_jades_kernel_H