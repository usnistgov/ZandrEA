// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Header declaring classes needed for Clock objects. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef TASKCLOCK_HPP
#define TASKCLOCK_HPP

#include "customTypes.hpp"
#include <chrono>
#include <ctime>

class CAgent;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare an abstract base class (ABC) for Clock objects.
// Here, "stamp" means a ctime time_t type, "struct" means a tm struct type.

class AClock {

   public:

   // Methods
      ~AClock( void );

      time_t               GetTimestamp( void ) const;
      int                  GetClockHour( void ) const;
      int                  GetClockWday( void ) const;
      int                  SayBellPeriodSecs( void ) const;
      bool                 IsValid( void ) const;
      void                 RegisterForBells( CAgent* );

   protected:

   // Handles
      CAgent*              p_Agent;   // no vector, so currently 1:1 for Clocks:Agents (i.e., tasks)

   // Methods
      explicit AClock( int );

   // Fields
      const int            bellPeriodSecs;   // signed int avoids errors mixing signed/unsigned
      std::tm              timeStructNow;
      std::tm              timeStructWas;
      SClockRead           clockReadAtBell;
      bool                 valid;
};


class CClockPerPort : public AClock { // "Port" refers to interface with Client software

   public: 

   // Methods
       explicit CClockPerPort( int );    // task period [currently "dumb", coordinated off-line with Client]

      ~CClockPerPort( void );

      EGuiReply            SetTimeFromPort( std::tm );
      EGuiReply            RingTaskBell( void );
};


class CClockPerHost : public AClock {  // "Host" refers to CPU of computer hosting this application

   public:

   // Methods
       explicit CClockPerHost( int );   // task period, secs [strike of "bell" causes clock to lauch task]

      ~CClockPerHost( void );

      void                 StartTicking( void );
      void                 StopTicking( void);

   private:

   // Internal types
      typedef std::chrono::steady_clock ChronoMono_t; // any autoRun done by monotonic CPU time
      typedef std::chrono::system_clock ChronoWall_t; // time stamp by system ("wall clock") CPU time
 
   // Member objects
      ChronoMono_t::time_point         bellNow_mono;      // See Class Note [3]
      ChronoMono_t::time_point         bellNext_mono;
      const ChronoMono_t::duration     bellAdvance;

   // Fields
      time_t              bellNow_wall;
      bool                ticking;

   // Methods
      void                TickTock( void );
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
