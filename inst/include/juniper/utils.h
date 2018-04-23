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
#ifndef juniper_juniper_utils_H
#define juniper_juniper_utils_H
#include <string>
#include <xeus/nl_json.hpp>
#include <zmq.hpp>
#include <Rcpp.h>

using nlohmann::json;


template<int SXP, class CTYPE>
static json from_sexp(SEXP s) {
  Rcpp::Vector<SXP> rvec = Rcpp::as<Rcpp::Vector<SXP>>(s);

  if( rvec.size()==1 ) {
    if( SXP==LGLSXP ) return (bool)rvec[0];
    return rvec[0];
  }

  std::vector<CTYPE> cvec;
  for(int i=0; i<rvec.size(); ++i)
    cvec.push_back(rvec[i]);
  return cvec;
}

// recursive parse a List into a json
static json from_list_r(Rcpp::List lst) {
  if( lst.size()==0 ) return json::object();
  bool nonames = TYPEOF(lst.attr("names")) == NILSXP;
  std::vector<std::string> names;
  if( !nonames ) {
    std::vector<std::string> names_attr = lst.names();
    names = names_attr; // copies
  }
  json j;
  int i=0;
  for( Rcpp::List::iterator it = lst.begin(); it!=lst.end(); ++it ) {
    switch( TYPEOF(*it) ) {
    case NILSXP:  { if( nonames ) j.push_back(nullptr); else j[names.at(i++)] = nullptr; break; }
    case INTSXP:  {
      json tmp = from_sexp<INTSXP, int>(*it);
      if( nonames ) j.push_back(tmp);
      else          j[names.at(i++)] = tmp;
      break;
    }
    case REALSXP: {
      json tmp = from_sexp<REALSXP, double>(*it);
      if( nonames ) j.push_back(tmp);
      else          j[names.at(i++)] = tmp;
      break;
    }
    case LGLSXP: {
      json tmp = from_sexp<LGLSXP, bool>(*it);
      if( nonames ) j.push_back(tmp);
      else          j[names.at(i++)] = tmp;
      break;
    }
    case VECSXP: {
      json tmp = from_list_r(*it);
      if( nonames ) j.push_back(tmp);
      else          j[names.at(i++)] = tmp;
      break;
    }
    case STRSXP: {
      Rcpp::StringVector tmp = Rcpp::as<Rcpp::StringVector>(*it);
      if( tmp.size()==1 ) {
        if( nonames ) j.push_back(Rcpp::as<std::string>(*it));
        else          j[names.at(i++)] = Rcpp::as<std::string>(*it);
      } else {
        std::vector<std::string> chars;
        std::string skip = "__juniper_vec_ignore_hack__";
        for( Rcpp::StringVector::iterator ii=tmp.begin(); ii!=tmp.end(); ++ii ) {
          if( !skip.compare(*ii) ) continue;
          chars.emplace_back(*ii);
        }
        if( nonames ) j.push_back(chars);
        else          j[names.at(i++)] = chars;
      }
      break;
    }
    default:
      std::stringstream s;
      s << "incompatible SEXP encountered: " << TYPEOF(*it);
      Rcpp::stop(s.str());
    }
  }
  return j;
}

static SEXP from_json_r(json j);

template<int SXP, class CTYPE>
static SEXP as_sexp(const json& j, bool is_val) {
  if( is_val ) {
    Rcpp::Vector<SXP> res(1);
    res[0] = j.get<CTYPE>();
    return Rcpp::wrap(res);
  }
  Rcpp::Vector<SXP> res(j.size());
  for( size_t i=0; i<j.size(); ++i ) {
    res[i] = j[i].get<CTYPE>();
  }
  return Rcpp::wrap(res);
}


// recursive parse json into list
static SEXP j_to_sexp(const json& j, bool is_val=false) {
  if( j.is_null() || (!is_val && j.size()==0) ) return R_NilValue;
  json::value_t type = is_val?j.type():j[0].type();
  switch( type ) {
  case json::value_t::null:            return R_NilValue;
  case json::value_t::boolean:         return as_sexp<LGLSXP,bool>(j, is_val);
  case json::value_t::string:          return as_sexp<STRSXP,std::string>(j, is_val);
  case json::value_t::number_unsigned: /*fall through*/
  case json::value_t::number_integer:  return as_sexp<INTSXP, int>(j, is_val);
  case json::value_t::number_float:    return as_sexp<REALSXP, double>(j, is_val);
  case json::value_t::object:          return from_json_r(j);
  default: 
    std::stringstream s;
    s << "don't know what to do with type: " << (int)type;
    Rcpp::stop(s.str());
  }
}


static SEXP from_json_r(json j) {
  if( j.is_object() ) {
    Rcpp::List res(j.size());
    Rcpp::StringVector names(j.size());
    int i=0;
    for(json::iterator it = j.begin(); it!=j.end(); ++it) {
      names(i) = it.key();
      res[i++] = from_json_r(it.value());
    }
    res.attr("names") = names;
    return Rcpp::wrap(res);
  }
  return j_to_sexp(j, !j.is_array());
}

static std::string msg_t_to_string(const zmq::message_t& msg) {
  std::stringstream ss;
  const char* chars = msg.data<char>();
  size_t sz = msg.size();
  for(size_t i=0; i<sz; ++i)
    ss << chars[i];
  return ss.str();
}

static int read_port(zmq::socket_t* sock) {
  char endpoint[32];
  size_t sz = sizeof(endpoint);
  sock->getsockopt(ZMQ_LAST_ENDPOINT, &endpoint, &sz);
  std::string ep(endpoint);
  std::string port(ep.substr(ep.find(":", ep.find(":")+1)+1));
  return stoi(port);
}

static std::string read_str(const zmq::message_t& msg) {
  std::stringstream ss;
  const char* buf = msg.data<const char>();
  size_t buflen = msg.size();
  for(size_t i=0; i<buflen; ++i)
    ss << static_cast<char>(buf[i]);
  return ss.str();
}
#endif // #ifndef juniper_juniper_utils_H
