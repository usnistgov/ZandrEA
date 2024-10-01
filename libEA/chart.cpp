// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements abstract base class AChart and concrete classes for all Chart objects.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "chart.hpp"
#include "rainfall.hpp"      // Need type completion due to method calls
#include "agentTask.hpp"     // Need type completion for Register() call in AChart ctor
#include "controlParts.hpp"
#include "dataChannel.hpp"
#include "formula.hpp"

#include <algorithm>
#include <numeric>


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// AChart (abstract) implementations

size_t AChart::FitRainCyclesToSecondsOrRtnNaN( int secsGiven, int secsPerCycle ) {

   return ( ( (secsGiven >= secsPerCycle) && (secsGiven < (FIXED_RAINFALL_SIZE * secsPerCycle)) ) ?
               // C++ int div rounds to floor, so fractions -> 0
               // caller re-multiplies (rounded off) rtn value by spc to get ACTUAL seconds spanned
               static_cast<size_t>( secsGiven/secsPerCycle ) :
               NaNSIZE
   );
}

AChart::AChart(   CSequence& bArg0,
                  ASubject& bArg1,
                  EApiType bArg2,
                  CPointAnalog* const bArg3,
                  CFormula* const bArg4,
                  CPointAnalog* const bArg5,
                  CFormula* const bArg6 )
                  :  ISeqElement(   bArg0,
                                    bArg1,
                                    bArg2,
                                    ( bArg3 ? bArg3->SayLabel() : bArg4->SayLabel() ),
                                    ( bArg3 ? bArg3->SayUnits() : bArg4->SayUnits() ),
                                    ( bArg3 ? bArg3->SayRange() : bArg4->SayRange() ),
                                    ( bArg3 ? bArg3->SaySuffix() : bArg4->SaySuffix() ),
                                    EPlotGroup::Undefined,    // chart itself is not plotted
                                    ( bArg3 ? bArg3->SayTriggersPerCycle() : bArg4->SayTriggersPerCycle() ),
                                    BASETRIGGRP_CHART
                     ),
                     p_ObsvdPoint (bArg3),
                     p_ObsvdFormula (bArg4),
                     p_GuidePoint (bArg5),
                     p_GuideFormula (bArg6),
                     p_ObsvdRain ( bArg3 ? bArg3->u_Rain.get() : bArg4->u_Rain.get() ),
                     p_GuideRain ( bArg5 ? bArg5->u_Rain.get() :
                                           ( bArg6 ? bArg6->u_Rain.get() : nullptr )
                     ),
                     chartRunning (false),
                     resetPending (true) {

   // empty chart interface ctor

}


void AChart::LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTableRef ) const {

   if ( p_ObsvdPoint ) { p_ObsvdPoint->LendRealtimeAnalogAccessTo( ptrTableRef ); }

   if ( p_ObsvdFormula ) { p_ObsvdFormula->LendRealtimeAnalogAccessTo( ptrTableRef ); }

   if ( p_GuidePoint ) { p_GuidePoint->LendRealtimeAnalogAccessTo( ptrTableRef ); }

   if ( p_GuideFormula ) { p_GuideFormula->LendRealtimeAnalogAccessTo( ptrTableRef ); }

   return;
}



/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CChartShewhart charts

