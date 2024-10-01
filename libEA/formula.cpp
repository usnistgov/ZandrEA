// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements CForumla class to create objects doing calculations using sampled or parametric values
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "formula.hpp"
#include "agentTask.hpp"      // register to sequence
#include "dataChannel.hpp"
#include "rainfall.hpp"
#include "viewParts.hpp"      // get NGuiKey of traces
#include "subject.hpp"        // call getters on subject

#include <limits>
#include <algorithm>          // min_element()
#include <cmath>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CFormula

// constructors

CFormula::CFormula(  CSequence& bArg0,
                     ASubject& bArg1,
                     EDataLabel bArg2,
                     EDataUnit bArg3,
                     EDataRange bArg4,
                     EDataSuffix bArg5,
                     EPlotGroup bArg6,
                     std::vector<CPointAnalog*> arg1,
                     std::function<float(void)> arg2 )
                     :  ISeqElement(   bArg0,
                                       bArg1,
                                       EApiType::Formula,
                                       bArg2,
                                       bArg3,
                                       bArg4,
                                       bArg5,
                                       bArg6,
                                       1,        // could be min(tpc of inputs), but =1 is simpler
                                       BASETRIGGRP_FORMULA
                        ),
                        u_Rain(  std::make_unique<CRainAnalog>(
                                    *this,
                                    knobKeys_ownedAndAntecedent,
                                    bArg1,
                                    bArg4
                                 )
                        ),
                        u_RealtimeTrace ( std::make_unique<CTraceRealtime>(
                                             bArg1.SayViewRef(),
                                             *u_Rain,
                                             knobKeys_ownedAndAntecedent
                                          )
                        ),
                        p_Operands (arg1),
                        Calculate (arg2),
                        resultNow (NaNFLOAT),
                        resultLastValid (NaNFLOAT) {

   CalcOwnTriggerGroup();                                              
   bArg0.Register(this);
   ConfigureCycling();

}
 
CFormula::~CFormula( void ) { }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFormula methods - private

void CFormula::CalcOwnTriggerGroup( void ) {

   std::vector<Nzint_t> inputTriggerGroups(0);
   for ( auto& ptr : p_Operands ) {
      inputTriggerGroups.push_back( ptr->SayOwnTriggerGroup() );
   }
   Nzint_t latestInputTriggerGroup =
      *std::max_element( inputTriggerGroups.begin(), inputTriggerGroups.end(), std::less<Nzint_t>() );
   
   if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
      ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
   }
   else { ownTriggerGroup = baseTriggerGroup; }
   return;
}


void CFormula::ConfigureCycling( void ) {

   std::vector<int> stackVec(0);

   for ( std::vector<CPointAnalog*>::const_iterator iter = p_Operands.begin();
         iter != p_Operands.end();
         ++iter ) {

            stackVec.push_back( (*iter)->SayTriggersPerCycle() );
   }
   
   // object to cycle at min triggers/cycle of all upstream objects sending input to it
   triggersPerCycle = *std::min_element( stackVec.begin(), stackVec.end() );
   if (triggersPerCycle == 1 ) { cyclingAtMaxRate = true; }
   triggersUntilCycle = triggersPerCycle;
   secsPerCycle = triggersPerCycle * SeqRef.SayTriggerPeriodSecs();

   return;
}

bool CFormula::PullValidity( void ) {

   bool reply = p_Operands.at(0)->IsValid();   // Get validity of 1st operand...

   for (size_t i=1; i<p_Operands.size(); ++i) {          // ...then enjoin any/all other operands
      reply = ( reply && (p_Operands.at(i)->IsValid() ) );
   }
   return reply;  // Return only, as this method alone cannot set "valid" on the CFormula object
}


void CFormula::Cycle( time_t timestampNow ) {

   validNow = PullValidity();    // reset own object validity up to point of calling calulation

   if ( validNow ) {

      resultNow = Calculate();

   if ( ! (std::isfinite( resultNow )) ) {
         validNow = false;
         resultNow = resultLastValid;
      }
   } 
   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  resultNow,
                  validNow
   );
   resultLastValid = ( validNow ? resultNow : resultLastValid );

   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFormula methods - public

void CFormula::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   NGuiKey ownHistogramKey = u_Rain->SayHistogramKey();

   bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                ownHistogramKey
                                             ) == borrowerRef.end() ?
                                                true :
                                                false
   ); 
   if ( borrowerNotAlreadyHoldingSameKey ) {

      borrowerRef.push_back( ownHistogramKey );
   }
   return;
}


void CFormula::LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTableRef ) const {

   // C++11 allows for direct c-tor calls on std::pair<>() vs. calling std::make_pair

   std::pair<RtTraceAccessTable_t::iterator, bool> tableVerbReply =
      ptrTableRef.emplace(
         std::pair< NGuiKey, CTraceRealtime*>(
            u_RealtimeTrace->SayGuiKey(),
            u_RealtimeTrace.get()
         )
      );

   for ( const auto ptr : p_Operands ) {

      ptr->LendRealtimeAnalogAccessTo( ptrTableRef );
   }

   return;
};


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
