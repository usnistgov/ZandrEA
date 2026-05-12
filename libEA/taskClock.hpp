//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* Source code file to an "EA" part of the ZandrEA (tm) project at: https://github.com/usnistgov/ZandrEA
This file last edited in base repo by: DAV, U.S. National Institute of Standards and Technology (NIST).
As a Work of the United States Government, this file is not subject to copyright within the United
States. For other countries, Copyright 2025-2026 National Institute of Standards and Technology.
For countries other than the United States, this file is licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy
of the License at: https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License. */
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* File summary:
   Header declaring classes needed for Clock objects.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

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
