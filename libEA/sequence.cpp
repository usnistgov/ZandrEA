// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implementation of CSequence (distributes trigger command to all ISeqElement objects in the
   Application) and CSeqTimeAxis class (provides time axis to time-series plots on GUI)
   There is no "sequence.hpp" file.  These classes are decalred in "agentTask.hpp"   
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "agentTask.hpp"   // A sequence is a form of agent task, so there is header of these classes
#include <vector>

// Fully declare inheritance of Trigger() method by all subclasses (so can call it on subclass ptrs)
// (Done here vs. in header, so to reduce chance of circulating header #includes)
#include "mvc_model.hpp"
#include "taskClock.hpp" 
#include "dataChannel.hpp"
#include "rainfall.hpp"
#include "formula.hpp"
#include "chart.hpp"
#include "state.hpp"
#include "rule.hpp"
#include "viewParts.hpp"   // needed for CSeqTimeAxis length

#include <numeric>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CSequence Implementation

CSequence::CSequence(   CAgent& bArg0,
                        CClockPerPort& bArg1 )
                        :  SeqClockRef (bArg1),
                           u_TimeAxis ( std::make_unique<CSeqTimeAxis>( *this,
                                                                        1 )
                           ),
                           numObjectsOfClass(NUMCLASSESINTRIGGERLOOP,0),
                           objectsTriggered(NUMCLASSESINTRIGGERLOOP,0),
                           baseTriggerGrp(NUMCLASSESINTRIGGERLOOP,0),
                           allTriggered(NUMCLASSESINTRIGGERLOOP,false),
                           totalObjects (0),
                           timeByHost (0),
                           groupTargeted (0),
                           lapIncrement (0),
                           allObjectsUpdated (false),
                           configured (false),
                           p_Points(0),
                           p_Formulae(0),
                           p_Charts(0),
                           p_Facts(0),
                           p_RuleKits(0) {

   bArg0.AddAsTask( this );
}

