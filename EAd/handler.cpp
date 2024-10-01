/*
 * handler.cpp
 *
 * TODO: investigate switching to extract_string & RapidJSON for performance
 */

#include "gzip.hpp"
#include "portability.hpp"
#include "handler.hpp"
#include "apiver.hpp"
#include "exportCalls.hpp"
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <ctime>
#include <memory>
#include <mutex>
#include <sstream>
#include <set>

//#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/copy.hpp>
//#include <boost/iostreams/filter/gzip.hpp>
//#include <boost/iostreams/filter/bzip2.hpp>
#ifdef USE_SSL
# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>
#endif

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

const std::regex handler::re_api("^(/v([0-9]+))?(/.*)$");

// Use a global mutex to lock all libEA API calls since it's doubtful the back end
// is thread-safe!
//std::mutex global_api_lock;

//
// Use a global mutex to lock out the compound sampletimestep call because
// it consists of 3 separate API calls that should be considered atomic.
//
//std::mutex global_sampletimestep_lock;

//handler::handler()
//{
//    //ctor
//}

#ifdef USE_SSL
handler::handler(const utility::string_t& url, const web::http::experimental::listener::http_listener_config& server_config, IExportOmni *tool) : m_listener(url,server_config), p_Port(tool), domain(tool->SayInfoFromDomain()), seq(0), alertseq(0)
#else
handler::handler(const utility::string_t& url, IExportOmni *tool) : m_listener(url), p_Port(tool), domain(tool->SayInfoFromDomain()), seq(0), alertseq(0)
#endif
{
   m_listener.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
   m_listener.support(methods::PUT, std::bind(&handler::handle_put, this, std::placeholders::_1));
   m_listener.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
   m_listener.support(methods::DEL, std::bind(&handler::handle_delete, this, std::placeholders::_1));
   m_listener.support(methods::OPTIONS, std::bind(&handler::handle_options, this, std::placeholders::_1));  // for CORS preflight requests
}

handler::~handler()
{
   //dtor
   // Should probably clean up contents here, but current in current use cases, only one object will
   // ever be created and it will only be destroyed on exit, so doing nothing explicit is acceptable.
}

void handler::handle_error(pplx::task<void>& t)
{
   try { t.get(); }
   catch(...) { /* Ignore the error, Log it if a logger is available */ }
}

//
// Private static function to pull the json body content out of an http_request
//
void handler::extract_json(const http_request& msg, json::value& jv) {
   msg.extract_json()
   .then([&jv](pplx::task<json::value> task) {
      try {
         jv = task.get();
      } catch (http_exception const & e) {
         // It's not an error here to have no json content
      }
      catch(json::json_exception & e) {
         //message.reply(status_codes::BadRequest);
         // Unfortunately we can only ignore json errors the way this is
         // structured for now
      }
   })
   .wait();
}

//
// Private static function to return a string value from a query string key
//
bool handler::get_querystring(const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, utility::string_t& value) {
   auto i = qsmap.find(key);
   if (i != qsmap.end()) {
      value = i->second;
      return true;
   } else {
      return false;
   }
}

//
// Private static function to return a json::value from a top-level key in a json::value
// Returns true if key was found, false if not.
//
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& value) {
   if (!jv.is_null() && jv.has_field(key)) {
      value = jv.at(key);
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      value = json::value::string(U(t));
      return true;
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, int& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = value.as_integer(); }
      catch (...) {
         ss << "Parameter " << key << " expects an int type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = std::stoi(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects an int type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type integer";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, uint64_t& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = (uint64_t) value.as_number().to_uint64(); }
      catch (...) {
         ss << "Key " << key << " expects a uint64_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = (uint64_t) std::stoull(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects a uint64_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type uint64_t";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, unsigned int& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = (unsigned int) value.as_number().to_uint32(); }
      catch (...) {
         ss << "Key " << key << " expects an unsigned int type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = (unsigned int) std::stoul(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects an unsigned int type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type unsigned int";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, time_t& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = (time_t) value.as_number().to_uint32(); }
      catch (...) {
         ss << "Key " << key << " expects a time_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = std::stoul(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects a time_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type time_t";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
#ifdef __arm64__
// Macs with Apple silicon (M1/M2 CPUs) need this extra type
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, size_t& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = (size_t) value.as_number().to_uint64(); }
      catch (...) {
         ss << "Key " << key << " expects a size_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = std::stoul(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects a size_t type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type size_t";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
#endif
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, utility::string_t& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = value.as_string(); }
      catch (...) {
         ss << "Key " << key << " expects a string type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (handler::get_querystring(qsmap, key, raw)) {
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type string";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, char& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = value.as_string()[0]; }
      catch (...) {
         ss << "Key " << key << " expects a char (string) type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try { raw = t[0]; }
      catch (...) {
         ss << "Querystring parameter " << key << " expects a char (string) type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type char (string)";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}
bool handler::get_json_value(const bool optional, const json::value& jv, const std::map<utility::string_t,utility::string_t>& qsmap, const utility::string_t& key, json::value& reply, double& raw) {
   stringstream ss;
   if (!jv.is_null() && jv.has_field(key)) {
      auto value = json::value(jv.at(key));
      try { raw = value.as_double(); }
      catch (...) {
         ss << "Key " << key << " expects a double type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   utility::string_t t;
   if (handler::get_querystring(qsmap, key, t)) {
      try {
         raw = std::stod(t);
      } catch (...) {
         ss << "Querystring parameter " << key << " expects a double type";
         reply[U("error")] = json::value::string(U(ss.str()));
         return false;
      }
      return true;
   }
   if (!optional) {
      ss << "Expected parameter " << key << " of type double";
      reply[U("error")] = json::value::string(U(ss.str()));
   }
   return false;
}

//
// Define some macros to streamline error handling below
//
#define TRYAPI0(CODE) \
try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
   stringstream ss; \
   ss << "API call failed"; \
   reply[U("error")] = json::value(U(ss.str())); \
   ucout << funcname << ": " << ss.str() << endl; \
   retval = status_codes::BadRequest; \
}
#define TRYAPI1(VAR,CODE) \
   try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
      stringstream ss; \
      ss << "API call failed for " #VAR "=" << VAR; \
      reply[U("error")] = json::value(U(ss.str())); \
      ucout << funcname << ": " << ss.str() << endl; \
      retval = status_codes::BadRequest; \
   }
#define TRYAPI2(VAR1,VAR2,CODE) \
   try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
      stringstream ss; \
      ss << "API call failed for " #VAR1 "=" << VAR1 << " " #VAR2 "=" << VAR2; \
      reply[U("error")] = json::value(U(ss.str())); \
      ucout << funcname << ": " << ss.str() << endl; \
      retval = status_codes::BadRequest; \
   }
#define TRYAPI3(VAR1,VAR2,VAR3,CODE) \
   try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
      stringstream ss; \
      ss << "API call failed for " #VAR1 "=" << VAR1 << " " #VAR2 "=" << VAR2 << " " #VAR3 "=" << VAR3; \
      reply[U("error")] = json::value(U(ss.str())); \
      ucout << funcname << ": " << ss.str() << endl; \
      retval = status_codes::BadRequest; \
   }
#define TRYAPI4(VAR1,VAR2,VAR3,VAR4,CODE) \
   try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
      stringstream ss; \
      ss << "API call failed for " #VAR1 "=" << VAR1 << " " #VAR2 "=" << VAR2 << " " #VAR3 "=" << VAR3 << " " #VAR4 "=" << VAR4; \
      reply[U("error")] = json::value(U(ss.str())); \
      ucout << funcname << ": " << ss.str() << endl; \
      retval = status_codes::BadRequest; \
   }
#define TRYAPI5(VAR1,VAR2,VAR3,VAR4,VAR5,CODE) \
   try { std::lock_guard<std::mutex> glock(instance_api_lock); CODE } catch (...) { \
      stringstream ss; \
      ss << "API call failed for " #VAR1 "=" << VAR1 << " " #VAR2 "=" << VAR2 << " " #VAR3 "=" << VAR3 << " " #VAR4 "=" << VAR4 << " " #VAR5 "=" << VAR5; \
      reply[U("error")] = json::value(U(ss.str())); \
      ucout << funcname << ": " << ss.str() << endl; \
      retval = status_codes::BadRequest; \
   }