CChartShewhart::CChartShewhart(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 CPointAnalog* const bArg2,
                                 CController& arg1 )
                                 : AChart(   bArg0,
                                             bArg1,
                                             EApiType::Chart_shewhart,
                                             bArg2,
                                             nullptr,
                                             nullptr,
                                             nullptr
                                    ),
                                    stdDevRef (NaNFLOAT),
                                    stdDevRefNew (NaNFLOAT),
                                    xMean (NaNFLOAT),
                                    xNow (NaNFLOAT),
                                    xStdDev (NaNFLOAT),
                                    zNow (NaNFLOAT),
                                    zPass (INIT_MINDEFMAX_SHEWHART_ZPASS[1]),
                                    numCyclesBeingUsed (START_SHEWCHART_CYCLESUSING),
                                    rainDepthBeingUsed ( numCyclesBeingUsed - 1u ),
                                    numSecsBeingUsed (   p_ObsvdRain->SourceRef.SaySecsPerCycle() *
                                                               numCyclesBeingUsed
                                    ),
                                    tripFreeCount (0),
                                    tripFreeMargin (INIT_MINDEFMAX_SHEWHART_TRIPFREEMARGIN[1]),
                                    abovePassband (false),
                                    belowPassband (false), 
                                    chartTrip (false),
                                    flipState (false),
                                    isSteadyNow (false),
                                    wasSteadyOnLastValid (false) {

   CalcOwnTriggerGroup();       // See File Note [1] rainfall.h
   bArg0.Register( this );
   p_ObsvdRain->AddNewStatisticsUserSpanningCycles( numCyclesBeingUsed );
   ConfigureCycling();
   AttachOwnKnobs( arg1 );
}


CChartShewhart::CChartShewhart(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 CFormula* bArg2,
                                 CController& arg1 )
                                 : AChart(   bArg0,
                                             bArg1,
                                             EApiType::Chart_shewhart,
                                             nullptr,
                                             bArg2,
                                             nullptr,
                                             nullptr
                                    ),
                                    stdDevRef (NaNFLOAT),
                                    stdDevRefNew (NaNFLOAT),
                                    xMean (NaNFLOAT),
                                    xNow (NaNFLOAT),
                                    xStdDev (NaNFLOAT),
                                    zNow (NaNFLOAT),
                                    zPass (INIT_MINDEFMAX_SHEWHART_ZPASS[1]),
                                    numCyclesBeingUsed (START_SHEWCHART_CYCLESUSING),
                                    rainDepthBeingUsed ( numCyclesBeingUsed - 1u ),
                                    numSecsBeingUsed (   p_ObsvdRain->SourceRef.SaySecsPerCycle() *
                                                               numCyclesBeingUsed
                                    ),
                                    tripFreeCount (0),
                                    tripFreeMargin (INIT_MINDEFMAX_SHEWHART_TRIPFREEMARGIN[1]),
                                    abovePassband (false),
                                    belowPassband (false), 
                                    chartTrip (false),
                                    flipState (false),
                                    isSteadyNow (false),
                                    wasSteadyOnLastValid (false) {

   CalcOwnTriggerGroup();       // See File Note [1] rainfall.h
   bArg0.Register( this );
   p_ObsvdRain->AddNewStatisticsUserSpanningCycles( numCyclesBeingUsed );
   ConfigureCycling();
   AttachOwnKnobs( arg1 );
}

CChartShewhart::~CChartShewhart( void ) {

// Empty destructor body

}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Private methods


void CChartShewhart::CalcOwnTriggerGroup( void ) {

   Nzint_t latestInputTriggerGroup = p_ObsvdRain->SourceRef.SayOwnTriggerGroup();

   if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
      ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
   }
   else { ownTriggerGroup = baseTriggerGroup; }
   return;  
}


void CChartShewhart::ConfigureCycling( void ) {

   triggersPerCycle = p_ObsvdRain->SourceRef.SayTriggersPerCycle();
   triggersUntilCycle = triggersPerCycle;
   secsPerCycle = ( triggersPerCycle * SeqRef.SayTriggerPeriodSecs() );
   return;
}

