// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares the concrete class for MVC Agent objects, class CAgent.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef MVC_MODEL_HPP
#define MVC_MODEL_HPP

#include "customTypes.hpp"

// Forward declares (to avoid unnecessary #includes)
class CClockPerPort;
class CSequence;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Declare the "Agent" (called "Model" in more conventional "MVC" designs), who does "tasks" as called by
   "bell" (from a "Clock" per host, or per Port from front end). Cycling program execution through all
   AFDD surveillance objects (which were instantiated in Application by AFDD "Tools") is such a "task".
*/

class CAgent {

   public:
   // Methods

      explicit CAgent( CClockPerPort& );

      ~CAgent( void );

      EGuiReply       HearTaskBell( const SClockRead& );

      void            AddAsTask( CSequence* );

   private:

   // Handles

      CSequence*    p_Task;
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
