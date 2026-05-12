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
   Declaration of CSequence (distributes trigger command to all ISeqElement objects in the
   Application) and CSeqTimeAxis class (provides time axis to time-series plots on GUI)
   There is no "agentTask.cpp" file.  These classes are implemented in "sequence.cpp"    
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef AGENTTASK_HPP
#define AGENTTASK_HPP

#include "customTypes.hpp"
#include <memory>

// Forward declarations
class AChart;
class ADataChannel;
class AFact;
class CProcess;

class CAgent;
class CClockPerPort;
class CFormula; 
class CRuleKit;
class CSeqTimeAxis;

class ISeqElement;


const size_t NUMCLASSESINTRIGGERLOOP = 5;
// 1-channels ("points"), 2-formulas, 3-charts, 4-processes, 5-facts
// (rules trigger later, outside of "loop")

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Only one "agent task" currently defined -> a real-time "Sequence" of data acquistion, processing, and
   analysis
*/

class CSequence { 

   public:
   // Handles

      const CClockPerPort&                   SeqClockRef;
      std::unique_ptr<CSeqTimeAxis>          u_TimeAxis;
   
   // Methods

     CSequence(  CAgent&,
                 CClockPerPort& );

     ~CSequence( void );

     EGuiReply          Trigger( const SClockRead& );    // See Class Note [1]
     int                SayTriggerPeriodSecs( void );

     void              Register( ADataChannel* );
     void              Register( CFormula* );
     void              Register( AChart* );
     void              Register( CProcess* );
     void              Register( AFact* );
     void              Register( CRuleKit* ); 

   private:

   // Fields
      std::vector<int>        numObjectsOfClass;   // int vs. size_t since called by STL accumulator
      // Class-specific indicies for objectsTriggered: Points=0, Formula=1, Chart=2, Process =3, Fact=4
      std::vector<int>        objectsTriggered;
      std::vector<Nzint_t>    baseTriggerGrp;
      std::vector<bool>       allTriggered;
      int                     totalObjects;
      time_t                  timeByHost;
      Nzint_t                 groupTargeted;
      Nzint_t                 lapIncrement; // goes ++ when full lap of while-loop leaves objects untriggered
      bool                    allObjectsUpdated;
      bool                    configured;

      std::vector<ISeqElement*>              p_Points;
      std::vector<ISeqElement*>              p_Formulas;
      std::vector<ISeqElement*>              p_Charts;
      std::vector<ISeqElement*>              p_Processes;
      std::vector<ISeqElement*>              p_Facts;
      std::vector<CRuleKit*>                 p_RuleKits;

   // Methods

      void                                   Configure( void );

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Triggering sends out basic info from clock so ISeqElement objects downstream do not need to run
      getters back to AClock just to get basic time-of-day info (i.e., saves CPU cycles).  ISeqElement
      subclasses needing more than basic info will hold a const ref back to the AClock object.
 
^^^^ END CLASS NOTES */


};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CSeqTimeAxis class is link between AClock and a time series of multiple triggers by a CSequence
object of many objects of ISeqElement concrete subclasses.  One CSequence object may own multiple
CSeqTimeAxis so to time-stamp cycles of ISeqElements working at different "triggers per cycle" rates

CSeqTimeAxis is then known (but not owned) and utilized by CView object of each ISubject to provide
time axis for realtime Kronos.  Both CView and CSeqTimeAxis have same indef lifetime so raw ptr is used
*/

class CSeqTimeAxis {

   public:

   // Methods
      CSeqTimeAxis(  CSequence&,
                     int );

      ~CSeqTimeAxis( void );

      void                          Trigger( time_t );

      KronoTimeStampsOldToNew_t     DisplayAxisFromSnapshotSet( Nzint_t ) const;
      KronoTimeStampsOldToNew_t     DisplayAxisInRealtime( void ) const;
      time_t                        SayTimeNewest( void ) const;
      int                           SaySecsPerCycle( void ) const;
      EGuiReply                     ResizeLoggingToAtLeastSecsAgo( int );
      void                          CreateSnapshotForSetSgi( Nzint_t );
      void                          DestroySnapshotForSetSgi( Nzint_t );

   private:

   // Fields
      SnapshotTimeAxesBank_t        snapshots_bySetSgi;
      std::deque<time_t>            timesHeld_newestToOldest;
      const std::string             caption;
      const int                     secsPerCycle;
      int                           secsLogging;
      bool                          firstCall;
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
