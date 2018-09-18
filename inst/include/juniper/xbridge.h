// Copyright (C) 2017-2018  Spencer Aiello
//
// This file is part of JuniperKernel.
//
// JuniperKernel is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// JuniperKernel is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with JuniperKernel.  If not, see <http://www.gnu.org/licenses/>.
#ifndef juniper_juniper_xbridge_H
#define juniper_juniper_xbridge_H

#define XEUS_EXPORTS

#include <nlohmann/json.hpp>
#include <xeus/xjson.hpp>
#include <xeus/xcomm.hpp>
#include <xeus/xinterpreter.hpp>
#include <juniper/juniper.h>
#include <juniper/jmessage.h>
#include <juniper/utils.h>

// mock interpreter
using namespace xeus;
xinterpreter::xinterpreter() {}
xcomm_manager::xcomm_manager(xkernel_core* kernel) {(void)p_kernel;} // void out; unused private var compiler warning
class xmock: public xinterpreter {
public:
  JuniperKernel* _jk;
  using base_type = xinterpreter;
  xmock() { register_comm_manager(new xcomm_manager()); }
  virtual ~xmock() = default;
  xmock(const xmock&) = delete;
  xmock& operator=(const xmock&) = delete;
  xmock(xmock&&) = delete;
  xmock& operator=(xmock&&) = delete;

private:
  void configure_impl() override{throw("unimpl");}
  inline xjson execute_request_impl(int,const std::string&,bool,bool,const xjson_node*,bool) override {throw("unimpl");}
  inline xjson complete_request_impl(const std::string&,int) override { throw("unimpl"); }
  inline xjson inspect_request_impl(const std::string&,int,int) override{throw("unimpl");}
  inline xjson history_request_impl(const xhistory_arguments&) override{throw("unimpl");}
  inline xjson is_complete_request_impl(const std::string&) override{throw("unimpl");}
  inline xjson kernel_info_request_impl() override{throw("unimpl");}
  inline void input_reply_impl(const std::string&) override{throw("unimpl");}
};

extern xmock* _xm;
extern SEXP R_xm;
// setup the mocked xeus interpreter
inline xinterpreter& xeus::get_interpreter() { return *_xm; }
xmock& get_xmock() { return static_cast<xmock&>(get_interpreter()); }

void xinterpreter::display_data(xjson data, xjson metadata, xjson transient) {
  json content = {{"data", data}, {"metadata", metadata}, {"transient", transient}};
  get_xmock()._jk->_request_server->iopub("display_data", content);
}

void xinterpreter::register_comm_manager(xcomm_manager* mgr) {
  p_comm_manager=mgr;
}

// xmessage impl
xmessage_base::xmessage_base(xjson header, xjson parent_header,
														 xjson metadata, xjson content, buffer_sequence buffers): 
  m_header(header),
  m_parent_header(parent_header),
  m_metadata(metadata),
  m_content(content),
  m_buffers(std::move(buffers)){}
const xjson& xmessage_base::content() const {return m_content; }
const xjson& xmessage_base::metadata() const { return m_metadata; }

xmessage::xmessage(const guid_list& zmq_id, xjson header, xjson parent_header,
									 xjson metadata, xjson content, buffer_sequence buffers):
  xmessage_base(std::move(header),
								std::move(parent_header),
								std::move(metadata),
								std::move(content),
								std::move(buffers)){ m_zmq_id=zmq_id;}

// xcomm impl
void xtarget::publish_message(const std::string& msg_type, xjson metadata, xjson content, buffer_sequence buffers) const {
  get_xmock()._jk->_request_server->iopub(msg_type, std::move(content), std::move(metadata), std::move(buffers));
}
xjson xcomm_manager::get_metadata() const { return {{"started", now()}}; }
void xcomm_manager::register_comm_target(const std::string& target_name, const target_function_type& callback){ 
  m_targets[target_name] = xtarget(target_name, callback, this);
}
void xcomm_manager::unregister_comm_target(const std::string& target_name) {
  m_targets.erase(target_name); 
}
void xcomm_manager::register_comm(xguid id, xcomm* comm) {
  m_comms[id] = comm;
}
void xcomm_manager::unregister_comm(xguid id) { 
  m_comms.erase(id);
}

template<class K, class T>
typename std::map<K,T>::iterator pos(std::map<K, T> m, K key, std::string type) {
  auto position = m.find(key);
  if( position==m.end() ) {
    std::stringstream ss;
    ss << "No such " << type << " registered: " << key;
    Rcpp::Rcout << ss.str() << std::endl;
  }
  return position;
}

xmessage to_xmessage(const json& request, const xmessage::guid_list& zmq_id, buffer_sequence buffers) {
  return xmessage(zmq_id, std::move(request["header"]),
									std::move(request["parent_header"]),
									std::move(request["metadata"]),
									std::move(request["content"]),
									std::move(buffers));
}

void xcomm_manager::comm_open(const xmessage& request) {
  const xjson& content = request.content();
  std::string target_name = content["target_name"];
  if( (m_targets.find(target_name))==m_targets.end() ) {
    Rcpp::Rcout << "No such target registered: " << target_name << std::endl;
    return;
  }

  auto position = pos(m_targets, target_name, "target");
  xtarget& target = position->second;
  xguid id = content["comm_id"];
  xcomm comm = xcomm(&target, id);
  target(std::move(comm), request);
}

void xcomm_manager::comm_close(const xmessage& request) { 
  const xjson& content = request.content();
  xguid id = content["comm_id"];
  auto position = pos(m_comms, id, "comm");
  position->second->handle_close(request);
  m_comms.erase(id);
}

void xcomm_manager::comm_msg(const xmessage& request) {
  const xjson& content = request.content();
  xguid id = content["comm_id"];
  auto position = pos(m_comms, id, "comm");
  position->second->handle_message(request);
}

xguid xeus::new_xguid() { return JMessage::uniq_id(); }
#endif // #ifndef juniper_juniper_xbridge_H