void CChartShewhart::AttachOwnKnobs( CController& ctrlrRef ) {

  /* AKnob subclasses test GUI input to range allowed prior to calling setter lambdas, so value
     given by User either gets ignored with O.O.R. error sent, or arrives at lambda arg as "vetted" */

   std::array<int,3> minDefMax_secsUsing = { p_ObsvdRain->SourceRef.SaySecsPerCycle(),
                                             numSecsBeingUsed,
                                             static_cast<int>(
                                                (FIXED_RAINFALL_SIZE) *
                                                p_ObsvdRain->SourceRef.SaySecsPerCycle()
                                             ) };


   u_KnobsOwned.push_back( std::make_unique<CKnobSint>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartShew_secsDataUsing,
                              EDataUnit::TimeSpan_sec,
                              EDataSuffix::None,
                              (  [&]( int userInputVetted ) ->void {
                                    SetNumSecsBeingUsedTo( userInputVetted ); // Operand set takes s-int
                                    return; }
                              ),
                              minDefMax_secsUsing,
                              numSecsBeingUsed     // sets what is shown as current value
                        )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartShew_zPass,
                              EDataUnit::Distance_stdDev,
                              EDataSuffix::None,
                              (  [&]( float userInputVetted ) -> void {
                                    zPass = userInputVetted;
                                    return; }
                              ),
                              INIT_MINDEFMAX_SHEWHART_ZPASS,
                              zPass )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobSint>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartShew_tripFreeMargin,
                              EDataUnit::Count_cycle,
                              EDataSuffix::ConsecutiveNoTrip,
                              (  [&]( int userInputVetted ) ->void {
                                    tripFreeMargin = userInputVetted;
                                    return; }
                              ),
                              INIT_MINDEFMAX_SHEWHART_TRIPFREEMARGIN,
                              tripFreeMargin
                        )
   );

   return;
}


void CChartShewhart::LendAntecedentKnobKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   //p_ObsvdPoint->LendKnobKeysTo( borrowerRef ); // Points not currently configured with knobs
   return;
}


bool CChartShewhart::PullInput( void ) {

   xNow = p_ObsvdRain->NowY();
   xMean = p_ObsvdRain->MeanY_toDepth( rainDepthBeingUsed );
   xStdDev = p_ObsvdRain->StdDevY_toDepth( rainDepthBeingUsed );

   if ( ! chartRunning ) {       // Want this assignment only upon chart startup
      stdDevRefNew = xStdDev;
      chartRunning = true;
   }

   return true;
}


bool CChartShewhart::ApplyChart( void ) { // applies Shewhart-like chart to detect transients

   stdDevRef = stdDevRefNew;   // After return, want a member field = stdDevRef value chart just used

   zNow = ( (xNow - xMean) / stdDevRef );   // signed z-score of xNow (see Method Note [1])

   abovePassband = ( ( std::max( 0.0f, SignX( zNow - zPass ) )) != 0.0f ); // Eqn (1) in Method Note [2]
 
   belowPassband = ( ( std::min( 0.0f, SignX( zNow + zPass ) )) != 0.0f ); // Eqn (2) in Method Note [2]
   
   chartTrip = (abovePassband || belowPassband);

   // See Method Note [4] regarding the folowing; 

   tripFreeCount = ( ( (chartTrip == false) ? 1 : 0 ) * std::min( (tripFreeCount+1), tripFreeMargin ) )

                     +
   
                   ( ( (chartTrip == true)  ? 1 : 0 ) * std::max( (tripFreeCount-1), 0 ) );

   
   flipState =  ( (chartTrip == false) && (wasSteadyOnLastValid == false) &&
                (tripFreeCount == tripFreeMargin) )                                // Condition "A"

                  ||    // unavoidably "XOR"

                 ( (chartTrip == true) && (wasSteadyOnLastValid == true) );         // Condition "B"

   isSteadyNow = ( flipState ? (!wasSteadyOnLastValid) : wasSteadyOnLastValid );

   stdDevRefNew = ( isSteadyNow ? xStdDev : stdDevRef );     // See Method Note [5]     


/* METHOD NOTES vvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Z-score is conventionally also normalized to sample size, but in this use that effect is neither
      required or desired.  See reference paper: Veronica (2013).

[2]   For Eqn (1):
      SignX() = +1 if 0 < zNow > zPass (i.e., zNow out of passband high), max() passes +1
      SignX() = -1 if 0 < zNow < zPass (i.e., zNow within passband), max() passes 0
      SignX() = -1 if 0 > zNow, max() passes 0 (i.e., neg. zNow, case goes to Eqn (2))

      for Eqn (2) :
      SignX() = -1 if 0 > zNow < -1*zPass (i.e., zNow out of passband low), min() passes -1
      SignX() = +1 if 0 > zNow > -1*zPass (i.e., zNow within passband), min() passes 0
      SignX() = +1 if 0 < zNow, min() passes 0 (i.e., pos. zNow, case goes to Eqn (1))

[3]   zPass classically would be equal to 3, which assumes x as an IID random variable.  That
      assumption seems appropriate here given this chart is intended to trap transitions to
      transient, non-steady-state (i.e., non-IID) behavior correlating to process "innovations".

[4]   TripFreeCount so named because it is number of SUCCESSIVE Apply() calls with no chart trip.
      Each chart application resulting in a chart trip decrements TripFreeCount from Margin toward zero.
      Each chart application not resulting in a chart trip increments TripFreeCount back toward Margin.

      Intent is that isSteadyNow change "true" -> "false" only when tripFreeCount == zero, and change
      "false" -> "true" only when tripfreeCount == tripFreeMargin.  Otherwise, isSteadyNow stays as is.
      Note: Condidtion "A" can only be "true" if chartTrip is currently "true", and Condition "B" can
      only be "true" if chartTrip is currently "false".  

[5]   current stdDev is used in z-scoring until input no longer "steady", which suspends overwrites and
      leaves last "steady" stdDev in effect as stdDevRef.  Resuming "steady" resumes the overwrites. 


^^^^ END METHOD NOTES */

   return true;
}