//
// Task to wait for timeout or seq change, whichever comes first
// Returns true if there was a change; false if it was a timeout
//
pplx::task<bool> handler::wait_for_event(uint64_t oldseq, unsigned int timeout) {
   std::chrono::seconds duration(timeout);

   return pplx::create_task([=]{
      std::cout << "wait_for_event:create_task waiting for lock" << std::endl;
      std::unique_lock<std::mutex> lk(cvm);
      std::cout << "wait_for_event:create_task got lock; waiting for timeout or seq change" << std::endl;
      return(cv.wait_for(lk, duration, [=]{ return(oldseq != seq); }));
   });
}

//
// Increment the sequence number and alert any waiting threads that it changed
//
void handler::update_seq(void) {
   //std::cout << "update_seq called" << std::endl;
   {
      std::lock_guard<std::mutex> lk(cvm);
      seq++;
   }
   // notify outside the block because we don't need to keep the lock during notify
   //std::cout << "update_seq notify_all" << std::endl;
   cv.notify_all();
}

// return a json object with a number
inline static const json::value json_num(const GuiFpn_t fpn) {
   return(json::value(fpn));
}
inline static const json::value json_num(const size_t n) {
    return(json::value((uint64_t) n));
}
inline static const json::value json_num(const GuiSin_t n) {
    return(json::value(n));
}
inline static const json::value json_num(const GuiUin_t n) {
    return(json::value(n));
}

// return a json object with a boolean
inline static const json::value json_bool(const GuiFpn_t b) {
   return(json::value(b != 0.0));
}
inline static const json::value json_bool(const int b) {
   return(json::value(b != 0));
}
inline static const json::value json_bool(const bool b) {
   return(json::value(b));
}

// return a json object with a numeric version of the object key
inline static const json::value json_key(const NGuiKey & key) {
   return(json::value((uint64_t) key.Peek()));
}

// return a json value object representing an EGuiReply numeric value
//inline static const json::value json_reply(const EGuiReply v) {
//   return json::value((int) v);
//}

// return a json object with a string value
inline static const json::value json_string(const std::string & s) {
   return(json::value::string(U(s)));
}

// Return a json string with a getterReply value
static const json::value json_reply(const EGuiReply r) {
   switch(r) {
      case EGuiReply::OKAY_allDone:                                    return(json_string("OKAY_allDone"));
      case EGuiReply::OKAY_done_caseDestroyedByUser_discardKey:        return(json_string("OKAY_done_caseDestroyedByUser_discardKey"));
      case EGuiReply::OKAY_done_kronoDestroyedByUser_discardKey:       return(json_string("OKAY_done_kronoDestroyedByUser_discardKey"));
      case EGuiReply::FAIL_any_calledFunctionNotYetImplemented:        return(json_string("FAIL_any_calledFunctionNotYetImplemented"));
      case EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled:      return(json_string("FAIL_any_givenKeyNotValidForFunctionCalled"));
      case EGuiReply::FAIL_get_calledDynamicUpdateBeforeFullGuiPack:   return(json_string("FAIL_get_calledDynamicUpdateBeforeFullGuiPack"));
      case EGuiReply::FAIL_get_calledDynamicUpdateUsingKeyToStaticData:return(json_string("FAIL_get_calledDynamicUpdateUsingKeyToStaticData"));
      case EGuiReply::FAIL_get_calledHistogramNotYetRecorded:          return(json_string("FAIL_get_calledHistogramNotYetRecorded"));
      case EGuiReply::FAIL_set_askedForNewKronoBeforeExistingCleared:  return(json_string("FAIL_set_askedForNewKronoBeforeExistingCleared"));
      case EGuiReply::FAIL_set_askedStatisticBeyondDepthLimit:         return(json_string("FAIL_set_askedStatisticBeyondDepthLimit"));
      case EGuiReply::FAIL_set_givenContainerWrongSizeForKeyGiven:     return(json_string("FAIL_set_givenContainerWrongSizeForKeyGiven"));
      case EGuiReply::FAIL_set_givenDataLoggingSizeNotWithinBounds:    return(json_string("FAIL_set_givenDataLoggingSizeNotWithinBounds"));
      case EGuiReply::FAIL_set_givenDialogueAnswerNotAllowed:          return(json_string("FAIL_set_givenDialogueAnswerNotAllowed"));
      case EGuiReply::FAIL_set_givenHistogramModeNotAvailable:         return(json_string("FAIL_set_givenHistogramModeNotAvailable"));
      case EGuiReply::FAIL_set_givenRuleIdNotInRuleKit:                return(json_string("FAIL_set_givenRuleIdNotInRuleKit"));
      case EGuiReply::FAIL_set_givenTimestampSameAsPrevious:           return(json_string("FAIL_set_givenTimestampSameAsPrevious"));
      case EGuiReply::FAIL_set_givenValueOutOfRangeAllowed:            return(json_string("FAIL_set_givenValueOutOfRangeAllowed"));
      case EGuiReply::FAIL_set_typeGivenNotTypeTaken:                  return(json_string("FAIL_set_typeGivenNotTypeTaken"));
      case EGuiReply::WARN_nullReply_FixApi:                           return(json_string("WARN_nullReply_FixApi"));
      case EGuiReply::WARN_ranSeqToExitWithObjectsYetToCycle_fixApi:   return(json_string("WARN_ranSeqToExitWithObjectsYetToCycle_fixApi"));
      default:
         stringstream ss;
         ss << "WARN unexpected EGuiReply (" << (int) r << ") - notify developer!";
         return(json_string(ss.str()));
   }
   //NOTREACHED
}

// static helper functions for building JSON arrays of specific types
inline static const json::value json_array(std::queue<std::string> arr) {
   auto a = json::value::array();
   int i = 0;
   while (! arr.empty()) {
      a[i++] = json::value::string(U(arr.front()));
      arr.pop();
   }
   return(a);
}
inline static const json::value json_array(const std::vector<std::string> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json::value(element);
   }
   return(a);
}
inline static const json::value json_array(const std::vector<NGuiKey> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json_key(element);
   }
   return(a);
}
inline static const json::value json_array(const std::vector<time_t> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json::value((uint64_t) element);
   }
   return(a);
}
inline static const json::value json_array(const std::vector<GuiFpn_t> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json::value(element);
   }
   return(a);
}
inline static const json::value json_array(const std::vector<GuiSin_t> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json::value(element);
   }
   return(a);
}
inline static const json::value json_array(const std::vector<GuiUin_t> & arr) {
   auto a = json::value::array();
   int i = 0;
   for (auto const &element : arr) {
      a[i++] = json::value(element);
   }
   return(a);
}

// return a json object with a string value
// these mappings came out of the EAwin32Client code
inline static const json::value json_state(EGuiState c) {
   switch(c) {
      case EGuiState::Undefined:
         return json_string("U");
      case EGuiState::Analog_invalid:
         return json_string("Analog_invalid");
      case EGuiState::Analog_unavailable:
         return json_string("Analog_unavailable");
      case EGuiState::Analog_valid:
         return json_string("Analog_valid");
      case EGuiState::Fact_false:
         return json_string("Fact_false");
      case EGuiState::Fact_invalid:
         return json_string("Fact_invalid");
      case EGuiState::Fact_unavailable:
         return json_string("Fact_unavailable");
      case EGuiState::Fact_true:
         return json_string("Fact_true");
      case EGuiState::Feature_false:
         return json_string("F");
      case EGuiState::Feature_invalid:
         return json_string("I");
      case EGuiState::Feature_neutral:
         return json_string("N");
      case EGuiState::Feature_true:
         return json_string("T"); break;
      case EGuiState::Rule_autoMode_fail:
         return json_string("Rule_autoMode_fail"); break;
      case EGuiState::Rule_autoMode_pass:
         return json_string("Rule_autoMode_pass"); break;
      case EGuiState::Rule_autoMode_skip:
         return json_string("Rule_autoMode_skip"); break;
      case EGuiState::Rule_caseMode_fail:
         return json_string("Rule_caseMode_fail"); break;
      case EGuiState::Rule_caseMode_pass:
         return json_string("Rule_caseMode_pass"); break;
      case EGuiState::Rule_caseMode_skip:
         return json_string("Rule_caseMode_skip"); break;
      case EGuiState::Rule_idleMode_fail:
         return json_string("Rule_idleMode_fail"); break;
      case EGuiState::Rule_idleMode_pass:
         return json_string("Rule_idleMode_pass"); break;
      case EGuiState::Rule_idleMode_skip:
         return json_string("Rule_idleMode_skip"); break;
      case EGuiState::Rule_invalid:
         return json_string("Rule_invalid"); break;
      case EGuiState::Rule_unavailable:
         return json_string("Rule_unavailable");
   }
   return(json_string("X"));
}

