#ifndef juniper_juniper_jupyter_H
#define juniper_juniper_jupyter_H

#include <fstream>
#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Rcpp.h>
#include <json.hpp>
#include <hmac.h>
#include <sha256.h>

using nlohmann::json;

// Here's where we read and write messages using the jupyter protocol.
// Since the protocol is actually serialized python dicts (a.k.a. JSON),
// we can just parse the message data with our favorite json parser!
// The message contains all of the routing information, use it to invoke
// callbacks in R to handle the actual processing of the request.
class RequestServer {
  const static int IDS     =0;
  const static int DELIM   =1;
  const static int HMAC_SIG=2;
  const static int HEADER  =3;
  const static int PHEADER =4;
  const static int METADATA=5;
  const static int CONTENT =6;

  public:
    static void serve(const zmq::multipart_t& request, const zmq::socket_t& channel, const std::string& key) {
      msg_t msg;
//      msg.ids = msg_t::read_str(request.at(IDS));
      msg.hmac = msg_t::read_str(request.at(HMAC_SIG));
      std::stringstream data;
      msg.header   = msg_t::read_json(request.at(HEADER  ), data);
      msg.pheader  = msg_t::read_json(request.at(PHEADER ), data);
      msg.metadata = msg_t::read_json(request.at(METADATA), data);
      msg.content  = msg_t::read_json(request.at(CONTENT ), data);

      std::string hmac2dig = hmac<SHA256>(data.str(), key);
      
      Rcpp::Rcout << "hmac2dig: " << hmac2dig << "; actual: " << msg.hmac << std::endl;
      Rcpp::Rcout << "hmac2dig.compare(actual)= " << hmac2dig.compare(msg.hmac) << std::endl;
      if( hmac2dig!=msg.hmac )
        throw("bad hmac digest");
    }

  private:
    struct msg_t {
      json header;
      json pheader;
      json metadata;
      json content;
      
      std::string hmac;
      std::string ids;
      
      static json read_json(const zmq::message_t& msg, std::stringstream& data) {
        std::string s = read_str(msg);
        data << read_str(msg);
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
    };


    // Keep a hash-table for routing requests
    using const_zmulti_t = const zmq::multipart_t&;
    typedef std::map<std::string, void(*)(const_zmulti_t msg)> request_map_t;
    request_map_t _handlers = {
      {"kernel_request_info", [](const_zmulti_t msg){}}
    };
};

#endif // ifndef juniper_juniper_jupyter_H
