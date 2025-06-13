// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements abstract base class AFact and the non-templated concrete classes for Fact objects. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "state.hpp"

#include "rainfall.hpp"       // call methods on rainfall
#include "subject.hpp"        // call getters on subject
#include "dataChannel.hpp"    // call getters on binary point
#include "taskClock.hpp"
#include "mvc_ctrlr.hpp"

#include <algorithm>

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// AFact (interface) implementations

AFact::AFact(  CSequence& bArg0,
               ASubject& bArg1,
               EDataLabel bArg2 )
               :  ISeqElement(   bArg0,
                                 bArg1,
                                 EApiType::Fact,
                                 bArg2,
                                 EDataUnit::Bindex_fact,
                                 EDataRange::Bindex_fact,
                                 EDataSuffix::None,
                                 EPlotGroup::Free,
                                 1,                       // All AFact subclasses work at tpc = 1
                                 BASETRIGGRP_FACT
                  ),
                  u_Rain ( std::make_unique<CRainFact>(  *this,
                                                         knobKeys_ownedAndAntecedent,
                                                         bArg1
                           )
                  ),
                  u_RealtimeTrace ( std::make_unique<CTraceRealtime>(
                                       bArg1.SayViewRef(),
                                       *u_Rain,
                                       knobKeys_ownedAndAntecedent
                                    )
                  ),
                  p_Operands(0),
                  numOperands (0),
                  spillage (0.0f),
                  timeOfClaimNow (0),
                  firstCycle (true),
                  claimNow (false),   // deliberately not NaNBOOL so can backfill rainfalls on startup
                  claimWas (NaNBOOL),
                  claimHasFlipped (NaNBOOL) {

   // File Note [1] re: why Register() by base vs. by subclass                                                  
   bArg0.Register( this );
}