// return a json object with a string value
// these mappings came out of the EAwin32Client code
inline static const json::value json_objtype(const EGuiType & t) {
   std::string s = "UNKNOWN";
   switch(t) {
      case EGuiType::Undefined:  // Typically only to fill field(s) in an empty view/control struct
         s = "Undefined";                                break;
      case EGuiType::Case:
         s = "Case";                                     break;
      case EGuiType::Display_ruleKit:
         s = "Display_ruleKit";                          break;
      case EGuiType::Domain:
         s = "Domain";                                   break;
      case EGuiType::Feature_analog:
         s = "Feature_analog";                           break;
      case EGuiType::Feature_fact:
         s = "Feature_fact";                             break;
      case EGuiType::Histogram_analog:
         s = "Histogram_analog";                         break;
      case EGuiType::Histogram_fact:
         s = "Histogram_fact";                           break;
      case EGuiType::Histogram_rule:
         s = "Histogram_rule";                           break;
      case EGuiType::Histogram_ruleKit:
         s = "Histogram_ruleKit";                        break;
      case EGuiType::HistogramBars_analogBins:
         s = "HistogramBars_analogBins";                 break;
      case EGuiType::HistogramBars_analogStates:
         s = "HistogramBars_analogStates";               break;
      case EGuiType::HistogramBars_factStates:
         s = "HistogramBars_factStates";                 break;
      case EGuiType::HistogramBars_ruleStates:
         s = "HistogramBars_ruleStates";                 break;
      case EGuiType::HistogramBars_rulesUai:
         s = "HistogramBars_rulesUai";                   break;
      case EGuiType::Knob_takesGuiFpnAsBoolean:
         s = "Knob_takesGuiFpnAsBoolean";                break;
      case EGuiType::Knob_takesGuiFpnAsInteger:
         s = "Knob_takesGuiFpnAsInteger";                break;
      case EGuiType::Knob_takesGuiFpnAsFloat:
         s = "Knob_takesGuiFpnAsFloat";                  break;
      case EGuiType::Knob_takesGuiUin:
         s = "Knob_takesGuiUin";                         break;
      case EGuiType::Knob_selectsGuiFpnFromList:
         s = "Knob_selectsGuiFpnFromList";               break;
      case EGuiType::Krono_realtime:
         s = "Krono_realtime";                           break;
      case EGuiType::Krono_snapshot:
         s = "Krono_snapshot";                           break;
      case EGuiType::Pane_analog:
         s = "Pane_analog";                              break;
      case EGuiType::Pane_fact:
         s = "Pane_fact";                                break;
      case EGuiType::Pane_rule:
         s = "Pane_rule";                                break;
      case EGuiType::Subject:
         s = "Subject";                                  break;
      case EGuiType::Trace_analog:
         s = "Trace_analog";                             break;
      case EGuiType::Trace_fact:
         s = "Trace_fact";                               break;
      case EGuiType::Trace_rule:
         s = "Trace_rule";                               break;
   }
   return(json_string(s));
}

// return a json object with a string value
// these mappings came out of the EAwin32Client code
inline static const json::value json_pointname(const EPointName & t) {
   std::string s = "UNKNOWN";
   switch(t) {
      case EPointName::Undefined:                        s = "Undefined";                          break;
      case EPointName::Binary_systemOccupied:            s = "Binary_systemOccupied";              break;
      case EPointName::Binary_zoneOccupied:              s = "Binary_zoneOccupied";                break;
      case EPointName::Command_damper_mixingBox:         s = "Command_damper_mixingBox";           break;
      case EPointName::Command_damper_outsideAir:        s = "Command_damper_outsideAir";          break;
      case EPointName::Command_damper_vav:               s = "Command_damper_vav";                 break;
      case EPointName::Command_fanSpeed:                 s = "Command_fanSpeed";                   break;
      case EPointName::Command_valve_chw:                s = "Command_valve_chw";                  break;
      case EPointName::Command_valve_hw:                 s = "Command_valve_hw";                   break;
      case EPointName::FlowRateVolume_air_ahu:           s = "FlowRateVolume_air_ahu";             break;
      case EPointName::FlowRateVolume_air_ahu_setpt:     s = "FlowRateVolume_air_ahu_setpt";       break;
      case EPointName::FlowRateVolume_air_vav:           s = "FlowRateVolume_air_vav";             break;
      case EPointName::FlowRateVolume_air_vav_setpt:     s = "FlowRateVolume_air_vav_setpt";       break;
      case EPointName::Position_damper_mixingBox:        s = "Position_damper_mixingBox";          break;
      case EPointName::Position_damper_outsideAir:       s = "Position_damper_outsideAir";         break;
      case EPointName::Position_damper_vav:              s = "Position_damper_vav";                break;
      case EPointName::Position_valve_chw:               s = "Position_valve_chw";                 break;
      case EPointName::Position_valve_hw:                s = "Position_valve_hw";                  break;
      case EPointName::Pressure_static_air_inlet:        s = "Pressure_static_air_inlet";          break;
      case EPointName::Pressure_static_air_supply:       s = "Pressure_static_air_supply";         break;
      case EPointName::Pressure_static_air_supply_setpt: s = "Pressure_static_air_supply_setpt";   break;
      case EPointName::Temperature_air_discharge:        s = "Temperature_air_discharge";          break;
      case EPointName::Temperature_air_inlet:            s = "Temperature_air_inlet";              break;
      case EPointName::Temperature_air_mixed:            s = "Temperature_air_mixed";              break;
      case EPointName::Temperature_air_outside:          s = "Temperature_air_outside";            break;
      case EPointName::Temperature_air_return:           s = "Temperature_air_return";             break;
      case EPointName::Temperature_air_supply:           s = "Temperature_air_supply";             break;
      case EPointName::Temperature_air_supply_setpt:     s = "Temperature_air_supply_setpt";       break;
      case EPointName::Temperature_air_zone:             s = "Temperature_air_zone";               break;
      case EPointName::Temperature_air_zone_setpt_clg:   s = "Temperature_air_zone_setpt_clg";     break;
      case EPointName::Temperature_air_zone_setpt_htg:   s = "Temperature_air_zone_setpt_htg";     break;

   }
   return(json_string(s));
}