void CChartShewhart::Cycle( time_t timestampNow ) {

   validNow = p_ObsvdRain->IsValidOverCycles( numCyclesBeingUsed );

   // >= one input invalid, hold chart result at last value known valid, and return
   // Avoids NaNBOOL returns from the public bool getters (e.g., IsFalling(), etc.) 
   isSteadyNow = ( validNow ? isSteadyNow : wasSteadyOnLastValid );
 
   if ( validNow ) { 
      if ( resetPending ) {  // if time for reset, then recycle tokens prior to charting
         isSteadyNow = true;
         wasSteadyOnLastValid = true;
         tripFreeCount = tripFreeMargin;
         resetPending = false;
      }
      PullInput();
      ApplyChart();
      wasSteadyOnLastValid = isSteadyNow; // recycle token for next call
   }
   return;
}


float CChartShewhart::SignX( float x ) {

   float reply = 1.0f;
   if ( x > 0.0f ) { reply = 1.0f; }
   if ( x < 0.0f ) { reply = -1.0f; }
   return reply;                     // return 1 on x = zero (i.e., SignX() not a signum)
}


EGuiReply CChartShewhart::SetNumSecsBeingUsedTo( int secsGiven ) {

/* next call either returns NaNSIZE on O.O.R., or returns newTimeSpanInCycles floored to fit secsGiven
   being necessarily in integer increments of source's secsPerCycle.
*/
   int secsPerCycle = p_ObsvdRain->SourceRef.SaySecsPerCycle(); 
   size_t numCyclesFittingSecsGiven = FitRainCyclesToSecondsOrRtnNaN( secsGiven, secsPerCycle );

   EGuiReply reply = (  numCyclesFittingSecsGiven != NaNSIZE ?
                        p_ObsvdRain->ShiftSpanOfStatisticsForOneUserFromTo(
                           numCyclesBeingUsed,
                           numCyclesFittingSecsGiven ) :
                        EGuiReply::FAIL_set_askedStatisticBeyondDepthLimit );

   if ( reply == EGuiReply::OKAY_allDone ) {
      numCyclesBeingUsed = numCyclesFittingSecsGiven; 
      numSecsBeingUsed = static_cast<int>(numCyclesBeingUsed) * secsPerCycle ;
   }
   return reply;
}


