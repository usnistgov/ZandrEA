#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <map>
#include <time>
//#include "stdafx.h"

typedef uint64_t session_id;

class session
{
   public:
      session(const utility::string_t&);   // remote IP address
      ~session();

      session_id key();
      time_t created();
      time_t accessed();
      const utility::string_t& ip();
      void touch();  // update access time
      utility::string_t str();

   protected:

   private:
      session_id key;
      time_t time_created;
      time_t time_accessed;
      utility::string_t remote_ip;
}

class sessionmap
{
   public:
      typedef std::map<session_id, session> SessionMap_t;
      //typedef SessionMap_t::iterator iterator;
      //typedef SessionMap_t::const_iterator const_iterator;
      //typedef SessionMap_t::reference reference;

      sessionmap();
      ~sessionmap();

      int size();
      bool has_key(const session_id);
      session& key(const session_id);
      const std::vector<session_id> keys();

      //iterator begin() { return map.begin(); }
      //const_iterator begin() const { return map.begin(); }
      //iterator end() { return map.end(); }
      //const_iterator end() const { return map.end(); }

      int forceout(int);      // force X oldest items out of map to make space
      int expire();           // remove expired sessions from map
      int purge();            // remove all sessions from map

   protected:

   private:
      SessionMap_t map;
};

#endif // SESSION_H
