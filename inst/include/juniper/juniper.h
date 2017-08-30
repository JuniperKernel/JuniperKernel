#ifndef juniper_juniper_juniper_H
#define juniper_juniper_juniper_H

#include <fstream>
#include <string>
#include <Rcpp.h>
#include <json.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#define LINGER 1000 // number of millis to linger for
#define INPROC_PUB "inproc://pub"
#define INPROC_SIG "inproc://sig"

using nlohmann::json;
struct config {
  std::string control_port;
  std::string hb_port;
  std::string iopub_port;
  std::string ip;
  std::string key;
  std::string shell_port;
  std::string signature_scheme;
  std::string stdin_port;
  std::string transport;

  void print_conf() {
    Rcpp::Rcout << "{"                                                << std::endl;
    Rcpp::Rcout << "  control_port: "     << control_port      << "," << std::endl;
    Rcpp::Rcout << "  hb_port: "          << hb_port           << "," << std::endl;
    Rcpp::Rcout << "  iopub_port: "       << iopub_port        << "," << std::endl;
    Rcpp::Rcout << "  ip: "               << ip                << "," << std::endl;
    Rcpp::Rcout << "  key: "              << key               << "," << std::endl;
    Rcpp::Rcout << "  shell_port: "       << shell_port        << "," << std::endl;
    Rcpp::Rcout << "  signature_scheme: " << signature_scheme  << "," << std::endl;
    Rcpp::Rcout << "  stdin_port: "       << stdin_port        << "," << std::endl;
    Rcpp::Rcout << "  transport: "        << transport                << std::endl;
    Rcpp::Rcout << "}"                                                << std::endl;
  }
  
  static config read_connection_file(const std::string& connection_file) {
    std::ifstream ifs(connection_file);
    json connection_info = json::parse(ifs);
    return {
      std::to_string(connection_info["control_port"    ].get<int        >()),
      std::to_string(connection_info["hb_port"         ].get<int        >()),
      std::to_string(connection_info["iopub_port"      ].get<int        >()),
                     connection_info["ip"              ].get<std::string>() ,
                     connection_info["key"             ].get<std::string>() ,
      std::to_string(connection_info["shell_port"      ].get<int        >()),
                     connection_info["signature_scheme"].get<std::string>() ,
      std::to_string(connection_info["stdin_port"      ].get<int        >()),
                     connection_info["transport"       ].get<std::string>() ,
    };
  }
};

void read_zmq(const zmq::multipart_t& msg) {
  std::stringstream ss;
  
  std::map<std::string, void(*)()> _map;
  _map["foo"] = [](){};
  for (size_t i = 0; i < msg.size(); i++) {
    const zmq::message_t& part = msg.at(i);
    
    const unsigned char* data = part.data<unsigned char>();
    size_t size = part.size();
    
    // Dump the message as text or binary
    bool isText = true;
    for (size_t j = 0; j < size; j++)
    {
      if (data[j] < 32 || data[j] > 127)
      {
        isText = false;
        break;
      }
    }
    ss << "\n[" << std::dec << std::setw(3) << std::setfill('0') << size << "] ";
    if (size >= 1000)
    {
      ss << "... (to big to print)";
      continue;
    }
    for (size_t j = 0; j < size; j++)
    {
      if (isText)
        ss << static_cast<char>(data[j]);
      else
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(data[j]);
    }
    Rcpp::Rcout << " part " << i << ": " << ss.str() << std::endl;
    ss.str(std::string());
  }
}

#endif // ifndef juniper_juniper_juniper_H