// fill in a json object reference with subject data
const json::value handler::json_subject(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      obj[U("idtext")] = json_string(p_Port->SayTextIdentifyingSubject(key));
      GuiPackSubjectBasic_t s = p_Port->SayInfoFromSubject(key);
      obj[U("reply")] = json_reply(s.getterReply);
      obj[U("domain")] = json_key(s.hostDomainKey);
      obj[U("label")] = json_string(s.infoText_byCR[0]);
      obj[U("info")] = json::value::array();
      int i = 0;
      for (auto const & t : s.infoText_byCR) {
         obj[U("info")][i++] = json_string(t);
      }
      obj[U("name")] = json_string(s.ownNameText);
      obj[U("featurekeys")] = json::value::array();
      obj[U("knobkeys")] = json::value::array();
      obj[U("rulekitkeys")] = json::value::array();
      obj[U("casekeys")] = json::value::array();
      obj[U("points")] = json::value::array();
      if (recurse) {
         obj[U("features")] = json::value::array();
         obj[U("knobs")] = json::value::array();
         obj[U("rulekits")] = json::value::array();
         obj[U("cases")] = json::value::array();
      }
      i = 0;
      for (auto const& fkey : s.featureKeys) {
         if (recurse)
            obj[U("features")][i] = json_feature(fkey);
         obj[U("featurekeys")][i++] = json_key(fkey);
      }
      i = 0;
      for (auto const& kkey : s.paramKnobKeys) {
         if (recurse)
            obj[U("knobs")][i] = json_knob(kkey);
         obj[U("knobkeys")][i++] = json_key(kkey);
      }
      i = 0;
      for (auto const& kkey : s.ruleKitKeys) {
         if (recurse)
            obj[U("rulekits")][i] = json_rulekit(kkey);
         obj[U("rulekitkeys")][i++] = json_key(kkey);
      }
      i = 0;
      auto cases = p_Port->SayCurrentCasesFromSubject(key);
      for (auto const& ckey : cases.currentCaseKeys) {
         if (recurse)
            obj[U("cases")][i] = json_case(ckey);
         obj[U("casekeys")][i++] = json_key(ckey);
      }
      i = 0;
      auto points = p_Port->SayInputPointNameOrderExpectedBySubject(key);
      for (auto const& p : points) {
         obj[U("points")][i++] = json_pointname(p);
      }
   } catch(...) {
      obj[U("error")] = json_string("Error fetching subject static info");
   }
   return(obj);
}

// fill in a json object reference with case data
const json::value handler::json_case(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      GuiPackCaseFull_t c = p_Port->SayFullInfoFromCase(key);
      obj[U("reply")] = json_reply(c.getterReply);
      if (c.getterReply == EGuiReply::OKAY_allDone) {
         obj[U("label")] = json_string(c.caseName);
         if (c.snapshotKronoKey.Peek() > 0) {
             obj[U("krono")] = json_krono(c.snapshotKronoKey);
         }
         obj[U("report")] = json_array(c.reportText_byCR);
         obj[U("prompt")] = json_array(c.promptText_byCR);
         obj[U("options")] = json_array(c.optionText_each);
      } else {
         obj[U("error")] = json_reply(c.getterReply);
      }
   } catch (...) {
      obj[U("error")] = json_string("Error fetching case info");
   }
   return(obj);
}
// fill in a json object reference with feature data
const json::value handler::json_feature(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      GuiPackFeatureFull_t f = p_Port->SayFullInfoFromFeature(key);
      obj[U("type")] = json_objtype(f.ownType);
      obj[U("uai")] = json::value(f.featureUai);
      obj[U("label")] = json_string(f.labelText);
      obj[U("units")] = json_string(f.unitsText);
      obj[U("message")] = json_string(f.messageText);
      obj[U("state")] = json_state(f.messageState);
      obj[U("knobs")] = json::value::array();
      obj[U("histogram")] = json_histogram(f.sourceHistogramKey);
      auto i = 0;
      for (auto const& kkey : f.ownKnobKeys) {
         obj[U("knobs")][i++] = json_knob(kkey);
      }
   } catch (...) {
      obj[U("error")] = json_string("Error fetching feature info");
   }
   return(obj);
}

// fill in a json object with histogram data
const json::value handler::json_histogram(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      obj[U("idtext")] = json_string(p_Port->SayTextIdentifyingHistogram(key));
      GuiPackHistogram_t h = p_Port->SayInfoFromHistogram(key);
      obj[U("type")] = json_objtype(h.ownType);
      obj[U("reply")] = json_reply(h.getterReply);
      obj[U("bartype")] = json_objtype(h.barTypeDisplayed);
      obj[U("barcount")] = json_num(h.numBarsDisplayed);
      obj[U("mode")] = json_num(h.modeNow_index);
      obj[U("span")] = json_num(h.spanNow_index);
      obj[U("caption")] = json::value::array();
      auto i = 0;
      for (auto const & c : h.captionText_byCR) {
         obj[U("caption")][i++] = json_string(c);
      }
      obj[U("bar_heights")] = json::value::array();
      i = 0;
      for (auto const & bh : h.barHeights_leftToRight) {
         obj[U("bar_heights")][i++] = json_num(bh);
      }
      obj[U("knobs")] = json::value::array();
      i = 0;
      for (auto const & k : h.knobKeys) {
         obj[U("knobs")][i++] = json_knob(k);
      }
      obj[U("bar_labels")] = json::value::array();
      if (h.barTypeDisplayed != EGuiType::HistogramBars_analogBins) {
         i = 0;
         for (auto const & l : h.barLabelsAsText_leftToRight_emptyIfBarsAnalog) {
            obj[U("bar_labels")][i++] = json_string(l);
         }
      }
      obj[U("mode_options")] = json::value::array();
      i = 0;
      for (auto const & o : h.modeOptionsText_each) {
         obj[U("mode_options")][i++] = json_string(o);
      }
      obj[U("span_options")] = json::value::array();
      i = 0;
      for (auto const & o : h.spanOptionsText_each) {
         obj[U("span_options")][i++] = json_string(o);
      }
      if (h.barTypeDisplayed == EGuiType::HistogramBars_analogBins &&
         !isnan(h.leftEndBarNumericLabel_nanIfBarsNotAnalog) &&
         !isnan(h.eachBarNumericLabelIncr_nanIfBarsNotAnalog)) {
            obj[U("left_bar_value")] = json_num(h.leftEndBarNumericLabel_nanIfBarsNotAnalog);
            obj[U("each_bar_incr")] = json_num(h.eachBarNumericLabelIncr_nanIfBarsNotAnalog);
            obj[U("right_bar_value")] = json_num(h.leftEndBarNumericLabel_nanIfBarsNotAnalog + (h.numBarsDisplayed * h.eachBarNumericLabelIncr_nanIfBarsNotAnalog));
         } else {
            obj[U("left_bar_value")] = json_string("NaN");
            obj[U("each_bar_incr")] = json_string("NaN");
            obj[U("right_bar_value")] = json_string("NaN");
         }
      } catch (...) {
         obj[U("error")] = json_string("Error fetching histogram info");
      }
      return(obj);
   }

// fill in a json object reference with knob data
const json::value handler::json_knob(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      obj[U("idtext")] = json_string(p_Port->SayTextIdentifyingKnob(key));
      GuiPackKnob_t knob = p_Port->GetInfoFromKnob(key);
      obj[U("reply")] = json_reply(knob.getterReply);
      obj[U("type")] = json_objtype(knob.ownType);
      obj[U("label")] = json_string(knob.labelText);
      obj[U("units")] = json_string(knob.unitsText);
      obj[U("options")] = json_array(knob.definedSelection_emptyIfNA);
      switch (knob.ownType) {
         case EGuiType::Knob_takesGuiFpnAsInteger:
         case EGuiType::Knob_takesGuiFpnAsFloat:
         case EGuiType::Knob_takesGuiUin:
         case EGuiType::Knob_selectsGuiFpnFromList:
            if (! knob.rangeMinMax_emptyIfBool.empty()) {
               obj[U("range_min")] = json_num(knob.rangeMinMax_emptyIfBool[0]);
               obj[U("range_max")] = json_num(knob.rangeMinMax_emptyIfBool[1]);
            }
            obj[U("value")] = json_num(knob.valueNow_numerIfBool);
            break;
         case EGuiType::Knob_takesGuiFpnAsBoolean:
            obj[U("value")] = json_bool(knob.valueNow_numerIfBool);
            break;
         default:
            // Default section needed to prevent compiler warnings
            break;
      }
   } catch (...) {
      obj[U("error")] = json_string("Error fetching knob info");
   }
   return(obj);
}

