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
   Declares the concrete class for MVC Agent objects, class CAgent.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

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
