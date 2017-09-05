#include <string>
#include <Rcpp.h>
#include <json.hpp>

using nlohmann::json;

// recursive parse a List into a json
// logical and string vectors need special handling so it
// defeats the purpose of templating the unpack from List->json
static json from_list_r(Rcpp::List lst) {
  std::vector<std::string> names = lst.names();
  json j;
  int i=0;
  for( Rcpp::List::iterator it = lst.begin(); it!=lst.end(); ++it ) {
    switch( TYPEOF(*it) ) {
    case VECSXP: {
      j[names.at(i++)] = from_list_r(*it);
      break;
    }
    case NILSXP: {
      j[names.at(i++)] = nullptr;
      break;
    }
    case INTSXP: {
      Rcpp::IntegerVector tmp = Rcpp::as<Rcpp::IntegerVector>(*it);
      if( tmp.size()==1 ) {
        j[names.at(i++)] = tmp[0];
      } else {
        std::vector<int> ints;
        for( Rcpp::IntegerVector::iterator ii=tmp.begin(); ii!=tmp.end(); ++ii )
          ints.emplace_back(*ii);
        j[names.at(i++)] = ints;
      }
      break;
    }
    case REALSXP: {
      Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(*it);
      if( tmp.size()==1 ) {
        j[names.at(i++)] = tmp[0];
      } else {
        std::vector<double> dbls;
        for( Rcpp::NumericVector::iterator ii=tmp.begin(); ii!=tmp.end(); ++ii )
          dbls.emplace_back(*ii);
        j[names.at(i++)] = dbls;
      }
      break;
    }
    case LGLSXP: {
      Rcpp::LogicalVector tmp = Rcpp::as<Rcpp::LogicalVector>(*it);
      if( tmp.size()==1 ) {
        j[names.at(i++)] = (bool)tmp[0];
      } else {
        std::vector<bool> bools;
        for( Rcpp::LogicalVector::iterator ii=tmp.begin(); ii!=tmp.end(); ++ii )
          bools.push_back(*ii);
        j[names.at(i++)] = bools;
      }
      break;
    }
    case STRSXP: {
      Rcpp::StringVector tmp = Rcpp::as<Rcpp::StringVector>(*it);
      if( tmp.size()==1 ) {
        j[names.at(i++)] = Rcpp::as<std::string>(*it);
      } else {
        std::vector<std::string> chars;
        for( Rcpp::StringVector::iterator ii=tmp.begin(); ii!=tmp.end(); ++ii )
          chars.emplace_back(*ii);
        j[names.at(i++)] = chars;
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

static SEXP j_to_sexp(const json& j, bool is_val=false) {
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
