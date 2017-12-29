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
#ifndef juniper_juniper_conf_H
#define juniper_juniper_conf_H

#include <fstream>
#include <string>
#include <xeus/nl_json.hpp>
#include <zmq.hpp>
// ERROR defined in zmq.hpp, but we want
// the one from Rcpp
#ifdef ERROR 
#undef ERROR
#endif
#include <Rcpp.h>
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

#endif // ifndef juniper_juniper_conf_H