const json::value handler::json_rulekit(const NGuiKey &key, bool recurse) {
   json::value obj;
   int count;

   obj[U("key")] = json_key(key);
   try {
      obj[U("idtext")] = json_string(p_Port->SayTextIdentifyingRuleKit(key));
      GuiPackRuleKitFull_t rule = p_Port->SayFullInfoFromRuleKit(key);
      obj[U("reply")] = json_reply(rule.getterReply);
      obj[U("histogram")] = json_histogram(rule.ruleKitHistogramKey);
      obj[U("krono")] = json_krono(rule.realtimeKronoKey_zeroIfNone);
      obj[U("caption")] = json_string(rule.captionText);
      obj[U("knobs")] = json::value::array();
      count = 0;
      for (auto & kkey : rule.ruleKitKnobKeys) {
         obj[U("knobs")][count++] = json_knob(kkey);
      }
      obj[U("rulelabels")] = json_array(rule.ruleLabels_topToBottom);
      obj[U("ruletexts_if")] = json_array(rule.ruleTexts_if_topToBottom);
      obj[U("ruletexts_then")] = json_array(rule.ruleTexts_then_topToBottom);
      obj[U("rulestates")] = json::value::array();
      count = 0;
      for (auto const &s : rule.ruleStates_topToBottom) {
         obj[U("rulestates")][count++] = json_state(s);
      }
      obj[U("ruleknobkeys")] = json_array(rule.ruleKnobKeys_topToBottom);
      if (recurse) {
         obj[U("ruleknobs")] = json::value::array();
         count = 0;
         for (auto & kkey : rule.ruleKnobKeys_topToBottom) {
            obj[U("ruleknobs")][count++] = json_knob(kkey);
         }
      }
      obj[U("rulehistogramkeys")] = json_array(rule.ruleHistogramKeys_topToBottom);
      if (recurse) {
         obj[U("rulehistograms")] = json::value::array();
         count = 0;
         for (auto const &hkey : rule.ruleHistogramKeys_topToBottom) {
            obj[U("rulehistograms")][count++] = json_histogram(hkey);
         }
      }
   } catch (...) {
      obj[U("error")] = json_string("Error fetching rulekit info");
   }
   return(obj);
}

const json::value handler::json_traceinkrono(const NGuiKey & key, const NGuiKey & krono, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      GuiPackTraceFull_t trace = p_Port->SayFullInfoFromTraceInKrono(key, krono);  // SWB TODO: what if not in krono?
      obj[U("reply")] = json_reply(trace.getterReply);
      obj[U("type")] = json_objtype(trace.ownType);
      obj[U("tag")] = json_string(trace.tag);
      obj[U("values")] = json_array(trace.numbers_olderToNewer);
      obj[U("states")] = json::value::array();
      int i = 0;
      for (auto const & s : trace.states_olderToNewer) {
          obj[U("states")][i++] = json_state(s);
      }
      obj[U("histogram")] = json_histogram(trace.sourceHistogramKey);
      obj[U("knobs")] = json::value::array();
      i = 0;
      for (auto & knob : trace.knobKeys) {
         obj[U("knobs")][i++] = json_knob(knob);
      }
   } catch(...) {
      obj[U("error")] = json_string("Error fetching trace info");
   }
   return(obj);
}

const json::value handler::json_paneinkrono(const NGuiKey & key, const NGuiKey & krono, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      GuiPackPane_t pane = p_Port->SayInfoFromPane(key);
      obj[U("reply")] = json_reply(pane.getterReply);
      obj[U("type")] = json_objtype(pane.ownType);
      //obj[U("ylabel")] = json_string(pane.yAxisLabel);  // SWB: NO LABEL???
      obj[U("yunits")] = json_string(pane.yAxisUnitsText);
      obj[U("ymax")] = isnan(pane.yAxisMax) ? json_string("NaN") : json::value(pane.yAxisMax);
      obj[U("ymin")] = isnan(pane.yAxisMin) ? json_string("NaN") : json::value(pane.yAxisMin);
      obj[U("traces")] = json::value::array();
      int count = 0;
      for (auto & t : pane.traceKeys) {
         obj[U("traces")][count++] = json_traceinkrono(t, krono);
      }
   } catch(...) {
      obj[U("error")] = json_string("Error fetching pane info");
   }
   return(obj);
}

const json::value handler::json_krono(const NGuiKey & key, bool recurse) {
   json::value obj;

   obj[U("key")] = json_key(key);
   try {
      GuiPackKronoFull_t krono = p_Port->SayFullInfoFromKrono(key);
      obj[U("reply")] = json_reply(krono.getterReply);
      obj[U("type")] = json_objtype(krono.ownType);
      obj[U("caption")] = json_string(krono.captionText);
      obj[U("panes")] = json::value::array();
      int count = 0;
      for (auto & p : krono.paneKeys_topToBottom) {
         obj[U("panes")][count++] = json_paneinkrono(p, key);
      }
      obj[U("timestamps")] = json_array(krono.timestamps_olderToNewer);
      obj[U("knobs")] = json::value::array();
      count = 0;
      for (auto & kkey : krono.knobKeys) {
          obj[U("knobs")][count++] = json_knob(kkey);
      }
   } catch(...) {
      obj[U("error")] = json_string("Error fetching krono info");
   }
   return(obj);
}