void CChartShewhart::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   /* Have ObsvdRainRef, so no need to call complementary override in CPointAnalog since
      it gets the same key. $$$ TBD an access to further inputs if obsvd src is a CFormula obj. $$$
   */
   NGuiKey histogramKeyOfSource = p_ObsvdRain->SayHistogramKey();

   bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                histogramKeyOfSource
                                             ) == borrowerRef.end() ?
                                                true :
                                                false
   ); 
   if ( borrowerNotAlreadyHoldingSameKey ) {

      borrowerRef.push_back( histogramKeyOfSource );
   }
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods

bool CChartShewhart::IsSteady( void ) const { return isSteadyNow; }

size_t CChartShewhart::GetNumCyclesBeingUsed( void ) const { return numCyclesBeingUsed; }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CChartTracking concrete class implementation
// Applies CUSUM-like chart to detect hunt or drift of input off of its own (i.e., "auto") mean


// ctor for tracking relative to an ASubject field quantity [ TBD a lambda field for this to work! ]
CChartTracking::CChartTracking(  CSequence& bArg0,
                                 ASubject& bArg1,
                                 CPointAnalog* const bArg2,    // observed var
                                 CChartShewhart* const arg0,   // steady-chart of obs var
                                 std::array<float,3> arg1,     // half-band minDefMax array
                                 std::array<float,3> arg2,     // warn minDefMax array
                                 CController& arg3 )
                                 :  AChart(  bArg0,
                                             bArg1,
                                             EApiType::Chart_track_autoregressive,
                                             bArg2,
                                             nullptr,
                                             nullptr,
                                             nullptr
                                    ),
                                    p_ObsvdShew (arg0),
                                    p_GuideShew (nullptr),
                                    halfBand (arg1[1]),
                                    lagFrac (INIT_MINDEFMAX_TRACKING_LAGFRAC[1]),
                                    register_N (0.0f),
                                    register_P (0.0f),
                                    staleFrac (INIT_MINDEFMAX_TRACKING_STALEFRAC[1]),
                                    warn (arg2[1]),
                                    xObsvdNow (0.0f),
                                    xGuideNow (0.0f),
                                    appsSinceReset (0),
                                    appsBtwnResets (INIT_MINDEFMAX_TRACKING_APPSBETWEENRESETS[1]),
                                    autoregressive (true),
                                    isFallingNow (false),
                                    isHuntingNow (false),
                                    isRisingNow (false),
                                    wasFallingOnLastValid (false),
                                    wasHuntingOnLastValid (false),
                                    wasRisingOnLastValid (false),
                                    trackerOn (false),
                                    parametric (false) {

   CalcOwnTriggerGroup();       // See File Note [1] rainfall.hpp
   bArg0.Register( this );
   ConfigureCycling();
   AttachOwnKnobs( arg3, arg1, arg2 );

}


// ctor for tracking against "guide" (e.g., setpoint) read from BAS channel via CPointAnalog object
CChartTracking::CChartTracking(  CSequence& bArg0,             // sequencer assigned
                                 ASubject& bArg1,
                                 CPointAnalog* const bArg2,    // ref to rainfall of obsvd variable
                                 CChartShewhart* const arg0,   // ref to steady-chart of obs var
                                 CPointAnalog* const bArg3,    // ref to guide as data acquired
                                 CChartShewhart* const arg1,   // ref to guide steady-chart obj
                                 std::array<float,3> arg2,     // half-band minDefMax array
                                 std::array<float,3> arg3,     // warn minDefMax array
                                 CController& arg5 ) 
                                 :  AChart(  bArg0,
                                             bArg1,
                                             EApiType::Chart_track_toDataGuide,
                                             bArg2,
                                             nullptr,
                                             bArg3,
                                             nullptr
                                    ),
                                    p_ObsvdShew (arg0),
                                    p_GuideShew (arg1),
                                    halfBand (arg2[1]),
                                    lagFrac (INIT_MINDEFMAX_TRACKING_LAGFRAC[1]),
                                    register_N (0.0f),
                                    register_P (0.0f),
                                    staleFrac (INIT_MINDEFMAX_TRACKING_STALEFRAC[1]),
                                    warn (arg3[1]),
                                    xObsvdNow (0.0f),
                                    xGuideNow (0.0f),
                                    appsSinceReset (0),
                                    appsBtwnResets (INIT_MINDEFMAX_TRACKING_APPSBETWEENRESETS[1]),
                                    autoregressive (false),
                                    isFallingNow (false),
                                    isHuntingNow (false),
                                    isRisingNow (false),
                                    wasFallingOnLastValid (false),
                                    wasHuntingOnLastValid (false),
                                    wasRisingOnLastValid (false),
                                    parametric (false),
                                    trackerOn (false) {

   CalcOwnTriggerGroup();                                                     
   bArg0.Register(this);
   ConfigureCycling();
   AttachOwnKnobs( arg5, arg2, arg3 );
}


