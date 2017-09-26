#ifndef juniper_juniper_xbridge_H
#define juniper_juniper_xbridge_H

#include <json.hpp>
#include <xeus/xcomm.hpp>
#include <xeus/xinterpreter.hpp>
#include <juniper/juniper.h>

// mock interpreter
using namespace xeus;
class JuniperKernel;
class xmock : public xinterpreter {
public:
  using base_type = xinterpreter;
  xmock();
  xmock(JuniperKernel* jk): _jk(jk) {}
  virtual ~xmock() = default;
  xmock(const xmock&) = delete;
  xmock& operator=(const xmock&) = delete;
  xmock(xmock&&) = delete;
  xmock& operator=(xmock&&) = delete;
  void display_data(xjson data, xjson metadata, xjson transient);
  void update_display(xjson data, xjson metadata, xjson transient);

private:
  JuniperKernel* _jk;
  void configure_impl() override{throw("unimpl");}
  inline xjson execute_request_impl(int,const std::string&,bool,bool,const xjson_node*,bool) override {throw("unimpl");}
  inline xjson complete_request_impl(const std::string&,int) override { throw("unimpl"); }
  inline xjson inspect_request_impl(const std::string&,int,int) override{throw("unimpl");}
  inline xjson history_request_impl(const xhistory_arguments&) override{throw("unimpl");}
  inline xjson is_complete_request_impl(const std::string&) override{throw("unimpl");}
  inline xjson kernel_info_request_impl() override{throw("unimpl");}
  inline void input_reply_impl(const std::string&) override{throw("unimpl");}
  xcomm_manager m_comm_manager;
};

#endif // #ifndef juniper_juniper_xbridge_H