static void json_object_merge(json::value &target, const json::value &copyfrom) {
   //ucout << "json_object_merge: copying fields into target" << endl;
   //ucout << " Copying from:" << endl << copyfrom << endl;
   //ucout << " Copying into:" << endl << target << endl;
   const auto & obj = copyfrom.as_object();
   for (auto iter = obj.cbegin(); iter != obj.cend(); ++iter) {
      const utility::string_t &k = iter->first;
      const json::value &v = iter->second;
      if (!target.has_field(k)) {
         target[k] = json::value(v);   // create a new instance for the target
         //ucout << "  merged field " << k << endl;
      }
   }
   //ucout << " Result:" << endl << target << endl << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// Code below are handlers for the various HTTP request types
//
///////////////////////////////////////////////////////////////////////////////


//
// GET Request
//
void handler::handle_get(http_request message)
{
   auto uri = message.relative_uri();
   //auto paths = http::uri::split_path(uri.path());
   auto querystringmap = http::uri::split_query(uri.query());
   bool compress = false;

   //auto encoding = message.headers().find(U("Accept-Encoding"));
   //if (encoding != message.headers().end()) {
   //   // if *encoding contains "gzip" then set the compress flag
   //   if (encoding->second.find("gzip") != std::string::npos)
   //      compress = true;
   //}

   // Pull in any alerts and store them in our local alert list. We have to keep
   // them because the library throws them away as soon as it reports them. Lock
   // everything for thread safety.
   {
      std::lock_guard<std::mutex> glock(instance_api_lock);
      auto newalerts = p_Port->SayNewAlertsFifoFromDomainThenClear();   // std::queue<std::string>

      // Make sure we keep the total alert count under control
      int s = alerts.size();
      int rm = newalerts.size() + s - MAX_ALERT_BUFFER_SIZE;
      auto realNumToRemove = std::min(std::max(0, rm), s);
      auto eraseIter = alerts.begin();
      std::advance(eraseIter, realNumToRemove);
      alerts.erase(alerts.begin(), eraseIter);

      // Move the new alerts into our stashed alert list.
      while (!newalerts.empty()) {
         alerts.insert(std::make_pair(alertseq++, newalerts.front()));
         newalerts.pop();
      }
   }

   std::smatch match;
   auto api = api_latest_version;
   auto retval = status_codes::OK;
   bool recurse = true;
   json::value reply;
   if (std::regex_match(uri.path(), match, re_api)) {
      if (match[2].length() > 0) {
         api = std::max(1,std::min(std::stoi(match[1]), api_latest_version));
         ucout << "Set API version to " << match[1] << endl;
      }
      auto path = match[3].str();
      json::value jvalue;
      try {
         handler::extract_json(message, jvalue);
      } catch (...) {
         // Error parsing the json, probably a syntax error
         ucout << U("Warning: extract_json failed - probably a syntax error") << endl;
      }
      int compact = 0;
      if (handler::get_json_value(true, jvalue, querystringmap, U("compact"), reply, compact)) {
         if (compact != 0)
            recurse = false;
      }
      if (path == U("/apiver") || path == U("/api") || path == U("/noop")) {
         ; // apiver is returned with all requests

      } else if (path == U("/ctrl/eventwait")) {
         // wait for a timeout (param - default = 5 seconds); return either
         // update:true or update:false
         auto funcname = U("WaitForEvent");
         uint64_t oldseq = 0;
         unsigned int timeout = default_eventwait_seconds;
         handler::get_json_value(true, jvalue, querystringmap, U("timeout"), reply, timeout);   // optional
         if (handler::get_json_value(false, jvalue, querystringmap, U("seq"), reply, oldseq)) {  // required
            // wait until either timeout or seq is no longer current
            std::cout << funcname << ": Waiting for event (seq " << oldseq << ")" << std::endl;
            if (handler::wait_for_event(oldseq, timeout).get()) {
               std::cout << funcname << ": Seq update detected (now " << seq << ")" << std::endl;
            } else {
               std::cout << funcname << ": Timed out waiting for seq update (still " << seq << ")" << std::endl;
               reply[U("timedout")] = json::value(true);
            }
         } else {
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/domain")) {
         auto funcname = U("SayInfoFromDomain");
         reply[U("label")] = json_string(domain.ownNameText);
         reply[U("subjectkeys")] = json::value::array();
         if (recurse)
            reply[U("subjects")] = json::value::array();
         int i = 0;
         for (auto & skey : domain.subjectKeys) {
            if (recurse)
               reply[U("subjects")][i] = json_subject(skey);
            reply[U("subjectkeys")][i++] = json_key(skey);
         }

      } else if (path == U("/casekeys")) {
         // return json array of case labels (note: these are not rawlabels!)
         auto funcname = U("CaseKeys"); // THIS IS AN EXTRA COMPOUND GETTER - funcname is made up
         uint64_t subjectkey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey)) {
            NGuiKey subject(subjectkey);
            try {
               auto cases = p_Port->SayCurrentCasesFromSubject(subject);
               reply[U("casekeys")] = json_array(cases.currentCaseKeys);
            } catch (...) {
               stringstream msg;
               ucout << funcname << ": Invalid subject key provided in request" << endl;
               retval = status_codes::BadRequest;
            }
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/casecounts")) {
         auto funcname = U("SayCaseCountsForAllSubjects");
         reply[U("casecounts")] = json::value::array();
         int i = 0;
         try {
            for (auto & skey : domain.subjectKeys) {
               reply[U("casecounts")][i] = json::value::object();
               reply[U("casecounts")][i][U("subject")] = json_key(skey);
               auto cases = p_Port->SayCurrentCasesFromSubject(skey);
               reply[U("casecounts")][i][U("cases")] = json_num((int) cases.currentCaseKeys.size());
               i++;
            }
         } catch (...) {
            stringstream msg;
            ucout << funcname << ": Unable to query subjects" << endl;
            reply[U("error")] = json_string("Unable to query subjects");
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/case")) {
         // return a single json object with all info about a certain case
         compress = true;
         auto funcname = U("SayCase"); // THIS IS AN EXTRA COMPOUND GETTER - funcname is made up
         uint64_t keykey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, keykey)) {
            NGuiKey key(keykey);
            json_object_merge(reply, handler::json_case(key, recurse));
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

     } else if (path == U("/cases")) {
         // return json array of case objects
         auto funcname = U("SayCases"); // THIS IS AN EXTRA COMPOUND GETTER - funcname is made up
         json::value caselist = json::value::array();
         uint64_t subjectkey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey)) {
            NGuiKey subject(subjectkey);
            try {
               auto cases = p_Port->SayCurrentCasesFromSubject(subject);
               int i = 0;
               for (auto const& key : cases.currentCaseKeys) {
                  caselist[i] = json::value::object();
                  json_object_merge(caselist[i], handler::json_case(key, recurse));
                  i++;
               }
            } catch(...) {
            }
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         compress = true;
         reply[U("cases")] = caselist;

      } else if (path == U("/featurekeys")) {
         auto funcname = U("FeatureKeys");
         uint64_t subjectkey;

         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey)) {
            NGuiKey subject(subjectkey);
            try {
               GuiPackSubjectBasic_t s = p_Port->SayInfoFromSubject(subject);
               reply[U("featurekeys")] = json_array(s.featureKeys);
            } catch(...) {
               stringstream msg;
               ucout << funcname << ": Invalid subject key provided in request" << endl;
               retval = status_codes::BadRequest;
            }
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/feature")) {
         auto funcname = U("SayFeature"); // THIS IS AN EXTRA COMPOUND GETTER - funcname is made up
         uint64_t featurekey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, featurekey)) {
            NGuiKey feature(featurekey);
            json_object_merge(reply, handler::json_feature(feature, recurse));
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/features")) {
         auto funcname = U("SayFeatures"); // THIS IS AN EXTRA COMPOUND GETTER - funcname is made up
         int fcount = 0;
         json::value featurelist = json::value::array();
         uint64_t subjectkey;

         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey)) {
            NGuiKey subject(subjectkey);
            try {
               GuiPackSubjectBasic_t s = p_Port->SayInfoFromSubject(subject); // Could probably be cached...?
               int i = 0;
               for (auto const &key : s.featureKeys) {
                  featurelist[i] = json::value::object();
                  json_object_merge(featurelist[i], handler::json_feature(key, recurse));
                  i++;
               }
            } catch(...) {
            }
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         compress = true;
         reply[U("features")] = featurelist;

      } else if (path == U("/krono")) {
         auto funcname = U("SayKrono");
         int i = 0;
         uint64_t kronokey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, kronokey)) {
            NGuiKey krono(kronokey);
            json_object_merge(reply, handler::json_krono(krono, recurse));

         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         compress = true;

      } else if (path == U("/subjectkeys")) {
         auto funcname = U("SaySubjectKeys");
         reply[U("subjectkeys")] = json_array(domain.subjectKeys);

      } else if (path == U("/subjects")) {
         auto funcname = U("SaySubjects");
         reply[U("subjects")] = json::value::array();
         utility::string_t detailsliststr;
         std::set<uint64_t> detailkeys;
         if (handler::get_json_value(true, jvalue, querystringmap, U("details"), reply, detailsliststr)) {
            recurse = false;
            ucout << "GET subjects: trying to get details for " << detailsliststr << endl;
            std::stringstream s_stream(detailsliststr); //create string stream from the string
            while(s_stream.good()) {
               std::string substr;
               getline(s_stream, substr, ','); //get first string delimited by comma
               try {
                  detailkeys.insert(std::stol(substr));
                  //ucout << "GET subjects: added detail for str '" << substr << "'" << endl;
              } catch (...) {
                  // unable to convert arg to long int - no good way to handle, so skip it
                  ucout << "GET subjects: unable to add detail for str '" << substr << "' " << endl;
              }
            }
         }
         int i = 0;
         for (auto & skey : domain.subjectKeys) {
            if (detailkeys.find(skey.Peek()) != detailkeys.end()) {
                reply[U("subjects")][i++] = json_subject(skey, true);
            } else {
                reply[U("subjects")][i++] = json_subject(skey, recurse);
            }
         }
         compress = true;

      } else if (path == U("/subject")) {
         auto funcname = U("SaySubject");
         int i = 0;
         uint64_t subjectkey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey)) {
            NGuiKey subject(subjectkey);
            json_object_merge(reply, handler::json_subject(subject, recurse));

         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         compress = true;

      } else if (path == U("/alerts")) {
         auto funcname = U("SayCachedAlerts");
         auto alertlist = json::value::array();
         int i = 0;
         std::lock_guard<std::mutex> glock(instance_api_lock);
         for (auto & a : alerts) {
            alertlist[i] = json::value();
            alertlist[i][U("id")] = json::value(a.first);
            alertlist[i][U("message")] = json_string(a.second);
            i++;
         }
         reply[U("alerts")] = alertlist;
         compress = true;

      } else {
         ucout << "Unrecognized GET path: " << match[3] << endl;
         reply[U("error")] = json::value::string(U("Unrecognized GET path (" + match[3].str() + ")"));
         retval = status_codes::NotImplemented;
      }
   }
   reply[U("apiver")] = json::value(api);   // Always include the API version in the reply
   reply[U("seq")] = json::value(seq);      // Always include the sequence number in the reply
   reply[U("alertseq")] = json::value(alertseq);
   //message.reply(retval, reply);
   http_response response (retval);
   response.headers().add(U("Cache-Control"), U("no-cache"));
   response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // TODO SECURITY: GET RID OF *
   if (compress) {
      std::stringstream body;
      reply.serialize(body);
      auto gzbody = Gzip::compress(body.str());
      response.set_body(gzbody);
      response.headers().add(U("Content-Encoding"), U("gzip"));
   } else {
      response.set_body(reply);
   }
   message.reply(response);         // reply is done here
   return;
};