CChartTracking::~CChartTracking( void ) {

// Empty destructor body

}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Private methods

void CChartTracking::CalcOwnTriggerGroup( void ) {

   std::vector<Nzint_t> inputTriggerGroups(0);

   if (p_ObsvdRain) { inputTriggerGroups.push_back( p_ObsvdRain->SourceRef.SayOwnTriggerGroup() ); }
   if (p_ObsvdShew) { inputTriggerGroups.push_back( p_ObsvdShew->SayOwnTriggerGroup() ); }
   if (p_GuideRain) { inputTriggerGroups.push_back( p_GuideRain->SourceRef.SayOwnTriggerGroup() ); }
   if (p_GuideShew) { inputTriggerGroups.push_back( p_GuideShew->SayOwnTriggerGroup() ); }

   Nzint_t latestInputTriggerGroup =
      *std::max_element( inputTriggerGroups.begin(), inputTriggerGroups.end(), std::less<Nzint_t>() );

   if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
      ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
   }
   else { ownTriggerGroup = baseTriggerGroup; }
   return;
}


void CChartTracking::LendAntecedentKnobKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   if ( p_ObsvdShew) { p_ObsvdShew->LendKnobKeysTo( borrowerRef ); }

   if ( p_GuideShew) { p_GuideShew->LendKnobKeysTo( borrowerRef ); }

   return;
}


void CChartTracking::ConfigureCycling( void ) {

   if (autoregressive) { triggersPerCycle = p_ObsvdRain->SourceRef.SayTriggersPerCycle(); }

   // "parametric" case uses ISeqElement class default triggersPerCycle ( = 1 )

   if (!autoregressive && !parametric) {

      triggersPerCycle = std::min(  p_ObsvdRain->SourceRef.SayTriggersPerCycle(),
                                    p_GuideRain->SourceRef.SayTriggersPerCycle()
      );
   }

   triggersUntilCycle = triggersPerCycle;
   secsPerCycle = ( triggersPerCycle * SeqRef.SayTriggerPeriodSecs() );

   return;
}


void CChartTracking::AttachOwnKnobs(   CController& ctrlrRef,
                                       std::array<float,3> minDefMax_halfband,
                                       std::array<float,3> minDefMax_warn ) {

   u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartTrax_halfBand,
                              SayUnits(),
                              EDataSuffix::AboutMean,
                              (  [&]( float userInputVetted ) -> void {
                                    halfBand = userInputVetted;
                                    return; }
                              ),
                              minDefMax_halfband,
                              halfBand )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartTrax_actionSum,
                              EDataUnit::Count_sum,
                              EDataSuffix::None,
                              (  [&]( float userInputVetted ) -> void {
                                    warn = userInputVetted;
                                    return; }
                              ),
                              minDefMax_warn,
                              warn )
   );

  u_KnobsOwned.push_back(  std::make_unique<CKnobFloat>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartTrax_lagFraction,
                              EDataUnit::Ratio_fraction,
                              EDataSuffix::TimesTotal,
                              (  [&]( float userInputVetted ) -> void {
                                    lagFrac = userInputVetted;
                                    return; }
                              ),
                              INIT_MINDEFMAX_TRACKING_LAGFRAC,
                              lagFrac )
   );

  u_KnobsOwned.push_back(  std::make_unique<CKnobFloat>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartTrax_staleFraction,
                              EDataUnit::Ratio_fraction,
                              EDataSuffix::TimesTotal,
                              (  [&]( float userInputVetted ) -> void {
                                    staleFrac = userInputVetted;
                                    return; }
                              ),
                              INIT_MINDEFMAX_TRACKING_STALEFRAC,
                              staleFrac )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobSint>(
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_chartTrax_appsBtwnResets,
                              EDataUnit::Count_cycle,
                              EDataSuffix::None,
                              (  [&]( int userInputVetted ) ->void {
                                    appsBtwnResets = userInputVetted;
                                    return; }
                              ),
                              INIT_MINDEFMAX_TRACKING_APPSBETWEENRESETS,
                              appsBtwnResets
                        )
   );

   return;
}



