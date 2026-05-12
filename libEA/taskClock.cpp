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
   Implementation of classes needed for Clock objects. 
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#include "taskClock.hpp"
#include "agentTask.hpp"

#ifndef __linux
  // e.g. MacOS
# include <errno.h>
#endif

#include "portability.hpp"

#include "mvc_model.hpp"


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// AClock (interface) implementation

AClock::AClock( int arg0 )
                        :  p_Agent (nullptr),
                           bellPeriodSecs (arg0), // signed, see note in header
                           timeStructNow (),
                           timeStructWas (),
                           clockReadAtBell ( SClockRead(0, 0, NaNBOOL, NaNBOOL) ),
                           valid (NaNBOOL) {

 // empty clock interface ctor
}


AClock::~AClock( void ) {

   // empty clock interface d-tor

}


time_t AClock::GetTimestamp( void ) const { return clockReadAtBell.timestamp; }


int AClock::GetClockHour( void ) const { return timeStructNow.tm_hour; }


int AClock::GetClockWday( void ) const { return timeStructNow.tm_wday; }


int AClock::SayBellPeriodSecs( void ) const { return bellPeriodSecs; }


bool AClock::IsValid( void ) const { return valid; }


void AClock::RegisterForBells( CAgent* arg ) {

   p_Agent = arg;
   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CClockPerCtrlPort implementation

CClockPerPort::CClockPerPort( int arg0 )
                                    :  AClock(arg0) {
   // empty c-tor
}


CClockPerPort::~CClockPerPort( void ) { /* empty d-tor*/ }


EGuiReply CClockPerPort::SetTimeFromPort( std::tm tmGiven ) {

   time_t tmGivenTranslated = 0;
   tmGivenTranslated = std::mktime( &tmGiven );  // mktime() returns time_t in local time (thread-safe TBD)

   if ( tmGivenTranslated == clockReadAtBell.timestamp) {
      return EGuiReply::FAIL_set_givenTimestampSameAsPrevious;
   }

   timeStructNow = tmGiven;
   clockReadAtBell.timestamp = tmGivenTranslated;
   clockReadAtBell.triggerCount =
      (   clockReadAtBell.triggerCount == FIXED_CLOCK_TRIGGERCOUNTER_RESET ?
          0 :
          clockReadAtBell.triggerCount + 1
      );
   clockReadAtBell.minutesAfterMidnight = ( (timeStructNow.tm_hour * 60 ) + timeStructNow.tm_min );    
   clockReadAtBell.newHour = ( (timeStructNow.tm_hour != timeStructWas.tm_hour) ? true : false );
   clockReadAtBell.newDay = ( (timeStructNow.tm_wday != timeStructWas.tm_wday) ? true : false );
   timeStructWas = timeStructNow;
 
   return EGuiReply::OKAY_allDone;
}


EGuiReply CClockPerPort::RingTaskBell( void )  {

   return p_Agent->HearTaskBell( clockReadAtBell ); 
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CClockPerHostCpu implementation


CClockPerHost::CClockPerHost(   int arg ) 
                                    :  AClock( arg ),
                                       bellNow_mono ( std::chrono::steady_clock::now() ),
                                       bellNext_mono ( std::chrono::steady_clock::now() ),
                                       bellAdvance (std::chrono::seconds(arg)),
                                       bellNow_wall ( ChronoWall_t::to_time_t(ChronoWall_t::now()) ),
                                       ticking (false) {
// empty ctor
} 


CClockPerHost::~CClockPerHost( void ) { /* empty d-tor*/ } 


void CClockPerHost::TickTock( void ) {

   bellNext_mono = ChronoMono_t::now();

   bellNow_wall = ChronoWall_t::to_time_t( ChronoWall_t::now() );

   clockReadAtBell.timestamp =  bellNow_wall;

   if ( p_Agent != nullptr) {
      p_Agent->HearTaskBell( clockReadAtBell );
   } 

   return;
}


void CClockPerHost::StartTicking( void ) {

   if ( ! ticking ) {

      ticking = true;
      
      //std::thread Thread_S( [&] () -> void { TickTock(); } );

      TickTock();

      //Thread_S.join();

   }
   return;
}


void CClockPerHost::StopTicking( void ) { if ( ticking ) { ticking = false; } return; }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
