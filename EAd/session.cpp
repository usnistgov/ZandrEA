#include "session.hpp"
#include <random>
#include <ctime>
#include <limits>
#include <sstream>

namespace std;

static const int max_session_count = 1024;
static const int max_session_idle_seconds = 3600;
static const int max_session_seconds = (3600 * 24);
static const std::random_device rd;
static const std::mt19937 gen(rd());
static const std::uniform_int_distribution<session_id> dis(std::numeric_limits<session_id>::min(),std::numeric_limits<session_id>::max());

// Create a new session object and store its corresponding remote IP address
session::session(const session_id uniquekey, const utility::string_t &ip) {
   key = uniquekey;
   remote_ip = ip;
   created = accessed = time(nullptr);
}

session::~session() {
   // clean up
}

inline session_id session::key() {
   return key;
}

inline time_t session::created() {
   return created;
}

inline time_t session::accessed() {
   return accessed;
}

inline utility::string_t session::ip() {
   return remote_ip;
}

inline void session::touch() {
   accessed = time(nullptr);
}

utility::string_t session::str() {
   stringstream ss;
   ss << std::hex << key;
   return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

sessionmap::sessionmap() {

}

sessionmap::~sessionmap() {

}

inline sessionmap::size() {
   return map.size();
}

inline bool sessionmap::has_key(const session_id id) {
   return (map.find(id) != map.end());
}

inline session& sessionmap::key(const session_id id) {
   return (map.find(id)->second);
}

inline session_id sessionmap::create(const utility::string& ip) {
   session_id randomkey;

   // Generate a random key
   // This is a bit scary, an unbounded loop, but the chances of collisions are so miniscule
   // that I'm calling this safe.
   while(1) {
      randomkey = dis(gen);
      if (! map.has_key(randomkey))
         break;
   }
   auto s = new session(randomkey, ip);

   return(s.key());
}


#if 0
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
      bool has_key(session_id);
      session& key(session_id);
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

#endif