void CChartTracking::PullValidity( void ) {

   if (autoregressive || parametric) {
      validNow = p_ObsvdShew->IsValid();
      return;
   }
   validNow = ( p_ObsvdShew->IsValid() && p_GuideShew->IsValid() );
   return;
}


void CChartTracking::PullTrackability( void ) {

   if (autoregressive || parametric) { trackerOn = p_ObsvdShew->IsSteady(); }
   else { trackerOn = ( p_ObsvdShew->IsSteady() && p_GuideShew->IsSteady() ); }
   return;
}


void CChartTracking::PullInput( void ) {

   // 'y' of its data source (rainfall) becomes 'x' of any chart
   // $$$ TBD after field experience if means vs. NowY() calls should be used

   if (autoregressive) { // here, "guide" is actually observed variable's own mean
      xObsvdNow = p_ObsvdRain->NowY();
      xGuideNow = p_ObsvdRain->MeanY_toDepth( ( p_ObsvdShew->GetNumCyclesBeingUsed() - 1u ) );
   }

   else if (parametric) {           // *** TBD to implement parametric guide ***
      xObsvdNow = p_ObsvdRain->NowY();
      xGuideNow = NaNFLOAT;
   }

   else {
      xObsvdNow = p_ObsvdRain->NowY();
      xGuideNow = p_GuideRain->NowY();
   }
   return;
}


void CChartTracking::ApplyChart( void ) {

// CUSUM-like chart detects drift & hunt off own mean (autoregressive), or off an independent setpoint
// Cycle() calls this method only when validNow == true, and trackerOn == true. 
   
   float areaPastTop = 0.0f;
   float areaPastBtm = 0.0f;

   ++appsSinceReset;
   if (appsSinceReset > appsBtwnResets) { register_N = 0.0f; register_P = 0.0f; appsSinceReset = 0; }

   // advance in time the non-counting band above and below the guide variable
   float bandTop = xGuideNow + halfBand;
   float bandBtm = xGuideNow - halfBand;

   /*
   "Areas" of xNow-time above and below non-counting band, in (((units-of-x)-minutes)/cycle)
   "distance beyond" = (xObsrvNow - x-value of applicable band edge)
   distance-minutes per cycle = an "area beyond" = "distance beyond" * secsPerCycle/60  
   */
   areaPastTop = (xObsvdNow - bandTop) * (static_cast<float>(secsPerCycle)/60.0f);
   areaPastBtm = (xObsvdNow - bandBtm) * (static_cast<float>(secsPerCycle)/60.0f);

   /*
   Chart "registers" accumulate, over multiple cycles of chart, "area beyond" the non-counting
   band, in (units-of-x)-minutes.  First compute register "deltas" (fresh and stale) for this cycle
   */
   float deltaP_fresh = (areaPastTop * ( (areaPastTop > 0.0f) ? 1.0f : 0.0f ) );
   float deltaN_fresh = (areaPastBtm * ( (areaPastBtm < 0.0f) ? 1.0f : 0.0f ) );

   // Note: ( (boolean expr.) ? 0 : 1 ) returns numbers per the negation of the boolean expression
   // Allow accumulations in registers to "stale off" when not increased in this cycle by xNow
   float deltaP_stale = ( (register_P * staleFrac) * ( (areaPastTop > 0.0f) ? 0.0f : 1.0f ) );
   float deltaN_stale = ( (register_N * staleFrac) * ( (areaPastBtm < 0.0f) ? 0.0f : 1.0f ) );

   // Test registers to +/- the "warn" parameter, also in units of ((units-of-x)-minutes)
   // register updates saturate upon reaching either plus or minus "warn"
   register_P = std::max( 0.0f, std::min( (register_P + (deltaP_fresh - deltaP_stale)), warn) ); 
   register_N = std::min( 0.0f, std::max( (register_N + (deltaN_fresh - deltaN_stale)), -1.0f*warn) );

   isRisingNow = ( (register_P == warn) && !(register_N < (lagFrac*-1.0f*warn)) );

   isFallingNow = ( (register_N == (-1.0f*warn)) && !(register_P > (lagFrac*warn)) );

   isHuntingNow = ( ( register_P == warn ) && ( register_N == (-1.0f*warn) ) );

   return;
}