//
// A POST request
//
void handler::handle_post(http_request message)
{
   auto uri = message.relative_uri();
   //auto paths = http::uri::split_path(uri.path());
   auto querystringmap = http::uri::split_query(uri.query());

   std::smatch match;
   auto api = api_latest_version;
   auto retval = status_codes::OK;
   json::value reply;
   if (std::regex_match(uri.path(), match, re_api)) {
      if (match[2].length() > 0) {
         api = std::max(1,std::min(std::stoi(match[1]), api_latest_version));
         ucout << "Set API version to " << match[1] << endl;
      }
      auto path = match[3].str();
      if (path == U("/ctrl/singlestep")) {
         p_Port->SingleStepDomainOnTimeAndInputs();
         update_seq();
         //ucout << "SingleStepDomainOnTimeAndInputs() called" << endl;
      } else if (path == U("/ctrl/shutdown")) {
         extern std::atomic<int> stop_main;   // tell the main loop that we want to shut down
         stop_main.store(1);
         ucout << "Shutdown requested due to API request" << endl;
      } else {
         ucout << "Unrecognized POST path: " << match[3] << endl;
         reply[U("error")] = json::value::string(U("Unrecognized POST path (" + match[3].str() + ")"));
         retval = status_codes::NotImplemented;
      }
   }
   reply[U("apiver")] = json::value(api);   // Always include the API version in the reply
   reply[U("seq")] = json::value(seq);      // Always include the sequence number
   //message.reply(retval, reply);
   http_response response (retval);
   response.headers().add(U("Cache-Control"), U("no-cache"));
   response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // TODO SECURITY: GET RID OF *
   response.set_body(reply);
   message.reply(response);         // reply is done here
   return;
};


