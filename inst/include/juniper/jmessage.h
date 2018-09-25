//Copyright (C) 2017-2018  Spencer Aiello
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
#ifndef juniper_juniper_jmessage_H
#define juniper_juniper_jmessage_H

#include <atomic>
#include <fstream>
#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <nlohmann/json.hpp>
#include <brumme/hmac.h>
#include <brumme/sha256.h>
#include <iostream>
#include <juniper/external.h>
#include <juniper/utils.h>
#include <Rcpp.h>

#define VERSION "5.2"
static const std::string DELIMITER = "<IDS|MSG>";

using nlohmann::json;
using namespace std::chrono;
using buffer_sequence = std::vector<zmq::message_t>;
// Here's where we read and write messages using the jupyter protocol.
// Since the protocol is actually serialized python dicts (a.k.a. JSON),
// we are stuck with json (de)serialization.
class JMessage {
  
public:
  std::string _key; // used for creating the hmac signature
	JMessage() {}
 	JMessage(buffer_sequence buffers): _b0(std::move(buffers)) {}
	JMessage& operator=(JMessage&& jm) = default;
	JMessage(JMessage&& jm):
		_key(jm._key),
		_msg(jm._msg),
		_b0(std::move(jm._b0)),
		_hmac(jm._hmac),
		_ids(jm._ids){}

	static JMessage read(zmq::multipart_t& request, const std::string& key) {
		std::vector<zmq::message_t> buffers;
    JMessage jm(std::move(buffers));
    jm._key = key;
    return std::move(jm.read_ids(request).read_hmac(request).read_body(request));
  }
  static zmq::multipart_t reply(const JMessage& parent, const std::string& msg_type,
																const json& content, const json& metadata=json({})) {
		std::vector<zmq::message_t> buffers;
		return reply(parent, msg_type, std::move(content), std::move(metadata), std::move(buffers));
	}
  static zmq::multipart_t reply(const JMessage& parent, const std::string& msg_type,
																const json& content, const json& metadata,
																buffer_sequence buffers) {
    JMessage jm(std::move(buffers));
    jm._key = parent._key;
    // construct header
    jm._msg["header"]["msg_id"] = uniq_id();
    jm._msg["header"]["username"] = parent._msg["header"]["username"];
    jm._msg["header"]["session"] = parent._msg["header"]["session"];
    jm._msg["header"]["date"] = now();
    jm._msg["header"]["msg_type"] = msg_type;
    jm._msg["header"]["version"] = VERSION;

    jm._msg["parent_header"] = parent._msg["header"];
    jm._ids = parent._ids;
    jm._msg["metadata"] = std::move(metadata);
    jm._msg["content"] = std::move(content);
    return jm.to_multipart_t();
  }

  json get() const { return _msg; }
  std::vector<std::string> ids() { return _ids; }
	std::vector<zmq::message_t>& bufs() { return _b0; }

	void dump() {
		Rcpp::Rcout << "{"                << std::endl;
		Rcpp::Rcout << "  ids: "          << std::endl;
		for( std::string id: _ids )
  		Rcpp::Rcout << "       " << id << "," << std::endl;
		Rcpp::Rcout << "  msg: "          << std::endl;
		Rcpp::Rcout << "       " << _msg  << std::endl;
		Rcpp::Rcout << "}"                << std::endl;
	}

  static long long int current_time_nanos() {
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
  }

  // smash the thread id and atomic long together
  static std::string uniq_id() {
    std::stringstream s;
    s << _ctr.fetch_add(1) << current_time_nanos() << "_" << (std::this_thread::get_id());
    return s.str();
  }

private:
  json _msg;
	std::vector<zmq::message_t> _b0;
	std::string _hmac;
  std::vector<std::string> _ids;
  static std::atomic<long long> _ctr;  // atomic counter for uniq ids

  bool not_delimiter(const zmq::message_t& m, std::string& id) {
    id=read_str(m);
    return id.compare(DELIMITER)!=0;
  }

  JMessage& read_ids(zmq::multipart_t& msg) {
    zmq::message_t fr = msg.pop();
    std::string id;
    while( fr.size()!=0 && not_delimiter(fr, id) ) {
      _ids.emplace_back(id);
      fr=msg.pop();
    }
    return *this;
  }

  JMessage& read_hmac(zmq::multipart_t& msg) {
    _hmac = read_str(msg.pop());
    return *this;
  }

  JMessage& read_body(zmq::multipart_t& msg) {
    std::stringstream data;
    _msg["header"]        = read_json(msg.pop(), data);
    _msg["parent_header"] = read_json(msg.pop(), data);
    _msg["metadata"]      = read_json(msg.pop(), data);
    _msg["content"]       = read_json(msg.pop(), data);

		while( !msg.empty() )
			_b0.emplace_back(msg.pop());

    // validate
    std::string hmac2dig = hmac<SHA256>(data.str(), _key);
    if( hmac2dig!=_hmac )
      throw("bad hmac digest");
    return *this;
  }

  // ideally read the message one time; for validation we want to cat together
  // the header, parent header, metadata, and content dicts; at the same time
  // as we read the fields of this struct, we also build up the cat'd string
  // with the data stringstream.
  static json read_json(const zmq::message_t& msg, std::stringstream& data) {
    std::string s = read_str(msg);
    data << s;
    return json::parse(s);
  }
  
  static zmq::message_t to_msg(const std::string& s) {
    return zmq::message_t(s.begin(), s.end());
  }

  // JMessage (this) -> multipart_t
  zmq::multipart_t to_multipart_t() {
    zmq::multipart_t multi_msg;
    
    // add IDS
    for(std::string id: _ids)
      multi_msg.add(to_msg(id));
    
    // now the delimiter
    multi_msg.add(to_msg(DELIMITER));
    
    // construct the hmac sig
    std::stringstream data;
    std::string header        = _msg["header"       ].dump(); data << header       ;
    std::string parent_header = _msg["parent_header"].dump(); data << parent_header;
    std::string metadata      = _msg["metadata"     ].dump(); data << metadata     ;
    std::string content       = _msg["content"      ].dump(); data << content      ;

    // serialize the signature and main message body
    std::string hmacsig = hmac<SHA256>(data.str(), _key);

    multi_msg.add(to_msg(hmacsig));
    multi_msg.add(to_msg(header));
    multi_msg.add(to_msg(parent_header));
    multi_msg.add(to_msg(metadata));
    multi_msg.add(to_msg(content));

		for(zmq::message_t& buf: _b0)
			multi_msg.add(std::move(buf));

		_b0.clear();  // put _b0 into a defined state (all elems have been moved)

    return multi_msg;
  }
};

#endif // ifndef juniper_juniper_jmessage_H