CSequence::~CSequence( void ) {

   // Empty d-tor
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV

void CSequence::Register( ADataChannel* arg )  {

   p_Points.push_back( arg );
 
   return;
}


void CSequence::Register( CFormula* arg )  {

   p_Formulae.push_back( arg );

   return;
}

void CSequence::Register( AChart* arg )  {

   p_Charts.push_back( arg );

   return;
}

void CSequence::Register( AFact* arg )  {

   p_Facts.push_back( arg );

   return;
}

void CSequence::Register( CRuleKit* arg )  {

   p_RuleKits.push_back( arg );

   return;
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV

void CSequence::Configure( void ) {

   /*
   Need tally of number of objects to trigger (i.e., registered) in each class (object type)
   Does NOT include CRuleKit objects, as rule kit(s) are triggered only after completing all
   iterations of sequentially triggering the other objects (as performed in Cycle() by repeated laps
   around a while-loop).
   */
   numObjectsOfClass[0] = static_cast<int>( p_Points.size() ); 
   numObjectsOfClass[1] = static_cast<int>( p_Formulae.size() );
   numObjectsOfClass[2] = static_cast<int>( p_Charts.size() );
   numObjectsOfClass[3] = static_cast<int>( p_Facts.size() );

   totalObjects = static_cast<int>(
      std::accumulate( numObjectsOfClass.begin(), numObjectsOfClass.end(), 0 )
   );   

   baseTriggerGrp[0] = BASETRIGGRP_POINT;
   baseTriggerGrp[1] = BASETRIGGRP_FORMULA;
   baseTriggerGrp[2] = BASETRIGGRP_CHART;
   baseTriggerGrp[3] = BASETRIGGRP_FACT;

   configured = true;
   return;
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
  
EGuiReply CSequence::Trigger( const SClockRead& clockInfo )  {
 
   if ( !configured ) { Configure(); }

   u_TimeAxis->Trigger( clockInfo.timestamp );

   allTriggered.assign(allTriggered.size(), false);
   objectsTriggered.assign(objectsTriggered.size(), 0);
   groupTargeted = 0;
   lapIncrement = 0;
   allObjectsUpdated = false;  // See Method Note [1]

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
// Triggering routine for all ISeqElement objects registered to this CSequence object

   while ( ! allObjectsUpdated )  {                   // "all" except rule kits and rules

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
          if ( allTriggered[0] == false ) {           // Test for untriggered ADataPoint objects
            groupTargeted = baseTriggerGrp[0] + lapIncrement;

            for ( auto ptr : p_Points ) { 

                //Trigger() rtns 1 only if object's trigger grp matches the group being targeted

               objectsTriggered[0] =
                        objectsTriggered[0] + ( ptr->Trigger( groupTargeted, clockInfo ) );
            }
            if ( objectsTriggered[0] == numObjectsOfClass[0] ) { allTriggered[0] = true; }
         }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
         if ( allTriggered[1] == false ) {      // Test for untriggered CFormula objects
            groupTargeted = baseTriggerGrp[1] + lapIncrement;

            for ( auto ptr : p_Formulae ) { 

               objectsTriggered[1] =
                        objectsTriggered[1] + ( ptr->Trigger( groupTargeted, clockInfo ) );
            }
            if ( objectsTriggered[1] == numObjectsOfClass[1]) { allTriggered[1] = true; }
         }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
         if ( allTriggered[2] == false ) {      // Test for untriggered AChart objects
            groupTargeted = baseTriggerGrp[2] + lapIncrement;

            for ( auto ptr : p_Charts ) { 

               objectsTriggered[2] =
                        objectsTriggered[2] + ( ptr->Trigger( groupTargeted, clockInfo ) );
            }

            if ( objectsTriggered[2] == numObjectsOfClass[2]) { allTriggered[2] = true; }
         }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
         if ( allTriggered[3] == false ) {      // Test for untriggered AFact objects
            groupTargeted = baseTriggerGrp[3] + lapIncrement;

            for ( auto ptr : p_Facts ) { 

               objectsTriggered[3] =
                        objectsTriggered[3] + ( ptr->Trigger( groupTargeted, clockInfo ) );
            }

            if ( objectsTriggered[3] == numObjectsOfClass[3]) { allTriggered[3] = true; }
         }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Update while-run condition: Have all objects been triggered (i.e., updated) ?

      if ( std::accumulate( objectsTriggered.begin(), objectsTriggered.end(), 0 ) == totalObjects ) {
         allObjectsUpdated = true;
         break;
      }

      // If not true, increment the trigger group targeted, and make another lap of 'while'
      ++lapIncrement;
   }     // closes while loop

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
// Trigger rule kit(s) registered to this CSequence

   for ( std::vector<CRuleKit*>::iterator iter = p_RuleKits.begin();
         iter != p_RuleKits.end();
         ++iter ) {

            (*iter)->Trigger( BASETRIGGRP_RULEKIT, clockInfo );
   }

   return ( allObjectsUpdated ?
               EGuiReply::OKAY_allDone :
               EGuiReply::WARN_ranSeqToExitWithObjectsYetToCycle_fixApi );

/* START METHOD NOTES ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/

[1]   Currently, all other registered ISeqElement objects are triggered before rule kit(s) receive
      triggering.  That may have to change if some rules are to be at higher frequency than others.

[2]   Current design is that AFact class gets timestamps through SayClockRef() call to CSequence&
      c-tor argument, and hold that ref to stamp their value changes.  Time is not sent out from
      CSequence object because its triggering calls already send out trigger group, and most objects
      downstream simply do not need timestamps at all, or only need one occasionally.

--------------------------------------------------------------------------------
XXX END METHOD NOTES */

}



int CSequence::SayTriggerPeriodSecs( void )  {  return SeqClockRef.SayBellPeriodSecs(); }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of CSeqTimeAxis

CSeqTimeAxis::CSeqTimeAxis(   CSequence& arg0,
                              int arg1 )
                              :  snapshots_bySetSgi(),
                                 timesHeld_newestToOldest(
                                    ( START_DATALOG_SECSLOGGING / (arg0.SayTriggerPeriodSecs() * arg1) ),
                                    0
                                 ),
                                 caption ("Local date/time"),
                                 secsPerCycle (arg0.SayTriggerPeriodSecs() * arg1 ),
                                 secsLogging (START_DATALOG_SECSLOGGING),
                                 firstCall (true) {

}


CSeqTimeAxis::~CSeqTimeAxis(  void  ) { /* Empty d-tor */ }

//=====================================================================================================/
// Private methods

   // Since triggers/cycle of a CSeqTimeAxis is always = 1, no Cycle() method is needed here 

//=====================================================================================================/
// Public methods

void CSeqTimeAxis::Trigger( time_t timeFromClock ) {

   // Most recent value at lowest index ("front"); oldest value at highest index ("back")

   if (firstCall) {

      time_t timeToStamp = timeFromClock;
      auto iter = timesHeld_newestToOldest.begin();
      while ( iter != timesHeld_newestToOldest.end() ) {
         *iter++ = timeToStamp;
         timeToStamp -= secsPerCycle;     // presumes POSIX-compliant implementation of time_t
      }
      firstCall = false;
      return;
   }

   timesHeld_newestToOldest.pop_back();
   timesHeld_newestToOldest.push_front(  timeFromClock   );

   return;
}


KronoTimeStampsOldToNew_t CSeqTimeAxis::DisplayAxisFromSnapshotSet( Nzint_t setSgi ) const {

   return  KronoTimeStampsOldToNew_t(  snapshots_bySetSgi.at( setSgi ).rbegin(),
                                       snapshots_bySetSgi.at( setSgi ).rend() );
}


KronoTimeStampsOldToNew_t CSeqTimeAxis::DisplayAxisInRealtime( void ) const {

   /*
   $$$ meth presumes tpc=1 on all ISeqObjects and all traces (R-T and S-S) having same fixed length
   TBD to provide more flexibility $$$
   */  

   return ( KronoTimeStampsOldToNew_t(
               ( timesHeld_newestToOldest.rend() - FIXED_KRONO_SNAPSHOT_SIZE ),
               timesHeld_newestToOldest.rend()
            )
   );
}


void CSeqTimeAxis::CreateSnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   SnapshotTimeAxis_t timeAxisSnapshot;

   for ( size_t iLog = 0; iLog < FIXED_KRONO_SNAPSHOT_SIZE; ++iLog ) {
         timeAxisSnapshot[iLog] = timesHeld_newestToOldest[iLog];
   }
   snapshots_bySetSgi.emplace(
      std::pair<Nzint_t, SnapshotTimeAxis_t>( snapshotSetSgi, timeAxisSnapshot ) );
   return;
}


void CSeqTimeAxis::DestroySnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   snapshots_bySetSgi.erase( snapshotSetSgi );
   return;
}   


time_t CSeqTimeAxis::SayTimeNewest( void ) const { return timesHeld_newestToOldest[0]; }


int CSeqTimeAxis::SaySecsPerCycle( void ) const { return secsPerCycle; }



EGuiReply CSeqTimeAxis::ResizeLoggingToAtLeastSecsAgo( int secsNeeded ) {

   if ( secsNeeded < 0 || secsNeeded > (FIXED_DATALOG_SIZECYCLES_MAX * secsPerCycle) ) {
      return EGuiReply::FAIL_set_givenDataLoggingSizeNotWithinBounds;
   }
   if ( !( secsNeeded > secsLogging ) ) { return EGuiReply::OKAY_allDone; }

   // C++ int divide rounds to floor. To round to ceiling when no negative arg or overflow possible:
   size_t newSize = static_cast<size_t>( (secsNeeded + secsPerCycle - 1) / secsPerCycle );

   timesHeld_newestToOldest.resize( newSize, timesHeld_newestToOldest.back() );
   secsLogging = timesHeld_newestToOldest.size() * secsPerCycle;
/*
   Method returns EGuiReply so it can work with realtime construction of kronos.  But API does not
   have that capability yet, so caller dumps "bad" reply into a throw, as only possible error source
   currently is instantiation script (i.e., a program "logic" error).
*/
   return EGuiReply::OKAY_allDone;
}


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