void CChartTracking::Cycle( time_t timestampNow ) {

   // "Cycling" a chart is the method for representing within it the passage of time.

   PullValidity();   // side-effect is to update 'valid'

   // Cycling chart while it is not valid freezes its principal states without re-evaluating registers 
   if ( ! validNow ) {
      isFallingNow = wasFallingOnLastValid; // Only effective upon 1st cycle after going invalid
      isHuntingNow = wasHuntingOnLastValid;
      isRisingNow = wasRisingOnLastValid;
      return;
   }

   // Clear of short-return conditions... continue charting into the current time step
   wasFallingOnLastValid = isFallingNow;
   wasHuntingOnLastValid = isHuntingNow;
   wasRisingOnLastValid = isRisingNow;
   isFallingNow = NaNBOOL;
   isHuntingNow = NaNBOOL;
   isRisingNow = NaNBOOL;
   trackerOn = true;

    // Chart is not to track during input transients (as detected by the SteadyChart object(s))
   PullTrackability();  // Side-effect is to update 'trackerOn', which remaining calls access

   PullInput();
   ApplyChart();
   return;
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods

void CChartTracking::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   NGuiKey histogramKeyOfObsvdObject = p_ObsvdRain->SayHistogramKey();
   NGuiKey histogramKeyOfGuideObject = p_GuideRain->SayHistogramKey();

   bool borrowerNotAlreadyHoldingKey = ( std::find(
                                             borrowerRef.begin(),
                                             borrowerRef.end(),
                                             histogramKeyOfObsvdObject
                                          ) == borrowerRef.end() ?
                                             true :
                                             false
   ); 
   if ( borrowerNotAlreadyHoldingKey ) {

      borrowerRef.push_back( histogramKeyOfObsvdObject );
   }
   borrowerNotAlreadyHoldingKey = ( std::find(
                                       borrowerRef.begin(),
                                       borrowerRef.end(),
                                       histogramKeyOfGuideObject
                                    ) == borrowerRef.end() ?
                                       true :
                                       false
   ); 
   if ( borrowerNotAlreadyHoldingKey ) {

      borrowerRef.push_back( histogramKeyOfGuideObject );
   }
   return;
}


bool CChartTracking::IsFalling( void ) const { return isFallingNow; }

bool CChartTracking::IsHunting( void ) const { return isHuntingNow; }

bool CChartTracking::IsRising( void ) const { return isRisingNow; }


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   User (GUI) setting of "locks" on objects of state and chart classes only affect value returned by
      class getter methods, e.g., Steady() or Falling().  Internal private fields of those classes
      (e.g., inputSteady, inputFalling) still retain value driven by algorithm and data in the object.


XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
