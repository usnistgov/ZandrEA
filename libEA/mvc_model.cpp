// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements the concrete class for MVC Agent objects, class CAgent.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "mvc_model.hpp"
#include "taskClock.hpp"
#include "agentTask.hpp"


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CAgent implementations

CAgent::CAgent( CClockPerPort& clockRef )
                  :  p_Task (nullptr) {

   clockRef.RegisterForBells( this );
}


CAgent::~CAgent( void ) {

   // Empty interface d-tor
}


EGuiReply CAgent::HearTaskBell( const SClockRead& clockReadAtBell ) {

   return p_Task->Trigger( clockReadAtBell );      // skip ptr check; here want a crash if miscoded
} 


void CAgent::AddAsTask( CSequence* arg ) { p_Task = arg; return; }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