//
// A PUT request
//
void handler::handle_put(http_request message)
{
   auto uri = message.relative_uri();
   //auto paths = http::uri::split_path(uri.path());
   auto querystringmap = http::uri::split_query(uri.query());

   std::smatch match;
   auto api = api_latest_version;
   auto retval = status_codes::OK;
   json::value reply;
   stringstream msg;
   if (std::regex_match(uri.path(), match, re_api)) {
      if (match[2].length() > 0) {
         api = std::max(1,std::min(std::stoi(match[1]), api_latest_version));
         ucout << "Set API version to " << match[1] << endl;
      }
      auto path = match[3].str();
      json::value jvalue;
      try {
         handler::extract_json(message, jvalue);
      } catch (...) {
         // Error parsing the json, probably a syntax error
         ucout << U("Warning: extract_json failed - probably a syntax error") << endl;
      }

      if (path == U("/ctrl/time")) {
         const auto funcname = U("SetTimeStampInDomain");
         time_t timestamp;
         if (handler::get_json_value(false, jvalue, querystringmap, U("time"), reply, timestamp)) {
            std::tm tm;
            localtime_s(&tm, &timestamp);
            TRYAPI1(time,
               p_Port->SetTimeStampInDomain(tm);
               reply[U("status")] = json::value(U("time set"));
               reply[U("returncode")] = json::value(0);
               retval = status_codes::OK;
            );
         } else {
            cout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/ctrl/sample")) {
         const auto funcname = U("SetCoincidentInputsForSubject");
         json::value values;
         uint64_t subjectkey;
         if (handler::get_json_value(false, jvalue, querystringmap, U("subject"), reply, subjectkey) &&
            handler::get_json_value(false, jvalue, querystringmap, U("values"), values)) {
            NGuiKey subject(subjectkey);
            if (values.is_array()) {
               auto a = values.as_array();
               // a is now a json::value::array (hopefully of doubles)
               int count = a.size();
               if (count > 0 && count < 10000) {  // MAXIMUM 9999 values accepted in a sample (make into a constant!)
                  // WARNING: dlist gets freed at end of block, so if Dan's code isn't
                  // making a copy of it there WILL be problems!
                  std::vector<double> dlist;
                  try {
                     for (auto const& v : a) {
                        dlist.push_back(v.as_double());
                     }
                  } catch (...) {
                     stringstream msg;
                     msg << U("Invalid value type likely due to non-numeric data");
                     ucout << msg.str() << endl;
                     reply[U("error")] = json::value(msg.str());
                     reply[U("returncode")] = json::value(-1);
                     retval = status_codes::BadRequest;
                     goto done;
                  }
                  TRYAPI1(values,
                     p_Port->SetCoincidentInputsForSubject(dlist, subject);
                     update_seq();
                     stringstream msg;
                     msg << U("added sample of ") << count << U(" channels");
                     reply[U("returncode")] = json::value(0);
                     reply[U("status")] = json::value(msg.str());
                  );
               } else {
                  stringstream msg;
                  msg << U("channel count out of range");
                  ucout << funcname << U(": ") << msg.str() << endl;
                  reply[U("error")] = json::value(msg.str());
                  reply[U("returncode")] = json::value(-1);
                  retval = status_codes::BadRequest;
               }
            } else {
               stringstream msg;
               msg << U("values parameter must be array of doubles");
               ucout << funcname << U(": ") << msg.str() << endl;
               reply[U("error")] = json::value(msg.str());
               reply[U("returncode")] = json::value(-1);
               retval = status_codes::BadRequest;
            }
         } else {
            stringstream msg;
            msg << U("values parameter not found");
            ucout << funcname << U(": ") << msg.str() << endl;
            reply[U("error")] = json::value(msg.str());
            reply[U("returncode")] = json::value(-1);
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/ctrl/sampletimestep")) {
         const auto funcname = U("SampleTimeStep");
         time_t timestamp;
         json::value valuesbysubject;
         uint64_t subjectkey;
         std::lock_guard<std::mutex> stslock(instance_sampletimestep_lock);  // released at end of scope
         if (  handler::get_json_value(false, jvalue, querystringmap, U("values_by_subject"), valuesbysubject) &&
               handler::get_json_value(false, jvalue, querystringmap, U("time"), reply, timestamp) ) {
            std::tm tm;
            localtime_s(&tm, &timestamp);
            TRYAPI1(time,
               p_Port->SetTimeStampInDomain(tm);
               reply[U("status")] = json::value(U("time set"));
               reply[U("returncode")] = json::value(0);
               retval = status_codes::OK;
            );
            if (valuesbysubject.is_array() && retval == status_codes::OK) {
               for (const auto & json_o : valuesbysubject.as_array()) {
                  // json_o is a json object with properties "subject" and "values"
                  uint64_t subjectkey = 0;
                  try { subjectkey = json_o.at(U("subject")).as_integer(); } catch (...) { /* NEED ERROR MESSAGE HERE */ continue; };
                  NGuiKey subject(subjectkey);
                  auto points = p_Port->SayInputPointNameOrderExpectedBySubject(subject);
                  try {
                     auto a = json_o.at(U("values")).as_array();
                     int count = a.size();
                     // TODO: We don't currently have any way to validate the point names
                     // (But for now we can at least validate the number of points matches
                     // what is expected by this subject)
                     if (points.size() == count && count > 0) {
                        // WARNING: dlist gets freed at end of block, so if Dan's code isn't
                        // making a copy of it there WILL be problems!
                        std::vector<double> dlist;
                        try {
                           for (auto const& v : a) {
                              dlist.push_back(v.as_double());
                           }
                        } catch (...) {
                           stringstream msg;
                           msg << U("Invalid value type likely due to non-numeric data");
                           ucout << msg.str() << endl;
                           reply[U("error")] = json::value(msg.str());
                           reply[U("returncode")] = json_reply(EGuiReply::FAIL_set_givenValueOutOfRangeAllowed);
                           retval = status_codes::BadRequest;
                           goto done;
                        }
                        TRYAPI1(valuesbysubject,
                           p_Port->SetCoincidentInputsForSubject(dlist, subject);
                           stringstream msg;
                           msg << U("added sample of ") << count << U(" channels to subject ") << subjectkey;
                           reply[U("returncode")] = json_reply(EGuiReply::OKAY_allDone);
                           reply[U("status")] = json::value(msg.str());
                        );
                     } else {
                        stringstream msg;
                        msg << U("channel count out of range for subject ") << subjectkey;
                        ucout << funcname << U(": ") << msg.str() << endl;
                        reply[U("error")] = json::value(msg.str());
                        reply[U("returncode")] = json_reply(EGuiReply::FAIL_set_givenContainerWrongSizeForKeyGiven);
                        retval = status_codes::BadRequest;
                     }
                  } catch (...) {
                     /* NEED ERROR MESSAGE HERE */
                     continue;
                  };
               } // end foreach(subject)

            } else {
               stringstream msg;
               msg << U("values_by_subject parameter must be array of objects with subject and values parameters");
               ucout << funcname << U(": ") << msg.str() << endl;
               reply[U("error")] = json::value(msg.str());
               reply[U("returncode")] = json_reply(EGuiReply::FAIL_set_givenValueOutOfRangeAllowed);
               retval = status_codes::BadRequest;
            }
            if (retval == status_codes::OK) {
               p_Port->SingleStepDomainOnTimeAndInputs();
               update_seq();
            } else {
               ucout << funcname << ": skipping SingleStep because of previous errors" << endl;
            }
         } else {
            stringstream msg;
            msg << U("time and/or values_by_subject parameters not found");
            ucout << funcname << U(": ") << msg.str() << endl;
            reply[U("error")] = json::value(msg.str());
            reply[U("returncode")] = json_reply(EGuiReply::FAIL_set_givenValueOutOfRangeAllowed);
            retval = status_codes::BadRequest;
         }

      } else if (path == U("/ctrl/answercase")) {
         // EGuiReply AnswerCaseOnSubjectWithZeroBasedOptionIndex( NGuiKey, NGuiKey, size_t );

         const auto funcname = U("AnswerCaseOnSubjectWithZeroBasedOptionIndex");
         EGuiReply returncode = EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
         uint64_t casekeykey;
         int answer;
         if (handler::get_json_value(false, jvalue, querystringmap, U("case"), reply, casekeykey) &&
               handler::get_json_value(false, jvalue, querystringmap, U("answer"), reply, answer)) {
            auto casekey = NGuiKey(casekeykey);
            TRYAPI2(casekeykey, answer,
               returncode = p_Port->AnswerCaseWithZeroBasedOptionIndex(casekey, (size_t) answer);
               update_seq();
            );
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         reply[U("returncode")] = json_reply(returncode);
         reply[U("success")] = returncode != EGuiReply::OKAY_allDone ? json::value(false) : json::value(true);

      } else if (path == U("/set/knob")) {
         auto funcname = U("SetKnobToValue");
         auto returncode = EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
         uint64_t knobkey;
         GuiFpn_t fpnvalue;
         int intvalue;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, knobkey) &&
               handler::get_json_value(false, jvalue, querystringmap, U("value"), reply, fpnvalue)) {
            TRYAPI2(knobkey, fpnvalue,
               returncode = p_Port->SetKnobToValue(NGuiKey(knobkey), fpnvalue);
               update_seq();
            );
         } else if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, knobkey) &&
               handler::get_json_value(false, jvalue, querystringmap, U("value"), reply, intvalue)) {
            fpnvalue = (GuiFpn_t) intvalue;
            TRYAPI2(knobkey, fpnvalue,
               returncode = p_Port->SetKnobToValue(NGuiKey(knobkey), fpnvalue);
               update_seq();
            );
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         reply[U("returncode")] = json_reply(returncode);
         reply[U("success")] = returncode != EGuiReply::OKAY_allDone ? json::value(false) : json::value(true);

      } else if (path == U("/set/histogram/mode")) {
         auto funcname = U("SetHistogramMode");
         auto returncode = EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
         uint64_t hkey;
         size_t intvalue;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, hkey) &&
               handler::get_json_value(false, jvalue, querystringmap, U("value"), reply, intvalue)) {
            TRYAPI2(hkey, intvalue,
               returncode = p_Port->SetModeOfHistogramToZeroBasedOptionIndex(NGuiKey(hkey), intvalue);
               update_seq();
            );
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         reply[U("returncode")] = json_reply(returncode);
         reply[U("success")] = returncode != EGuiReply::OKAY_allDone ? json::value(false) : json::value(true);

      } else if (path == U("/set/histogram/span")) {
         auto funcname = U("SetHistogramSpan");
         auto returncode = EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
         uint64_t hkey;
         size_t intvalue;
         if (handler::get_json_value(false, jvalue, querystringmap, U("key"), reply, hkey) &&
               handler::get_json_value(false, jvalue, querystringmap, U("value"), reply, intvalue)) {
            TRYAPI2(hkey, intvalue,
               returncode = p_Port->SetSpanOfHistogramToZeroBasedOptionIndex(NGuiKey(hkey), intvalue);
               update_seq();
            );
         } else {
            stringstream msg;
            ucout << funcname << ": " << reply[U("error")].as_string() << endl;
            retval = status_codes::BadRequest;
         }
         reply[U("returncode")] = json_reply(returncode);
         reply[U("success")] = returncode != EGuiReply::OKAY_allDone ? json::value(false) : json::value(true);

      } else {
         stringstream msg;
         msg << "Unrecognized PUT path: " << path;
         ucout << msg.str() << endl;
         reply[U("error")] = json::value(msg.str());
         retval = status_codes::NotImplemented;
      }
   }
done:
   reply[U("apiver")] = json::value(api);   // Always include the API version in the reply
   reply[U("seq")] = json::value(seq);      // Always include the sequence number in the reply
   //message.reply(retval, reply);
   http_response response (retval);
   response.headers().add(U("Cache-Control"), U("no-cache"));
   response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // TODO SECURITY: GET RID OF *
   response.set_body(reply);
   message.reply(response);         // reply is done here
   return;
}

//
// A DELETE request
//
void handler::handle_delete(http_request message)
{
   auto uri = message.relative_uri();
   auto api = api_latest_version;
   auto retval = status_codes::NotImplemented;
   json::value reply;
   ucout << "Unrecognized DELETE path: " << uri.path() << endl;
   reply[U("error")] = json::value::string(U("Unrecognized DELETE path (" + uri.path() + ")"));
   reply[U("apiver")] = json::value(api);   // Always include the API version in the reply
   reply[U("seq")] = json::value(seq);      // Always include the sequence number in the reply
   //message.reply(retval, reply);
   http_response response (retval);
   response.headers().add(U("Cache-Control"), U("no-cache"));
   response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // TODO SECURITY: GET RID OF *
   response.set_body(reply);
   message.reply(response);         // reply is done here
   return;
};

//
// An OPTIONS request
//
// TODO: CORS needs work. I think the Allow-Origin: * should be changed to use the Origin header value from the message...?
//
void handler::handle_options(http_request message)
{
   auto uri = message.relative_uri();
   auto retval = status_codes::OK;
   json::value reply;
   //ucout << "Received OPTIONS request for: " << uri.path() << endl;
   //message.reply(retval, reply);
   http_response response (retval);
   response.headers().add(U("Cache-Control"), U("no-cache"));
   response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // TODO SECURITY: GET RID OF *
   response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
   response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
   response.set_body(reply);
   message.reply(response);         // reply is done here
   return;
};
