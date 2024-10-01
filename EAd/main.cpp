// main.cpp : Defines the entry point for the application.

//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include <string>
#include <iostream>
#include <limits>
#include <csignal>
#include <mutex>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#ifdef USE_SSL
# include <boost/asio.hpp>
# include <boost/asio/ssl.hpp>
#endif
#include "stdafx.h"
#include "handler.hpp"
#include <pplx/threadpool.h>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
using namespace boost::filesystem;
namespace po = boost::program_options;
#ifdef USE_SSL
namespace ssl = boost::asio::ssl;
#endif

std::unique_ptr<handler> g_httpHandler;
std::atomic<int> stop_main;   // store true when ready to shut down
IExportOmni* tool;     // master pointer to the DLL API

#ifdef USE_SSL
http_listener_config server_config;
#endif

#ifdef USE_SSL
void on_initialize(const string_t& address, const string_t& keyfile, const string_t& certfile, IExportOmni* tool)
#else
void on_initialize(const string_t& address, IExportOmni* tool)
#endif
{
   uri_builder uri(address);
#ifdef USE_SSL
   uri.set_scheme("https");

   server_config.set_ssl_context_callback(
      [&](boost::asio::ssl::context & ctx) {
         boost::system::error_code ec;

         //ctx.load_verify_file(keyfile);

         //ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3 | ssl::context::no_tlsv1, ec);
         ctx.set_options(ssl::context::default_workarounds, ec);
         std::cout << "lambda::set_options " << ec.value() << " " << ec.message() << std::endl;

         ctx.use_certificate_chain_file(certfile, ec);
         std::cout << "lambda::use_certificate_file " << ec.value() << " " << ec.message() << std::endl;

         ctx.use_private_key_file(keyfile, ssl::context::pem, ec);
         std::cout << "lambda::use_private_key " << ec.value() << " " << ec.message() << std::endl;
      });
#endif

   auto addr = uri.to_uri().to_string();
#ifdef USE_SSL
   g_httpHandler = std::unique_ptr<handler>(new handler(addr, server_config, tool));
#else
   g_httpHandler = std::unique_ptr<handler>(new handler(addr, tool));
#endif
   g_httpHandler->open().wait();

   ucerr << utility::string_t(U("Listening for requests at: ")) << addr << endl;

   return;
}

void on_shutdown()
{
   std::cout << "Shutting down http handler..." << std::endl;
   g_httpHandler->close().wait();
   if (tool) {
      std::cout << "Shutting down DLL..." << std::endl;
      tool->PrepareApplicationForShutdown();
   }
   return;
}

void signalHandler(int signum) {
   stop_main.store(1);
   std::cout << "Shutdown request set due to signal" << std::endl;
}

