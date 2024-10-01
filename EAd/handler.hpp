#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include <regex>
#include <mutex>
#include <atomic>
#include <map>
#include "stdafx.h"
#include "exportCalls.hpp"

#define DEFAULT_EVENTWAIT_SECONDS 30
#define MAX_ALERT_BUFFER_SIZE 128

typedef uint64_t AlertId_t;

class handler
{
   public:
      //handler();
#ifdef USE_SSL
      handler(const utility::string_t&, const web::http::experimental::listener::http_listener_config&, IExportOmni*);
#else
      handler(const utility::string_t&, IExportOmni*);
#endif
      virtual ~handler();

      pplx::task<void>open()  {return m_listener.open();}
      pplx::task<void>close() {return m_listener.close();}

   protected:

   private:
      void handle_get(web::http::http_request message);
      void handle_put(web::http::http_request message);
      void handle_post(web::http::http_request message);
      void handle_delete(web::http::http_request message);
      void handle_options(web::http::http_request message);
      void handle_error(pplx::task<void>& t);
      pplx::task<bool> wait_for_event(uint64_t, unsigned int);
      void update_seq(void);
      // generate JSON objects from EA objects
      const web::json::value json_subject(const NGuiKey &, bool recurse = true);
      const web::json::value json_rulekit(const NGuiKey &, bool recurse = true);
      const web::json::value json_case(const NGuiKey &, bool recurse = true);
      const web::json::value json_feature(const NGuiKey &, bool recurse = true);
      const web::json::value json_knob(const NGuiKey &, bool recurse = true);
      const web::json::value json_histogram(const NGuiKey &, bool recurse = true);
      const web::json::value json_traceinkrono(const NGuiKey &, const NGuiKey &, bool recurse = true);
      const web::json::value json_paneinkrono(const NGuiKey &, const NGuiKey &, bool recurse = true);
      const web::json::value json_krono(const NGuiKey &, bool recurse = true);

      web::http::experimental::listener::http_listener m_listener;

      std::mutex instance_api_lock;
      std::mutex instance_sampletimestep_lock;

      // condition_variable monitors seq so tasks can wait for updates efficiently
      std::atomic<uint64_t> seq;    // the global atomic sequence counter
      std::mutex cvm;
      std::condition_variable cv;

      // Keep a local copy of the global EA pointer
      IExportOmni* p_Port;

      // Keep local copies of "UNCHANGING" EA information and precomputed lookup information
      GuiPackDomain_t domain;

      // Keep a list of the last X alert messages. We don't have sessions so the clients
      // will have to keep track of which ones they've seen.
      std::map<AlertId_t, std::string> alerts;
      std::atomic<AlertId_t> alertseq;  // id number for alerts

      // Since we don't currently have sessions, the client must keep track of the
      // current subject. Don't store one here!

      static void extract_json(const web::http::http_request&, web::json::value&);
      static bool get_querystring(const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, utility::string_t&);

      // Here we try to consolidate
      // json body object to parse, key to look for, json value of key, json reply object to put error and status messages into, and the raw value (if it could be determined)
      // returns true if found and no errors extracting type, or false if not
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, int&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, uint64_t&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, unsigned int&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, time_t&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, utility::string_t&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, char&);
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, double&);
#ifdef __arm64__
      static bool get_json_value(const bool, const web::json::value&, const std::map<utility::string_t,utility::string_t>&, const utility::string_t&, web::json::value&, size_t&);
#endif

      static const std::regex re_api;
      static const unsigned int default_eventwait_seconds = DEFAULT_EVENTWAIT_SECONDS;
};

#endif // HANDLER_H
