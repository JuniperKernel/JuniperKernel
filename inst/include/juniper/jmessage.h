// Copyright (C) 2017  Spencer Aiello
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
#include <json.hpp>
#include <hmac.h>
#include <ctime>
#include <iostream>
#include <sha256.h>
#include <juniper/external.h>
#include <Rcpp.h>

#define VERSION "5.2"
static const std::string DELIMITER = "<IDS|MSG>";

using nlohmann::json;
using namespace std::chrono;
// Here's where we read and write messages using the jupyter protocol.
// Since the protocol is actually serialized python dicts (a.k.a. JSON),
// we are stuck with json (de)serialization.
class JMessage {
  
public:
  std::string _key; // used for creating the hmac signature
  static JMessage read(zmq::multipart_t& request, const std::string& key) {
    JMessage jm;
    jm._key = key;
    return jm.read_ids(request).read_hmac(request).read_body(request);
  }

  static zmq::multipart_t reply(const JMessage& parent, const std::string& msg_type, const json& content, const json& metadata=json({})) {
    JMessage jm;
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
    jm._msg["metadata"] = metadata;
    jm._msg["content"] = content;
    return jm.to_multipart_t();
  }

  json get() const { return _msg; }
  std::vector<std::string> ids() { return _ids; }
  // from https://stackoverflow.com/questions/9527960/how-do-i-construct-an-iso-8601-datetime-in-c
  static std::string now() {
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
    // this will work too, if your compiler doesn't support %F or %T:
    //strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    std::stringstream s;
    s << buf;
    return s.str();
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

    // validate
    std::string hmac2dig = hmac<SHA256>(data.str(), _key);
    // Rcpp::Rcout << "hmac2dig: " << hmac2dig << "; actual: " << _hmac << std::endl;
    // Rcpp::Rcout << "hmac2dig.compare(actual)= " << hmac2dig.compare(_hmac) << std::endl;
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

  static std::string read_str(const zmq::message_t& msg) {
    std::stringstream ss;
    const char* buf = msg.data<const char>();
    size_t buflen = msg.size();
    for(size_t i=0; i<buflen; ++i)
      ss << static_cast<char>(buf[i]);
    return ss.str();
  }
  
  static zmq::message_t to_msg(const std::string& s) {
    return zmq::message_t(s.begin(), s.end());
  }

  // JMessage (this) -> multipart_t
  zmq::multipart_t to_multipart_t() {
    zmq::multipart_t multi_msg;
    
    // add IDS
    for(std::string id: _ids) {
      multi_msg.add(to_msg(id));
    }

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
    return multi_msg;
  }
};

#endif // ifndef juniper_juniper_jmessage_H