int main(int argc, char **argv) {
   // Process command line options
   const int default_port = 34568;
#ifdef USE_SSL
   const utility::string_t default_url = U("https://127.0.0.1");
   const utility::string_t default_keyfile = default_dir + U("/sslkey.pem");
   const utility::string_t default_certfile = default_dir + U("/sslcert.pem");
#else
   const utility::string_t default_url = U("http://127.0.0.1");
#endif
   const utility::string_t default_dir = U(".");
   bool interactive = false;
   bool fg = false;

   // Initialize the CPPREST/PPLX thread pool
   // NOTE: SWB: TODO: default is 40, min is 2 (hangs with 1) - reduced to ease debugging!
   crossplat::threadpool::initialize_with_threads(5);

   // Initialize the EA library
   // Need to reference the library's init function here so it gets linked, otherwise it won't be called
   void LibMain(void);
   void (*f)() = LibMain;
   tool = (IExportOmni *)NULL;

   po::variables_map vm;
   po::options_description desc{"Options"};
   try {
      desc.add_options()
         ("help,h", "Help screen")
         ("baseurl,b", po::value<std::string>()->default_value(default_url),"Base URL")
         ("port,p",    po::value<int>()->default_value(default_port),       "Port to listen on")
#ifdef USE_SSL
         ("sslkey,k",  po::value<std::string>()->default_value(default_keyfile),"SSL key PEM file")
         ("sslcert,c", po::value<std::string>()->default_value(default_certfile),"SSL certficate chain PEM file")
#endif
         ("workdir,w", po::value<std::string>()->default_value(default_dir),"Work directory")
         ("fg,f",      po::bool_switch(&fg),                                "Run in foreground")
         ("interactive,i",po::bool_switch(&interactive),                    "Run until user hits return");

      po::store(parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   } catch (const po::error &e) {
      std::cerr << e.what() << endl;
      return(1);
   }

   if (vm.count("help")) {
      std::cout << desc << endl;
      return(0);
   }

   int pval;
   std::string address;
   std::string workdir;
#ifdef USE_SSL
   std::string sslkeyfile;
   std::string sslcertfile;
#endif

   try { pval = vm["port"].as<int>(); } catch (...) { cerr << U("Port argument must be a positive integer") << endl; return(1); }
   try { address = std::string(vm["baseurl"].as<std::string>()); } catch (...) { cerr << U("Error parsing BaseURL string") << endl; return(1); }
   try { workdir = std::string(vm["workdir"].as<std::string>()); } catch (...) { cerr << U("Error parsing workdir argument") << endl; return(1); }
#ifdef USE_SSL
   try { sslkeyfile = std::string(vm["sslkey"].as<std::string>()); } catch (...) { cerr << U("Error parsing sslkey argument") << endl; return(1); }
   try { sslcertfile = std::string(vm["sslcert"].as<std::string>()); } catch (...) { cerr << U("Error parsing sslcert argument") << endl; return(1); }
#endif

   if (pval <= 1 or pval > std::numeric_limits<unsigned short>::max()) {
      std::cerr << "Error: illegal port value" << endl;
      return(1);
   }
   utility::string_t port = U( to_string(pval) );
   address.append(U(":"));
   address.append(port);
   if (boost::filesystem::is_directory(workdir)) {
      try { boost::filesystem::current_path(workdir); } catch (...) {
         cerr << U("Unable to change directory to ") << U(workdir) << endl;
         return(1);
      }
   } else {
      cerr << U("Workdir ") << workdir << U(" does not exist.") << endl;
      return(1);
   }
#ifdef USE_SSL
   if (! boost::filesystem::is_regular_file(sslkeyfile)) {
      cerr << sslkeyfile << U(": SSL key PEM file not found") << endl;
      return(1);
   }
   if (! boost::filesystem::is_regular_file(sslcertfile)) {
      cerr << sslcertfile << U(": SSL cert chain PEM file not found") << endl;
      return(1);
   }
#endif

   // Get the pointer to the EA objects
   try { tool = SExportedHandles::GetPortPointer(); } catch (...) { cerr << U("Unable to obtain the EA port pointer") << endl; return(1); }
   if (! tool) {
      cerr << U("NULL EA port pointer returned; aborting") << endl;
      return(1);
   }

   // Now invoke cpprest listener and event handler
   if (! fg) {
      // TODO: Daemonize me
      cerr << "WARNING: daemonizing code is not implemented yet!" << endl;
   }
   cerr << "(Running in foreground: interrupt or send signal to quit)" << endl;

   // Start the listener
#ifdef USE_SSL
   on_initialize(address, sslkeyfile, sslcertfile, tool);
#else
   on_initialize(address, tool);
#endif

   if (interactive) {
      // Wait for user to press return
      std::cerr << "Press ENTER to exit." << std::endl;
      std::string line;
      std::getline(std::cin, line);
   } else {
      // Wait for a signal to stop
      stop_main.store(0);
      std::signal(SIGINT, signalHandler);
      std::signal(SIGHUP, signalHandler);
      std::signal(SIGQUIT, signalHandler);
      std::signal(SIGTERM, signalHandler);

      while(!stop_main.load()) {
         sleep(1);
      }
   }

   // Stop the listener and close up shop
   on_shutdown();

   // and done
   cerr << "Stopped cleanly." << endl;
   return(0);
}



//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