AFact::AFact(  CSequence& bArg0,
               ASubject& bArg1,
               EDataLabel bArg2,
               std::vector<AFact*> arg0 )
               :  ISeqElement(   bArg0,
                                 bArg1,
                                 EApiType::Fact,
                                 bArg2,
                                 EDataUnit::Bindex_fact,
                                 EDataRange::Bindex_fact,
                                 EDataSuffix::None,
                                 EPlotGroup::Free,
                                 1,                       // All AFact subclasses work at tpc = 1
                                 BASETRIGGRP_FACT
                  ),
                  u_Rain ( std::make_unique<CRainFact>(  *this,
                                                         knobKeys_ownedAndAntecedent,
                                                         bArg1
                           )
                  ),
                  u_RealtimeTrace ( std::make_unique<CTraceRealtime>(
                                       bArg1.SayViewRef(),
                                       *u_Rain,
                                       knobKeys_ownedAndAntecedent
                                    )
                  ),
                  p_Operands(arg0),
                  numOperands ( arg0.size() ),
                  spillage (0.0f),
                  timeOfClaimNow (0),
                  firstCycle (true),
                  claimNow (false),   // deliberately not NaNBOOL so can backfill rainfalls on startup
                  claimWas (NaNBOOL),
                  claimHasFlipped (NaNBOOL) {

   // File Note [1] re: why Register() by base vs. by subclass                                                  
   bArg0.Register( this );
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public Methods

float AFact::SaySpillage( void ) const { return spillage; }

time_t AFact::SayTimeOfClaimNow( void ) const { return timeOfClaimNow; }

bool AFact::Now( void ) const {return claimNow;}

bool AFact::HasClaimFlipped( void ) const { return claimHasFlipped; }


void AFact::LendRealtimeAccessTo( RtTraceAccessTable_t& ptrTableRef ) const {

   // C++11 allows for direct c-tor calls on std::pair<>() vs. calling std::make_pair

   std::pair<RtTraceAccessTable_t::iterator, bool> tableVerbReply =
      ptrTableRef.emplace(
         std::pair< NGuiKey, CTraceRealtime*>(
            u_RealtimeTrace->SayGuiKey(),
            u_RealtimeTrace.get()
         )
      );

   // NOP from the default (ISeqElement virtual defn) can be overridden here by derived classes :
   LendRealtimeAnalogAccessTo( ptrTableRef );

   if (numOperands > 0) {    // Forward call to input Facts (operands) iff this object has any

      for ( auto ptr : p_Operands ) {

         ptr->LendRealtimeAccessTo( ptrTableRef );
      }
   }
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CFactFromFacts

CFactFromFacts::CFactFromFacts(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 EDataLabel bArg2,
                                 std::vector<AFact*> bArg3,
                                 std::function<bool(void)> arg1 )
                                 :  AFact(   bArg0,
                                             bArg1,
                                             bArg2,
                                             bArg3
                                    ),
                                    Statement (arg1) {

   CalcOwnTriggerGroup();
   ConfigureCycling();
   LendKnobKeysTo( knobKeys_ownedAndAntecedent );
}
 
CFactFromFacts::~CFactFromFacts( void ) { /* empty */ }

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Private methods


void CFactFromFacts::CalcOwnTriggerGroup( void ) {

   std::vector<Nzint_t> inputTriggerGroups(0);
   for ( auto ptr : p_Operands ) {
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

//======================================================================================================/

void CFactFromFacts::ConfigureCycling( void ) {

   // Determine which operand has lowest triggers/cycle (highest freq.) and use it for this Fact object

   std::vector<int> stackVec(0);

   for ( std::vector<AFact*>::const_iterator iter = p_Operands.begin();
         iter != p_Operands.end();
         ++iter ) {

            stackVec.push_back( (*iter)->SayTriggersPerCycle() );
   }
   
   triggersPerCycle = *std::min_element( stackVec.begin(), stackVec.end() );
   triggersUntilCycle = triggersPerCycle;
   secsPerCycle = triggersPerCycle * SeqRef.SayTriggerPeriodSecs();
   
   return;
}

//======================================================================================================/

bool CFactFromFacts::PullValidity( void ) {

   bool reply = p_Operands.at(0)->IsValid();                // Get Validity() of 1st operand...

   for (int i=1; i<numOperands; ++i) {
      reply = ( reply && p_Operands.at(i)->IsValid() );    // ...then enjoin all other operands
   }
   return reply;
}

//======================================================================================================/

void CFactFromFacts::Cycle( time_t timestampNow ) {

   validWas = validNow;
   validNow = PullValidity();

   if ( validNow ) { 

      claimWas = claimNow;

      claimNow = Statement();

      claimHasFlipped = (claimNow != claimWas);

      timeOfClaimNow = (   ( claimHasFlipped || (validWas != validNow) || firstCycle ) ?
                              timestampNow :
                              timeOfClaimNow );
   }   
   /* Else, log last valid value throughout invalid periods
      Relies on valueNow in AFact c-tor not being initialized as NaNBOOL
   */
   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  claimNow,
                  validNow
   );

   firstCycle = false;
   return;
}

//======================================================================================================/

void CFactFromFacts::LendAntecedentKnobKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   for ( auto ptr : p_Operands ) { ptr->LendKnobKeysTo( borrowerRef ); }

   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods

void CFactFromFacts::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

/* Note what method must do here:  e.g., a chart has no own rain or own histogram, but a chart's
   (analog) input object has both, and the Fact(s) a chart feeds into has both.  Lending keys out so
   to be delivered to GUI in a GuiPack must accommodate that.
   not workable to have u_Rain in abstract base class ARainfall
*/

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
   for ( auto ptr : p_Operands ) { ptr->LendHistogramKeysTo( borrowerRef ); }
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CStateSched 

CFactFromClock::CFactFromClock(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 EDataLabel bArg2 )
                                 :  AFact(  bArg0,
                                             bArg1,
                                             bArg2,
                                             std::vector<AFact*>(0)
                                    ),
                                    Condition ( [] ()->bool {return false; } ),
                                    ClockRef (bArg0.SeqClockRef) {

   CalcOwnTriggerGroup();
}
 
CFactFromClock::~CFactFromClock( void ) { /* empty */ }


void CFactFromClock::SetCondition( std::function<bool(void)> arg) { // for run-time reset of Condition 

   Condition = arg;

   return;
}

int CFactFromClock::Hour( void ) const { return ClockRef.GetClockHour(); }


int CFactFromClock::Day( void ) const { return ClockRef.GetClockWday(); }


void CFactFromClock::CalcOwnTriggerGroup( void ) {
    ownTriggerGroup = baseTriggerGroup;
   return;
}


void CFactFromClock::Cycle( time_t timestampNow ) {

// Subclass not currently using validity (clock info presumed fault-free)

   claimWas = claimNow;

   claimNow = Condition();

   claimHasFlipped = (claimNow != claimWas);

   timeOfClaimNow = (   ( claimHasFlipped || firstCycle ) ?
                           timestampNow :
                           timeOfClaimNow );

   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  claimNow,
                  validNow

   );

   firstCycle = false;
   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CFactFromAntecedentSubject 

CFactFromAntecedentSubject::CFactFromAntecedentSubject(  CSequence& bArg0,
                                                         ASubject& bArg1,
                                                         EDataLabel bArg2,
                                                         CDomain& arg0,
                                                         ERealName arg1 )
                                                         :  AFact(   bArg0,
                                                                     bArg1,
                                                                     bArg2,
                                                                     std::vector<AFact*>(0) 
                                                            ),
                                                            p_AnteSubj ( arg0.SayPtrToSubjectNamed(arg1) ),
                                                            anteSubjName (arg1) {

   if (p_AnteSubj == nullptr) {
      throw std::logic_error( "Fact constructed before its antecedent Subject" );
   }
   CalcOwnTriggerGroup();
}
 
CFactFromAntecedentSubject::~CFactFromAntecedentSubject( void ) { /* empty */ }


void CFactFromAntecedentSubject::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

/* Note what method must do here:  e.g., a chart has no own rain or own histogram, but a chart's
   (analog) input object has both, and the Fact(s) a chart feeds into has both.  Lending keys out so
   to be delivered to GUI in a GuiPack must accommodate that.
   not workable to have u_Rain in ARainfall
*/

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


void CFactFromAntecedentSubject::Cycle(  time_t timestampNow ) {

   claimWas = claimNow;

   claimNow = p_AnteSubj->IsUnitOutputOkay();

   claimHasFlipped = (claimNow != claimWas);

   timeOfClaimNow = (   ( claimHasFlipped || firstCycle ) ?
                           timestampNow :
                           timeOfClaimNow );

   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  claimNow,
                  validNow
 
   );

   firstCycle = false;
   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CFactFromPoint 

CFactFromPoint::CFactFromPoint(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 EDataLabel bArg2,
                                 const CPointBinary& arg)
                                 :  AFact(   bArg0,
                                             bArg1,
                                             bArg2,
                                             std::vector<AFact*>(0) 
                                    ),
                                    PointRef (arg),
                                    binaryPostedByPoint (NaNBOOL) {

   CalcOwnTriggerGroup();
}
 
CFactFromPoint::~CFactFromPoint( void ) { /* empty */ }


void CFactFromPoint::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

/* Note what method must do here:  e.g., a chart has no own rain or own histogram, but a chart's
   (analog) input object has both, and the Fact(s) a chart feeds into has both.  Lending keys out so
   to be delivered to GUI in a GuiPack must accommodate that.
   not workable to have u_Rain in IRainfall
*/

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


void CFactFromPoint::Cycle(  time_t timestampNow ) {

   validWas = validNow;
   claimWas = claimNow;

   validNow = PointRef.IsValid();

   binaryPostedByPoint = PointRef.SayBinaryPosted();
   claimNow = ( validNow ? binaryPostedByPoint : claimWas );

   claimHasFlipped = (claimNow != claimWas);

   timeOfClaimNow = (   ( claimHasFlipped || (validWas != validNow) || firstCycle ) ?
                           timestampNow :
                           timeOfClaimNow );

   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  claimNow,
                  validNow
 
   );

   firstCycle = false;
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CFactFromFacts

CFactSustained::CFactSustained(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 EDataLabel bArg2,
                                 const AFact& arg0,
                                 const ESustainedAs arg1,
                                 std::array<int,3> arg2,
                                 CController& arg3 )
                                 :  AFact( bArg0,
                                           bArg1,
                                           bArg2,
                                           std::vector<AFact*>(0)
                                    ),
                                    FactToWatchRef (arg0),
                                    watchMode (arg1),
                                    minCyclesToSustain (arg2[1]),
                                    cyclesSustained (0),
                                    claimToWatch (NaNBOOL) {

CalcOwnTriggerGroup();   
AttachOwnKnobs( arg3, arg2 );
}
 
CFactSustained::~CFactSustained( void ) { /* empty */ }

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Private methods

void CFactSustained::AttachOwnKnobs( CController& ctrlrRef, std::array<int,3> rangeStops ) {

  /* AKnob subclasses test GUI input to range allowed prior to calling setter lambdas, so value
     given by User either gets ignored with O.O.R. error sent, or arrives at lambda arg as "vetted" */

   u_KnobsOwned.push_back( std::make_unique<CKnobSint>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_factSustained_minCycles,
                              EDataUnit::Count_cycle,
                              EDataSuffix::None,
                              (  [&]( int userInputVetted ) ->void {
                                    minCyclesToSustain = userInputVetted;
                                    return; }
                              ),
                              rangeStops,
                              minCyclesToSustain
                        )
   );
   return;
}

void CFactSustained::CalcOwnTriggerGroup( void ) {

   ownTriggerGroup = FactToWatchRef.SayOwnTriggerGroup() + 1u;
   return;
}


void CFactSustained::Cycle( time_t timestampNow ) {

   if ( firstCycle ) { 

      claimToWatch = ( ( watchMode == ESustainedAs::TrueXorFalse ) ?
         FactToWatchRef.Now() :
         ( (watchMode == ESustainedAs::True) ? true : false )
      );
      firstCycle = false;
   }

   validWas = validNow;  // ISeqElement inits to TRUE
   validNow = FactToWatchRef.IsValid();

   claimWas = claimNow;  // AFact init sets claimWas, claimToWatch to NaNBOOL; claimNow to FALSE

   if ( validNow ) {

      // once cyclesSustained reaches min threshold, keep it there until == test is failed
      cyclesSustained =
         ( ( claimToWatch == FactToWatchRef.Now() ) ?
           std::min( minCyclesToSustain, (cyclesSustained + 1) ) : 0 );

      claimNow = ( cyclesSustained > (minCyclesToSustain - 1) ); 

      claimHasFlipped = (claimNow != claimWas);

      if ( claimHasFlipped && ( watchMode == ESustainedAs::TrueXorFalse ) ) {
         claimToWatch = FactToWatchRef.Now();
      }  

      timeOfClaimNow = (   ( claimHasFlipped || (validWas != validNow) || firstCycle ) ?
                              timestampNow :
                              timeOfClaimNow );
   }  // close IF on validNow

   /* Log most recent claim, including invalid periods.
      Relies on claimNow in AFact c-tor not being initialized as NaNBOOL
   */
   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  claimNow,
                  validNow
   );

   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods

void CFactSustained::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

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

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   'this' pointer of a non-template ABC must be used to register objects of templated subclass,
      otherwise the template param has to be supplied, making impossible one vector holding handles to
      all AFact objects.

XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
