#ifndef juniper_juniper_xbridge_H
#define juniper_juniper_xbridge_H

#include <json.hpp>
#include <xeus/xjson.hpp>
#include <xeus/xcomm.hpp>
#include <xeus/xinterpreter.hpp>
#include <juniper/juniper.h>
#include <juniper/jmessage.h>

// mock interpreter
using namespace xeus;
xinterpreter::xinterpreter() {}
xcomm_manager::xcomm_manager(xkernel_core* kernel) {}
class xmock: public xinterpreter {
public:
  JuniperKernel* _jk;
  using base_type = xinterpreter;
  xmock() { register_comm_manager(_comm_mgr=new xcomm_manager()); }
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
  xcomm_manager* _comm_mgr;
};

xmock& get_xmock() { return static_cast<xmock&>(get_interpreter()); }

void xinterpreter::display_data(xjson data, xjson metadata, xjson transient) {
  json content = {{"data", data}, {"metadata", metadata}, {"transient", transient}};
  get_xmock()._jk->_request_server->iopub("display_data", content);
}

void xinterpreter::register_comm_manager(xcomm_manager* mgr) {
  p_comm_manager=mgr;
}

// xmessage impl
xmessage_base::xmessage_base(xjson header, xjson parent_header, xjson metadata, xjson content): 
  m_header(header),
  m_parent_header(parent_header),
  m_metadata(metadata),
  m_content(content){}
const xjson& xmessage_base::content() const {return m_content; }

xmessage::xmessage(const guid_list& zmq_id, xjson header, xjson parent_header, xjson metadata, xjson content):
  xmessage_base(header,parent_header,metadata,content){ m_zmq_id=zmq_id;}

// xcomm impl
void xtarget::publish_message(const std::string& msg_type, xjson metadata, xjson content) const {
  get_xmock()._jk->_request_server->iopub(msg_type, content, metadata);
}
xjson xcomm_manager::get_metadata() const { return {{"started", JMessage::now()}}; }
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
    throw std::runtime_error(ss.str());
  }
  return position;
}

xmessage to_xmessage(const json& request, const xmessage::guid_list& zmq_id) {
  return xmessage(zmq_id, request["header"], request["parent_header"], request["metadata"], request["content"]);
}

void xcomm_manager::comm_open(const xmessage& request) {
  const xjson& content = request.content();
  std::string target_name = content["target_name"];
  auto position = pos(m_targets, target_name, "target");
  xtarget& target = position->second;
  xguid id = content["comm_id"];
  xcomm comm = xcomm(&target, id);
  target(comm, request);
  comm.open(get_metadata(), content["data"]);
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

xguid xeus::new_xguid() { return JMessage::uuid(); }
#endif // #ifndef juniper_juniper_xbridge_H
