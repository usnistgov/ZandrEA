// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements abstract and concrete classes for all rainfalls and (basic) statistics kit
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "rainfall.hpp"

#include "viewParts.hpp"         // need type completion due to Histogram Kit c-tor calls
#include "dataChannel.hpp"       // need type completion due to method calls
#include "formula.hpp"           // need type completion due to method calls
#include "state.hpp"
#include "rule.hpp"
#include "agentTask.hpp"        // must follow other *.h includes to override F.D. with a complete type

#include <limits>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cmath>                // SWB: std::sqrt uses this

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// ARainfall ABC implementation

/* One (statically allocated) actual "rainfall" register is reused among all objects of appropriate
      subclass.  No rainfall init, as gets refilled prior to each object's access, to the depth that
      object requires.

   "Newest" data at lowest index (container "front") and "oldest" at highest index (container "back").

   Number of indicies in a Snapshot, FIXED_SNAPSHOT_SIZE, is that needed to cover
      FIXED_SNAPSHOT_SPANSECS at fastest data rate used anywhere, which is FIXED_TASKCLOCK_SECSPERTRIGGER.
      If particular rainfall instance cycles slower than that data rate, each of its logged bindex values
      gets label-mapped and then "pasted" (as a float) into more than one index of the snapshot.
*/

Rainfall_t ARainfall::rainfallRegister;

ARainfall::ARainfall(   ISeqElement& arg0,
                        EApiType arg1 ) 
                        :  SourceRef (arg0),
                           ruleStatesLoggedAsBindex_byRuleUai(),
                           statesLoggedAsBindex(0),
                           valuesLoggedAsBindex(0),
                           snapshotsStateBindex_bySetSgi(),
                           snapshotsValueBindex_bySetSgi(),
                           spansInUse(),       // < first, second > = < depth as key, num uses >
                           movingHourSpanInCycles (   static_cast<size_t>(
                                                         3600 / arg0.SaySecsPerCycle() )
                           ),
                           lastIndexInMovingHour (movingHourSpanInCycles - 1u), 
                           numCyclesDuringSnapshot ( static_cast<size_t>(
                              ( FIXED_KRONO_SNAPSHOT_SPANSECS / arg0.SaySecsPerCycle() ) )
                           ),
                           numCyclesLogging (0u),
                           secsPerCycle ( arg0.SaySecsPerCycle() ),
                           numPastesEachCycleToSnapshot (   arg0.SaySecsPerCycle() /
                                                            arg0.SaySecsPerTrigger()
                           ),
                           secsLoggingMax ( FIXED_DATALOG_SIZECYCLES_MAX * arg0.SaySecsPerCycle() ),
                           secsLogging (START_DATALOG_SECSLOGGING),
                           ownApiType (arg1) {
   // Empty c-tor body
}

// ~ARainfall() destructor must be left as virtual [in class header]
ARainfall::~ARainfall( void ) { }

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Protected methods of abstract class

void ARainfall::ReadBindexLogIntoRainfall( const std::deque<Bindex_t>& logToRead ) {

   // Agnostic as to whether Bindex_t log read holds analog values or holds analog/fact/rule states

   std::deque<Bindex_t>::const_iterator logIter = logToRead.begin();

/* [row][column] paradigm in use of rainfallRegister is:
   [cycles ago][bin (index) of the value of an analog or of the state of an analog, fact, or rule]
*/
   for ( auto rainRowIter = rainfallRegister.begin();
         rainRowIter < ( rainfallRegister.begin() + logToRead.size() ); // log sizes <= rain size
         ++rainRowIter ) {

         (*rainRowIter).fill( false );    // all columns of rainfall, whether reading values or states
         if ( *logIter != NaNBINDEX ) {   // reading NaN from log leaves whole bin row (cycle) 'false'
            // Sole 'true' put in column indexed by log value read
            (*rainRowIter)[ static_cast<size_t>( *logIter ) ] = true;
         }
         std::advance(logIter,1);
   } 
   return;
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods

EGuiReply ARainfall::ResizeLoggingToAtLeastSecsAgo( int secsLoggingWanted ) {

/* $$$ TBD to refactor this method as sole means rainfall logs are resized by ANYTHING, and call it
   from existing subclass resizers $$$
*/

   int secsPerLogCycle = SourceRef.SaySecsPerCycle();

   // C++ int divide rounds to floor. To round to ceiling when no negative arg or overflow possible:
   size_t numCyclesProposed = static_cast<size_t>(
                                 (secsLoggingWanted + secsPerLogCycle - 1) /
                                 secsPerLogCycle );

   if (  (numCyclesProposed < FIXED_KRONO_SNAPSHOT_SIZE) || // Don't go smaller than a snapshot
         (numCyclesProposed > FIXED_RAINFALL_SIZE) ) {

      return EGuiReply::FAIL_set_givenDataLoggingSizeNotWithinBounds;
   }
   if ( numCyclesProposed < numCyclesLogging ) {   // In bounds but > current size to get here
      return EGuiReply::OKAY_allDone;
   }

   // Else, sizeProposed in bounds but > than current size

   if ( ownApiType == EApiType::Rainfall_ruleKit ) {
      for ( auto& pairRef_log : ruleStatesLoggedAsBindex_byRuleUai ) {
         pairRef_log.second.resize( numCyclesProposed, BINDEX_RULE_UNAVAIL );
      }
   }
   else {
      if (ownApiType == EApiType::Rainfall_fact) {
         statesLoggedAsBindex.resize( numCyclesProposed, BINDEX_FACT_UNAVAIL );
      }
      else {   // I am a Rainfall of analog subclass
         valuesLoggedAsBindex.resize(            
            numCyclesProposed,
            valuesLoggedAsBindex.back()   // extend analog value log using oldest value presently held
         );
         statesLoggedAsBindex.resize(
            numCyclesProposed,
            BINDEX_ANALOGSTATE_UNAVAIL    // extend analog state log using bindex for "unavailable"
         );
      }
   }   
   secsLogging = static_cast<int>(numCyclesProposed) * secsPerLogCycle;
   return EGuiReply::OKAY_allDone;
}


bool ARainfall::IsSnapshotSgiValid( Nzint_t sgiToCheck ) const {

   return ( (  snapshotsStateBindex_bySetSgi.count( sgiToCheck ) +
               snapshotsValueBindex_bySetSgi.count( sgiToCheck ) ) == 0 ?
            false :
            true );
}


void ARainfall::DestroySnapshotForSetSgi( Nzint_t snapshotSetSgi) {

   snapshotsStateBindex_bySetSgi.erase( snapshotSetSgi );
   snapshotsValueBindex_bySetSgi.erase( snapshotSetSgi );
   return;
}    


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of concrete rainfall class CRainAnalog


SBinParamsAnalogValue CRainAnalog::SpecifyBinParamsForAnalogValues(  float minValueLabeled,
                                                                     float maxValueLabeled ) {

/* Since bins are indexed C-style zero-based and labled by "floor" value, inclusive range from min to
   max must be divided among (N-1) labels, where N = number of bins made avaiable by the size of the
   Bindex_t type. 
*/

   float binWidth = std::max( ( maxValueLabeled - minValueLabeled ) /
                                 static_cast<float>( FIXED_RAIN_ANALOGVALUE_NUMBINS - 1),
                              ( 100.0f * std::numeric_limits<float>::lowest() ) );

   std::array<float,FIXED_RAIN_ANALOGVALUE_NUMBINS> stackLabels;
  
   float pasteIn = minValueLabeled;
   for ( size_t i = 0; i < FIXED_RAIN_ANALOGVALUE_NUMBINS; i++) {
               
      stackLabels[i] =  pasteIn;
      pasteIn = pasteIn + binWidth;
   }

   return SBinParamsAnalogValue( stackLabels, binWidth );      
}
  
#define ANALOGBINSDEF \
   { \
      { EDataRange::Analog_percent, SpecifyBinParamsForAnalogValues( 0.0f, 100.0f ) }, \
      { EDataRange::Analog_zeroToOne, SpecifyBinParamsForAnalogValues( 0.0f, 1.0f ) }, \
      { EDataRange::Analog_zeroTo3, SpecifyBinParamsForAnalogValues( 0.0f, 3.0f ) }, \
      { EDataRange::Analog_zeroTo120, SpecifyBinParamsForAnalogValues( 0.0f, 120.0f ) }, \
      { EDataRange::Analog_zeroTo3k, SpecifyBinParamsForAnalogValues( 0.0f, 3000.0f ) } \
   }

const SBinParamsAnalogValue& CRainAnalog::lookup_AnalogValueBins(const EDataRange &dr) {
   static AnalogValueBinsTable_t analogValueBinsLookup(ANALOGBINSDEF);
   return analogValueBinsLookup.at(dr);
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV

CRainAnalog::CRainAnalog(  CPointAnalog& bArg0,        // for base to call getters in its c-tor
                           std::vector<NGuiKey>& arg0,  // knob keys, src obj and antecedents -> histo
                           ASubject& arg1,  // non-const, histogram regstr to View and get caption info
                           EDataRange arg2 )
                           :  ARainfall(  bArg0,
                                          EApiType::Rainfall_analog
                              ),

// Next line throws uncaught if arg2 (EDataRange) is not an analog data range choice

                              binParamsRef ( lookup_AnalogValueBins(arg2) ),
                              u_Histogram_analog ( std::make_unique<CHistogramAnalog>(
                                                   arg1,
                                                   bArg0,
                                                   arg0,
                                                   movingHourSpanInCycles,
                                                   binParamsRef.labels[0],
                                                   binParamsRef.binWidth )
                              ),
                              yMeansByDepth(),
                              yVariancesByDepth(),
                              yStdDevsByDepth(),
                              xNow (binParamsRef.labels[0]),
                              yNow (NaNFLOAT),
                              yMaxHeld (NaNFLOAT),
                              yMinHeld (NaNFLOAT),
                              xMaxSeen (NaNFLOAT),
                              xMinSeen (NaNFLOAT),
                              binOverUnderSeen (false),
                              firstCycle (true),
                              validAtSource (NaNBOOL) {


   statesLoggedAsBindex.assign(  static_cast<size_t>( secsLogging / bArg0.SaySecsPerCycle() ),
                                 BINDEX_ANALOGSTATE_UNAVAIL
   );
   valuesLoggedAsBindex.assign(  statesLoggedAsBindex.size(),
                                 BINDEX_ANALOGVALUE_UNAVAIL
   );
   spansInUse.emplace( std::pair<size_t, Nzint_t>( movingHourSpanInCycles, 1 ) );
   yMeansByDepth.emplace( std::pair<size_t, float>( movingHourSpanInCycles, NaNFLOAT ) );
   yVariancesByDepth.emplace( std::pair<size_t, float>( movingHourSpanInCycles, NaNFLOAT ) );
   yStdDevsByDepth.emplace( std::pair<size_t, float>( movingHourSpanInCycles, NaNFLOAT ) );

}


CRainAnalog::CRainAnalog(  CFormula& bArg0,
                           std::vector<NGuiKey>& arg0,
                           ASubject& arg1,
                           EDataRange arg2 )
                           :  ARainfall(  bArg0,
                                          EApiType::Rainfall_analog
                              ),

// Next line throws uncaught if arg2 (EDataRange) is not an analog choice

                              binParamsRef ( lookup_AnalogValueBins(arg2) ),
                              u_Histogram_analog ( std::make_unique<CHistogramAnalog>(
                                                   arg1,
                                                   bArg0,
                                                   arg0,
                                                   movingHourSpanInCycles,
                                                   binParamsRef.labels[0],
                                                   binParamsRef.binWidth )
                              ),
                              yMeansByDepth(),
                              yVariancesByDepth(),
                              yStdDevsByDepth(),
                              xNow (binParamsRef.labels[0]),
                              yNow (NaNFLOAT),
                              yMaxHeld (NaNFLOAT),
                              yMinHeld (NaNFLOAT),
                              xMaxSeen (NaNFLOAT),
                              xMinSeen (NaNFLOAT),
                              binOverUnderSeen (false),
                              firstCycle (true),
                              validAtSource (NaNBOOL) {


   statesLoggedAsBindex.assign(  static_cast<size_t>( secsLogging / bArg0.SaySecsPerCycle() ),
                                 BINDEX_ANALOGSTATE_UNAVAIL
   );
   valuesLoggedAsBindex.assign(  statesLoggedAsBindex.size(),
                                 BINDEX_ANALOGVALUE_UNAVAIL
   );

   spansInUse.emplace( std::pair<size_t, Nzint_t>( movingHourSpanInCycles, 1 ) );
   yMeansByDepth.emplace( std::pair<size_t, float>( lastIndexInMovingHour, NaNFLOAT ) );
   yVariancesByDepth.emplace( std::pair<size_t, float>( lastIndexInMovingHour, NaNFLOAT ) );
   yStdDevsByDepth.emplace( std::pair<size_t, float>( lastIndexInMovingHour, NaNFLOAT ) );

}


CRainAnalog::~CRainAnalog( void ) {

// Empty destructor body
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Private Methods

void CRainAnalog::UpdateLogging_Analog(   time_t timestampNow,
                                          bool beginNewClockHour,
                                          bool beginNewCalendarDay )  {

   /* First, offset xNow upward by half a bin width, to avoid statistical bias from always rounding to
      "floor value" in following step.  Then "scale" result to a floating point count of "bin widths"
      the offset xNow value lies above minimum label.
   */

   float xScaled = ( (  ( xNow + (binParamsRef.binHalfWidth) )
                        - binParamsRef.labels.front()
                     ) /
                     binParamsRef.binWidth );

   /* Round xScaled (essentially a real number "index") into an int.  Signed int used to allow negative
      value without crashing function.  Any negative "index" will get attention elsewhere via xMinEver
      being O.O.R. of bin labeling.
   */

   int iDropRaw = static_cast<int>( std::floor( xScaled ) );

   bool binOverUnderNow =  (iDropRaw < 0) ||
                           (iDropRaw > static_cast<int>( FIXED_RAIN_ANALOGVALUE_NUMBINS-1 ) );

   binOverUnderSeen = ( binOverUnderNow ? true : binOverUnderSeen ); // a "true" on "...Now" will "stick"

   // squash any O.O.R bin index (STL min/max requires size_t cast to signed int)

   size_t iDropBin = ( binOverUnderNow ?
                        static_cast<size_t>(
                           std::min(   std::max( 0, iDropRaw ),
                                       static_cast<int>( FIXED_RAIN_ANALOGVALUE_NUMBINS-1 ) )
                        ) :
                        static_cast<size_t>( iDropRaw )
   );

   Bindex_t bindexOfValueNow = static_cast<Bindex_t>( iDropBin );

   Bindex_t bindexOfStateNow = (   (validAtSource & (! binOverUnderNow) ) ?
                                    static_cast<Bindex_t>( 1u ) :          // state = "Valid"
                                    static_cast<Bindex_t>( 0u ) );         // state = "Invalid"

   //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
   if ( firstCycle ) {

      // overwrite initial values w/ value first received to preserve valid statistics from start up
      std::fill(  valuesLoggedAsBindex.begin(),
                  valuesLoggedAsBindex.end(),
                  bindexOfValueNow );
      firstCycle = false;
   } 
   //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/

   // Cycle Histogram Kit with data from this Rainfall cycle
   u_Histogram_analog->Cycle( timestampNow,
                              beginNewClockHour,
                              beginNewCalendarDay,
                              bindexOfStateNow,
                              statesLoggedAsBindex[lastIndexInMovingHour],
                              bindexOfValueNow,
                              valuesLoggedAsBindex[lastIndexInMovingHour] );       

   // Update logs, most recent value at lowest index ("front"); oldest value at highest index ("back")
   valuesLoggedAsBindex.pop_back(); valuesLoggedAsBindex.push_front( bindexOfValueNow );
   statesLoggedAsBindex.pop_back(); statesLoggedAsBindex.push_front( bindexOfStateNow );

   yNow = binParamsRef.labels[iDropBin];

   return;
}

//======================================================================================================/

void CRainAnalog::UpdateStatistics_Analog( void ) {

   /* Method expects rainfall deque has just been updated with push-in of a new "current" row at front,
     and pop-off of oldest row at back. Note that valid stats are obtained even initially, when only one
     row (the first) has a bin with a bool "one" ("true") in it, as are stats based on total "ones". */

   ReadBindexLogIntoRainfall( valuesLoggedAsBindex );

   // Could declare these high-freq "reusers" as members, but S.O. says they'll run faster on call stack
   size_t                  depth;
   BinSumsAnalogValue_t    binSums_values;
   BinSum_t                sumBinSums_values;  
   std::vector<float>      binFractions_values( FIXED_RAIN_ANALOGVALUE_NUMBINS, 0.0f );
   std::vector<float>      stackVector_values( FIXED_RAIN_ANALOGVALUE_NUMBINS, 0.0f );
   float                   stackF;

   // Need separate statistics for each depth in use, so TPD re-iterates the whole rainfall
   for ( auto& pairRef : spansInUse ) { 

      depth = ( pairRef.first - 1u );
      binSums_values.fill( 0 );

      for (size_t cyclesBack=0; cyclesBack < depth; ++cyclesBack) { // See Method Note [1]

         // currently Bindex_t is a u-char, so any bindex from 0 to 255 < 256
         for ( size_t iBin=0; iBin < FIXED_RAIN_ANALOGVALUE_NUMBINS; ++iBin ) {
      
            if ( rainfallRegister[cyclesBack][iBin] ) ++binSums_values[iBin]; 
         }
      }

      // prevent zero (however unlikely) on binSums accumulation to protect later divide
      sumBinSums_values = std::max(   std::accumulate(  binSums_values.begin(),
                                                         binSums_values.end(),
                                                         0 ),
                                       1 );
      
      // Divide each element of binSums by sumBinSums, writing each quotient into binFractions
      std::transform(   binSums_values.begin(),
                        binSums_values.end(),
                        binFractions_values.begin(),
                        // the below 'vectorizes' sumBinSums to a size conforming to other operands
                        std::bind2nd(  std::divides<float>(),
                                       sumBinSums_values ) ); 

      // Mean over depth = inner (scalar) product of mapping bin fractions through bin labels
      yMeansByDepth[depth] = std::inner_product(   binFractions_values.begin(),
                                                   binFractions_values.end(),
                                                   binParamsRef.labels.begin(),
                                                   0.0f ); // add nothing to scalar product

      // write bin-to-mean distances into stackVector
      std::transform(   binParamsRef.labels.begin(),
                        binParamsRef.labels.end(),
                        stackVector_values.begin(),
                        std::bind2nd(  std::minus<float>(),
                                       yMeansByDepth[depth] ) ); // xform dynamic with data here only!

      // Square stackVector via element-by-element multiply, writing product back to stackVector.
      // This puts squared residuals of data off its mean for each bin into each element of stackVector
      std::transform(   stackVector_values.begin(),
                        stackVector_values.end(),
                        stackVector_values.begin(), // squaring => 2nd operand iter starts same as 1st
                        stackVector_values.begin(), // iter to where result of operation is to be written
                        std::multiplies<float>() );   // operation called (std or user functor)

      // Inner product maps bin-to-mean distances thru bin fractions of data, summing into a scalar
      // This results in stackF holding variance of data over timespan of cycles counted into the bins
      stackF = std::inner_product(  stackVector_values.begin(),
                                    stackVector_values.end(),
                                    binFractions_values.begin(),
                                    0.0f );
 
      // Smallest value assignable to variance is a constant = (binning process uncertainty)^2
      yVariancesByDepth[depth] = std::max( binParamsRef.minVariance, stackF );

      // Std dev never < sqrt(minVariance), thus no divide-by-zeros possible in calcs downstream
      yStdDevsByDepth[depth] = std::sqrt( yVariancesByDepth[depth] );

   }  // close for-loop on depth

/* Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Upon "apparent" runtime (i.e., cycling on sampled data, vs. "latent", construction-stage runtime),
      analog value bindex logs are filled to back end with bindex of first data value received as input.
      Thus, statistics calculated over early cycles first completely, then decreasingly, reflect that
      one first value, since at startup no other value was "available".

      So, analog state log is first fully backfilled with "Unavailable", then actual state of first sample
      is pushed onto its front, with ending "Unavailable" popped off back.  Since Traces draw from bindex
      logs, they show similar effects.  Histograms must startup under a similar, compatible process, seen
      in ViewParts.cpp.

      The above provides a fairly "transparent" app startup: i.e., (1) everything works from "Input One"
      without crashing the app, and (2) little to no code still gets executed for ever despite being
      made superfluous once the logs/rainfalls fill with actual sampled data.     

''' End Method Notes''' */


   return;



}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public Methods

TraceGuiNumbersOldToNew_t CRainAnalog::SayGuiNumbersFromSnapshotInSet( Nzint_t snapshotSetSgi ) const {

   auto tableIter = snapshotsValueBindex_bySetSgi.find( snapshotSetSgi ); 

   if  ( tableIter == snapshotsValueBindex_bySetSgi.end() ) {
      // no match found for ID given. Return empty vector
      return TraceGuiNumbersOldToNew_t();
   }

   // Else, tableIter now points at table element (a std::pair) keyed by snapshotSetId

   std::array<GuiFpn_t, FIXED_KRONO_SNAPSHOT_SIZE> numbersOldestToNewest;
 
   /* Table holds snapshots as fixed-length arrays of bindex values ordered from the newest to oldest,
      as that is order they have in the originating bindex log ( a std::deque ). But, snapshots are to
      display on GUI as time-series GuiFpn_t values ordered oldest (leftmost) to newest (rightmost).
      So...
   */
   auto forwardReadingCiter = (*tableIter).second.cbegin();
   for ( auto  reverseWritingIter = numbersOldestToNewest.rbegin();
               reverseWritingIter < numbersOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = static_cast<GuiFpn_t>( binParamsRef.labels[*forwardReadingCiter] );
      std::advance( forwardReadingCiter, 1 );
   }
   // nameless return of type, so compiler invokes move semantics (?)
   return TraceGuiNumbersOldToNew_t(numbersOldestToNewest.data(),
                              numbersOldestToNewest.data() +
                                 (sizeof(numbersOldestToNewest) / sizeof(GuiFpn_t)) );
}

//======================================================================================================/

TraceGuiStatesOldToNew_t CRainAnalog::SayGuiStatesFromSnapshotInSet( Nzint_t snapshotSetSgi ) const {

   auto tableIter = snapshotsStateBindex_bySetSgi.find( snapshotSetSgi ); 

   if  ( tableIter == snapshotsStateBindex_bySetSgi.end() ) {
       // no match found for ID given. Return empty vector
      return TraceGuiStatesOldToNew_t();
   }

   // Else, tableIter now points at table element (a std::pair) keyed by snapshotSetId

   std::array<EGuiState, FIXED_KRONO_SNAPSHOT_SIZE> statesOldestToNewest;
 
   /* Table holds snapshots as fixed-length arrays of bindex values ordered from the newest to oldest,
      as that is order they have in the originating bindex log ( a std::deque ). But, snapshots are to
      display on GUI as time-series GuiFpn_t values ordered oldest (leftmost) to newest (rightmost).
      So...
   */
   auto forwardReadingCiter = (*tableIter).second.cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter < statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_ANALOGSTATEBINS[ *forwardReadingCiter ];
      std::advance( forwardReadingCiter, 1 );
   }
   // nameless return of type, so compiler invokes move semantics (?)
   return TraceGuiStatesOldToNew_t( statesOldestToNewest.data(),
                                    statesOldestToNewest.data() +
                                    (sizeof( statesOldestToNewest ) / sizeof( EGuiState )) );
}



//======================================================================================================/

TraceGuiNumbersOldToNew_t
CRainAnalog::SayGuiNumbersFromBindexLog( void ) {

      /* $$$ c-tor of CKrono precludes numCyclesInKrono > FIXED_RAIN_CYCLESLOGGING_MAX. Only guard against
      oversizing logs is having only Kronos call this method, using their size   $$$ TBD to refactor
   */

   TraceGuiNumbersOldToNew_t numbersOldestToNewest( FIXED_KRONO_SNAPSHOT_SIZE, NaNDBL );

   // Must backfill any extention w/ interim "bum" values that are PLOTTABLE, so use oldest value held
   //if ( numCyclesInKrono > valuesLoggedAsBindex.size() ) {

   //   valuesLoggedAsBindex.resize( numCyclesInKrono, valuesLoggedAsBindex.back() );
   //}
  
   /* bindexLog holds realtime series of bindex values ordered from the newest to oldest.
      But, realtime series display on GUI as GuiFpn_t values ordered oldest (left) to newest (right).
   */
   auto forwardReadingCiter = valuesLoggedAsBindex.cbegin();
   for ( auto  reverseWritingIter = numbersOldestToNewest.rbegin();
               reverseWritingIter < numbersOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = static_cast<GuiFpn_t>( binParamsRef.labels[*forwardReadingCiter] );

// $$$ w/o next line sure crash if length of krono > of bindexLog - TBD refactor to make impossible $$$
      if ( forwardReadingCiter != valuesLoggedAsBindex.cend() ) {
         std::advance(forwardReadingCiter, 1);
      }
   }
   return numbersOldestToNewest;
}

//======================================================================================================/

TraceGuiStatesOldToNew_t CRainAnalog::SayGuiStatesFromBindexLog( void ) {
/*
   if ( numCyclesWanted > numCyclesLogging ) {

      if ( numCyclesWanted > FIXED_DATALOG_SIZECYCLES_MAX )  { return TraceGuiStatesOldToNew_t(0); }

      statesLoggedAsBindex.resize(  numCyclesWanted, BINDEX_ANALOGSTATE_UNAVAIL );
   }
*/
   TraceGuiStatesOldToNew_t statesOldestToNewest( FIXED_KRONO_SNAPSHOT_SIZE, EGuiState::Undefined );
  
   /* stateLog holds realtime series of state values ordered from the newest to oldest.
      But, realtime series display on GUI as values ordered oldest (left) to newest (right).
   */
   auto forwardReadingCiter = statesLoggedAsBindex.cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter < statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_ANALOGSTATEBINS[ *forwardReadingCiter ];

// $$$ w/o next line sure crash if length of krono > of bindexLog - TBD refactor to make impossible $$$
      if ( forwardReadingCiter != statesLoggedAsBindex.cend() ) {
         std::advance(forwardReadingCiter, 1);
      }
   }
   return statesOldestToNewest;
}

//======================================================================================================/

NGuiKey CRainAnalog::SayHistogramKey( void ) const {

   return u_Histogram_analog->SayGuiKey();
}


GuiFpn_t CRainAnalog::SayGuiNumberFromNewestBindex( void ) const {

   return static_cast<GuiFpn_t>( binParamsRef.labels[ valuesLoggedAsBindex.front() ] );
}


EGuiState CRainAnalog::SayGuiStateFromNewestBindex( void ) const {

   return GUISTATES_ANALOGSTATEBINS[ statesLoggedAsBindex.front() ];
}

//======================================================================================================/
// Data getters ('X' refers to values prior to binning, 'Y' is the value post-binning)

float CRainAnalog::NowX( void ) const { return xNow; }


float CRainAnalog::NowY( void ) const { return yNow; }


float CRainAnalog::OldY_atDepth( size_t depthWanted ) const {
 
   return ( ( depthWanted < valuesLoggedAsBindex.size() ) ?     // Have it?
               binParamsRef.labels[ valuesLoggedAsBindex[depthWanted] ] :
               NaNFLOAT );
}


float CRainAnalog::MeanY_toDepth( size_t depthWanted ) const {
 
   return ( ( yMeansByDepth.count(depthWanted) != 0 ) ?     // Have it?
            yMeansByDepth.at(depthWanted) :
            NaNFLOAT );
}


float CRainAnalog::StdDevY_toDepth( size_t depthWanted ) const {
 
   return ( ( yStdDevsByDepth.count(depthWanted) != 0) ?     // Have it?
            yStdDevsByDepth.at(depthWanted) :
            NaNFLOAT );
}


float CRainAnalog::SayCenterBinLabel( void ) const {

   return binParamsRef.labels[ (FIXED_RAIN_ANALOGVALUE_NUMBINS / 2u) ];
}


float CRainAnalog::SayBinWidth( void ) const { return binParamsRef.binWidth; }

//======================================================================================================/

EGuiReply CRainAnalog::AddNewStatisticsUserSpanningCycles( size_t numCyclesSpannedForNewUser ) {

/* Single-cycle MeanY is okay [ ="NowY" value], single-cycle StdDevY is okay [= bin halfWidth]
   "depth" is an index (of type size_t), "span" is a size (number of cycles, also of type size_t).
   Triadic used here in case a flawed call gives method stats span (a size) of value = 0
   $$$ TBD to simplify/refactor this to tabulate on spans instead of depths $$$
*/
   if ( (numCyclesSpannedForNewUser > FIXED_RAINFALL_SIZE) || (numCyclesSpannedForNewUser < 1u )  ) { 

      return EGuiReply::FAIL_set_askedStatisticBeyondDepthLimit;
   }

   if ( numCyclesSpannedForNewUser > valuesLoggedAsBindex.size() ) {

      // never to shrink log, only expand (to <= rainfall cycles)
      // backfill extention w/ interim "unavailable" values that are PLOTTABLE, so use oldest value held  
      valuesLoggedAsBindex.resize(  numCyclesSpannedForNewUser,
                                    valuesLoggedAsBindex.back()
      );
   }
   std::map<size_t, Nzint_t>::iterator p_match = spansInUse.find( numCyclesSpannedForNewUser ); 
   if ( p_match == spansInUse.end() ) { // end() is theor. index beyond last element, no match if reached

      spansInUse.insert( std::pair<size_t, Nzint_t>( numCyclesSpannedForNewUser, 1u ) );
   }
   else {   // have match, so 'depth wanted' is already in use. Increase count of users for this depth  
      p_match->second = (p_match->second)++;
   }

   return EGuiReply::OKAY_allDone;
}

//======================================================================================================/

EGuiReply CRainAnalog::ShiftSpanOfStatisticsForOneUserFromTo(  size_t oldSpanInCycles,
                                                               size_t newSpanInCycles ) {

/* Single-cycle MeanY is okay [ ="NowY" value], single-cycle StdDevY is okay [= bin halfWidth]
   "depth" is an index (of type size_t), "span" is a size (number of cycles, also of type size_t).
   Triadic used here in case a flawed call gives method stats span (a size) of value = 0
   $$$ TBD to simplify/refactor this to tabulate on spans instead of depths $$$
*/

   if ( (newSpanInCycles > FIXED_RAINFALL_SIZE) || (newSpanInCycles < 1u )  ) { 

      return EGuiReply::FAIL_set_askedStatisticBeyondDepthLimit;
   }

   if ( newSpanInCycles > valuesLoggedAsBindex.size() ) {

      // never shrink log, only expand (to <= rainfall cycles)
      // Must backfill extention w/ interim "bum" values that are PLOTTABLE, so use oldest value held  
      valuesLoggedAsBindex.resize(  newSpanInCycles,
                                    valuesLoggedAsBindex.back() );
   }

   std::map<size_t, Nzint_t>::iterator p_match = spansInUse.find( newSpanInCycles );
 
   if ( p_match == spansInUse.end() ) { // end() is theor. index beyond last element, no match if reached

      spansInUse.insert( std::pair<size_t, Nzint_t>( newSpanInCycles, 1 ) );
   }
   else {   // have match, so 'depth wanted' is already in use. Increase count of users for this depth
      p_match->second = (p_match->second)++;
   }

   p_match = spansInUse.find( oldSpanInCycles );              // Should be no way p_match == end() here
   p_match->second = (p_match->second)--;
   if ( (p_match->second) < 1 ) { spansInUse.erase( p_match ); }

   return EGuiReply::OKAY_allDone;
}

//======================================================================================================/

void CRainAnalog::CaptureSnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   SnapshotBindex_t snapshot_values; // fixed size, secs/cycle = fastest (smallest) in app
   SnapshotBindex_t snapshot_states;

   if ( numPastesEachCycleToSnapshot == 1 ) {         // are secs/cycle same for source and snapshot?

      for ( size_t iLog = 0; iLog < FIXED_KRONO_SNAPSHOT_SIZE; ++iLog ) {
         snapshot_values[iLog] = valuesLoggedAsBindex[iLog];
         snapshot_states[iLog] = statesLoggedAsBindex[iLog];
      }
   }
   else {   // secs/cycle of source > that of snapshot, so source values must be "pasted" multiple times

      auto readingCiter = valuesLoggedAsBindex.cbegin();   // Stepping through source bindex log...
      auto writingIter = snapshot_values.begin();      // ...and simultaneously through target snapshot...
      while ( writingIter != snapshot_values.end() ) {

         // ...read same value, but write into advancing indicies, until pastes are done or end is hit        
         for ( int pastesToDo = numPastesEachCycleToSnapshot; pastesToDo > 0; --pastesToDo ) {
            *writingIter = *readingCiter;
            // can pastesToDo start off > snapshot length (not sure, $$$ TBD to refactor next line $$$ ) 
            if ( writingIter != snapshot_values.end() ) { std::advance(writingIter, 1); }
         }
         ++readingCiter;   // Rain c-tor and methods prevent bindexLog shorter than snapshot
      }

      readingCiter = statesLoggedAsBindex.cbegin();   // Stepping through source bindex log...
      writingIter = snapshot_states.begin();      // ...and simultaneously through target snapshot...
      while ( writingIter != snapshot_states.end() ) {

         // ...read same value, but write into advancing indicies, until pastes are done or end is hit        
         for ( int pastesToDo = numPastesEachCycleToSnapshot; pastesToDo > 0; --pastesToDo ) {
            *writingIter = *readingCiter;
            // can pastesToDo start off > snapshot length (not sure, $$$ TBD to refactor next line $$$ ) 
            if ( writingIter != snapshot_states.end() ) { std::advance(writingIter, 1); }
         }
         ++readingCiter;   // Rain c-tor and methods prevent bindexLog shorter than snapshot
      }
   }   
   snapshotsValueBindex_bySetSgi.emplace(  std::pair<Nzint_t, SnapshotBindex_t>(
                                             snapshotSetSgi,
                                             snapshot_values ) );

   snapshotsStateBindex_bySetSgi.emplace(  std::pair<Nzint_t, SnapshotBindex_t>(
                                             snapshotSetSgi,
                                             snapshot_states ) );
   return;
}


bool CRainAnalog::IsValidOverCycles( size_t spanCallerIsUsing ) const {

   auto firstIteratorPositionPastSpan = ( statesLoggedAsBindex.begin() + spanCallerIsUsing );

   // allows any "Analog_unavailable" to sneak by okay
   return ( std::find(  statesLoggedAsBindex.begin(),
                        firstIteratorPositionPastSpan,
                        BINDEX_ANALOGSTATE_INVALID )
            == firstIteratorPositionPastSpan );
}

//======================================================================================================/

void CRainAnalog::Cycle(   time_t timestampNow,
                           bool beginNewClockHour,
                           bool beginNewCalendarDay,
                           float valueGiven,
                           bool isValueGivenValidAtSourceObj )  {

/* $$$ Employment of a bool input from external data source vv. the number it sent being valid is TBD $$$
   So currently point holds last value known to external (client) as valid
*/
   validAtSource = isValueGivenValidAtSourceObj;    // hard-coded true at source until above TBD done
   xNow =  ( validAtSource ? valueGiven : xNow ); 

   UpdateLogging_Analog(   timestampNow,
                           beginNewClockHour,
                           beginNewCalendarDay
   );

   UpdateStatistics_Analog();
   // $$$ TBD to alert on binOverUnder $$$
   firstCycle = false;
   return;
}
 

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Implementations for CRainFact


CRainFact::CRainFact(   AFact& bArg0,     // for base to call getters in its c-tor
                        std::vector<NGuiKey>& arg0,  // knob keys of src obj and its antecedents -> histogram
                        ASubject& arg1 )   // non-const, histo regstr to View and get caption info
                        :  ARainfall(  bArg0,
                                       EApiType::Rainfall_fact
                           ),
                           u_Histogram_fact (   std::make_unique<CHistogramFact>(
                                                arg1,
                                                bArg0,
                                                arg0,
                                                movingHourSpanInCycles )
                            ),
                            bindexNow (NaNBINDEX),
                            lastValidClaim (NaNBOOL) {

   statesLoggedAsBindex.assign(  static_cast<size_t>(
                                    START_DATALOG_SECSLOGGING / bArg0.SaySecsPerCycle()
                                 ),
                                 BINDEX_FACT_UNAVAIL
   );

}


CRainFact::~CRainFact( void ) {

   // Empty destructor body
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public Methods


TraceGuiStatesOldToNew_t CRainFact::SayGuiStatesFromSnapshotInSet( Nzint_t snapshotSetSgi ) const {

   auto tableIter = snapshotsStateBindex_bySetSgi.find( snapshotSetSgi ); 

   if  ( tableIter == snapshotsStateBindex_bySetSgi.end() ) {
      // no match found for ID given. Return empty vector
      return TraceGuiStatesOldToNew_t();
   }

   // Else, tableIter now points at table element (a std::pair) keyed by snapshotSetId

   std::array<EGuiState, FIXED_KRONO_SNAPSHOT_SIZE> statesOldestToNewest;
 
   /* Table holds snapshots as fixed-length arrays of bindex values ordered from the newest to oldest,
      as that is order they have in the originating bindex log ( a std::deque ). But, snapshots are to
      display on GUI as time-series GuiFpn_t values ordered oldest (leftmost) to newest (rightmost).
      So...
   */
   auto forwardReadingCiter = (*tableIter).second.cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter < statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_FACTBINS[ *forwardReadingCiter ];
      std::advance( forwardReadingCiter, 1 );
   }
   // nameless return of type, so compiler invokes move semantics (?)
   return TraceGuiStatesOldToNew_t( statesOldestToNewest.data(),
                                    statesOldestToNewest.data() +
                                    (sizeof( statesOldestToNewest ) / sizeof( EGuiState )) );
}

//======================================================================================================/

TraceGuiStatesOldToNew_t CRainFact::SayGuiStatesFromBindexLog( void ) {
/*
   if ( numCyclesWanted > numCyclesLogging ) {

      if ( numCyclesWanted > FIXED_DATALOG_SIZECYCLES_MAX ) { return TraceGuiStatesOldToNew_t(0); }
      statesLoggedAsBindex.resize( numCyclesWanted, BINDEX_FACT_UNAVAIL );
   }
*/
   TraceGuiStatesOldToNew_t statesOldestToNewest( FIXED_KRONO_SNAPSHOT_SIZE, EGuiState::Undefined );
  
   /* state log holds realtime series of states ordered from the newest to oldest.
      But, realtime series display on GUI ordered oldest (left) to newest (right).
   */
   auto forwardReadingCiter = statesLoggedAsBindex.cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter < statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_FACTBINS[ *forwardReadingCiter ];

// $$$ w/o next line sure crash if length of krono > of bindexLog - TBD refactor to make impossible $$$
      if ( forwardReadingCiter != statesLoggedAsBindex.cend() ) {
         std::advance(forwardReadingCiter, 1);
      }
   }
   return statesOldestToNewest;
}


//======================================================================================================/

NGuiKey CRainFact::SayHistogramKey( void ) const {

   return u_Histogram_fact->SayGuiKey();
}


EGuiState CRainFact::SayGuiStateFromNewestBindex( void ) const {

   return GUISTATES_FACTBINS[ statesLoggedAsBindex.front() ];
}


//======================================================================================================/

Bindex_t CRainFact::BindexWas_atCycles( size_t lagCycles ) const {

   return ( ( lagCycles < statesLoggedAsBindex.size() ) ?
            statesLoggedAsBindex[lagCycles] :
            NaNBINDEX );
}

//======================================================================================================/
 
void CRainFact::CaptureSnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   SnapshotBindex_t snapshotBindex;

   if ( numPastesEachCycleToSnapshot == 1 ) {         // are secs/cycle same for source and snapshot?
      for ( size_t iLog = 0; iLog < FIXED_KRONO_SNAPSHOT_SIZE; ++iLog ) {
         snapshotBindex[iLog] = statesLoggedAsBindex[iLog];
      }
   }

   // $$$ TBD to see whether this 'if-else' can be wacked, with ALL facts cycling at max rate $$$

   else {   // secs/cycle of source > that of snapshot, so source values must be "pasted" multiple times
      auto readingCiter = statesLoggedAsBindex.cbegin();   // Stepping through source bindex log...
      auto writingIter = snapshotBindex.begin();      // ...and simultaneously through target snapshot...
      while ( writingIter != snapshotBindex.end() ) {

         // ...read same value, but write into advancing indicies, until pastes are done or end is hit        
         for ( int pastesToDo = numPastesEachCycleToSnapshot; pastesToDo > 0; --pastesToDo ) {
            *writingIter = *readingCiter;
            // can pastesToDo start off > snapshot length (not sure, $$$ TBD to refactor next line $$$ ) 
            if ( writingIter != snapshotBindex.end() ) { std::advance(writingIter, 1); }
         }
         ++readingCiter;   // Rain c-tor and methods prevent bindexLog shorter than snapshot
      }
   }   
   snapshotsStateBindex_bySetSgi.emplace(
                                    std::pair<Nzint_t, SnapshotBindex_t>(  snapshotSetSgi,
                                                                           snapshotBindex )
   );
}

//======================================================================================================/

void CRainFact::Cycle(  time_t timestampNow,
                        bool beginNewClockHour,
                        bool beginNewCalendarDay,
                        bool claimNow,
                        bool isClaimNowValidAtSourceObj )  {

   lastValidClaim = ( isClaimNowValidAtSourceObj ? claimNow : lastValidClaim );

   bindexNow = (  isClaimNowValidAtSourceObj ?
                  ( claimNow ?
                     static_cast<Bindex_t>(1u) :   // Fact object claims its Statement() is now "True"
                     static_cast<Bindex_t>(0u)     // Fact object claims its Statement() is now "False"
                  ) :
                  static_cast<Bindex_t>(2u) );     // Claim of Fact object is now "Invalid"
   // Facts can also show a fourth value ( = 3, "unavailable") that is not applicable inside this method

   // Cycle Histogram Kit with data from this Rainfall cycle
   u_Histogram_fact->Cycle(   timestampNow,
                              beginNewClockHour,
                              beginNewCalendarDay,
                              bindexNow,
                              statesLoggedAsBindex[lastIndexInMovingHour] );       

   // Most recent value at lowest index ("front"); oldest value at highest index ("back")
   statesLoggedAsBindex.pop_back();
   statesLoggedAsBindex.push_front( bindexNow );

   return;
} 


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Implementations for CRainRuleKit


//======================================================================================================/
// Constructor/Destructor

CRainRuleKit::CRainRuleKit(   CRuleKit& bArg0,
                              std::vector<NGuiKey>& arg0,
                              ASubject& arg1,
                              size_t arg2 )
                              :  ARainfall(  bArg0,
                                             EApiType::Rainfall_ruleKit
                                 ),
                                 TimeAxisRef ( bArg0.SayTimeAxisRef() ),
                                 u_HistogramsForEachRuleInKit_byUai(),
                                 u_Histogram_ruleKitOverview (nullptr), // null until kit object finalized
                                 ruleKitKnobKeysRef (arg0),
                                 bindexEnteringMovingHour_byRuleUai(),
                                 bindexLeavingMovingHour_byRuleUai(),
                                 ruleFailCosts_byRuleUai(),
                                 snapshotSetSgis_byRuleUai(),
                                 ruleUais_indexedAsLogsIterate(0),
                                 ruleStatesNewest_indexedAsLogsIterate(0),
                                 ruleHasNoSnapshot_indexedAsLogsIterate(0),
                                 rulePinnedToUnitOutput_indexedAsLogsIterate(0),
                                 ruleFailedNow_anyMode_indexedAsLogsIterate(0),
                                 pinnedRuleFailedNow_anyMode_indexedAsLogsIterate(0),
                                 numRulesInKit (0),
                                 trapSpanInCycles (arg2),
                                 trapSpanInSecs ( static_cast<int>(arg2) * bArg0.SaySecsPerCycle() ),
                                 cyclesPerTrapEnable ( static_cast<int>(arg2) ),
                                 cyclesUntilTrapEnable ( static_cast<int>(arg2) ) {
}


CRainRuleKit::~CRainRuleKit( void ) {

   // Empty destructor body
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Private Methods

void CRainRuleKit::FinalizeRainfallOnTheRulesAddedToKit(
                     CRuleKit& ruleKitRef,
                     CView& viewRefFromSubj,
                     const RuleUaiToPtrTable_t& p_Rules_byUai_ref,
                     const std::vector<Nzint_t>& ruleUais_guiTopToBottom_ref ) {

   numRulesInKit = p_Rules_byUai_ref.size();  // sized here, as = zero at time of object c-tor call
   size_t logSize =  static_cast<size_t>( ( START_DATALOG_SECSLOGGING / SourceRef.SaySecsPerCycle() ) );

   for ( const auto pairValues : p_Rules_byUai_ref ) {

      bindexEnteringMovingHour_byRuleUai.emplace(
         std::pair<  Nzint_t,
                     Bindex_t >(
                        pairValues.first,
                        NaNBINDEX
         )
      );
      bindexLeavingMovingHour_byRuleUai.emplace(
         std::pair<  Nzint_t,
                     Bindex_t >(
                        pairValues.first,
                        NaNBINDEX
         )
      );
      ruleStatesLoggedAsBindex_byRuleUai.emplace(
         std::pair<  Nzint_t,
                     BindexLog_t >(
                        pairValues.first,
                        BindexLog_t( logSize, BINDEX_RULE_UNAVAIL )
         )
      );
      ruleFailCosts_byRuleUai.insert(
         std::pair<  Nzint_t,
                     int >(
                        pairValues.first,
                        pairValues.second->SayDollarPerDayFaultCost( SourceRef.EnergyPriceRef )
        )
      );
      /* Each rule ID gets one entry in a table holding IDs of snapshot sets.  Snapshot set ID = 0
         when rule has no snapshot set outstanding.
      */
      snapshotSetSgis_byRuleUai.insert(
         std::pair<  Nzint_t,
                     size_t   >(
                        pairValues.first,
                        0ull
         )
      );

    }  // close for-loop iterating rule UAIs

   for ( const auto& logTableCiter : ruleStatesLoggedAsBindex_byRuleUai ) {

      // Order of iteration over a container type stays unchanged per Class Note [1] in .hpp

      Nzint_t ruleUaiIterated = logTableCiter.first;

      ruleUais_indexedAsLogsIterate.push_back( ruleUaiIterated );

      rulePinnedToUnitOutput_indexedAsLogsIterate.push_back(
         p_Rules_byUai_ref.at(ruleUaiIterated)->IsPinnedToUnitOutput()
      );

   }

   ruleStatesNewest_indexedAsLogsIterate.reserve( numRulesInKit );
   ruleHasNoSnapshot_indexedAsLogsIterate.reserve( numRulesInKit );
   ruleFailedNow_anyMode_indexedAsLogsIterate.reserve( numRulesInKit );
   pinnedRuleFailedNow_anyMode_indexedAsLogsIterate.assign( numRulesInKit, 0 );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Build ownership table of Histograms for Rules in finalized kit

   for ( auto pairByValue_rule : p_Rules_byUai_ref ) {

      std::pair<RuleHistoOwnershipTable_t::iterator, bool> ruleHistoTableVerbReply =
         u_HistogramsForEachRuleInKit_byUai.emplace(
            std::pair<Nzint_t, std::unique_ptr<CHistogramRule> >(
               pairByValue_rule.first,    // first in pair is the Rule's UAI
               std::make_unique<CHistogramRule>(   SourceRef.SaySubjectRefAsConst(),
                                                   *pairByValue_rule.second,
                                                   ruleKitRef,
                                                   pairByValue_rule.second->SayCrefToKnobKeys(),
                                                   movingHourSpanInCycles
               )
            )
         );
   }

   u_Histogram_ruleKitOverview =   std::make_unique<CHistogramRuleKit>(
                                    ruleKitRef.SaySubjectRefAsConst(),
                                    ruleKitRef,
                                    ruleKitKnobKeysRef,
                                    movingHourSpanInCycles,
                                    ruleUais_guiTopToBottom_ref
                                 );

   return;
}

//======================================================================================================/

void CRainRuleKit::SaveRuleSnapshotUnderSetSgi( Nzint_t ruleUai,
                                                Nzint_t snapshotSetSgi ) {

   SnapshotBindex_t snapshotBindex;

   if ( numPastesEachCycleToSnapshot == 1 ) {        // are secs/cycle same for source and snapshot?
      for ( size_t iLog = 0; iLog < FIXED_KRONO_SNAPSHOT_SIZE; ++iLog ) {
         snapshotBindex[iLog] = ruleStatesLoggedAsBindex_byRuleUai[ruleUai][iLog];
      }
   }
   else {  // secs/cycle of source > that of snapshot, so source values must be "pasted" multiple times
      auto readingCiter =
         ruleStatesLoggedAsBindex_byRuleUai[ruleUai].cbegin();  // Stepping through src bindex log..
      auto writingIter = snapshotBindex.begin();      // ...and simultaneously through target snapshot..
      while ( writingIter != snapshotBindex.end() ) {

         // ...read same value, but write into advancing indicies, until pastes are done or end is hit        
         for ( int pastesToDo = numPastesEachCycleToSnapshot; pastesToDo > 0; --pastesToDo ) {
            *writingIter = *readingCiter;
            // can pastesToDo start off > snapshot length (not sure, $$$ TBD to refactor next line $$$ ) 
            if ( writingIter != snapshotBindex.end() ) { std::advance(writingIter, 1); }
         }
         ++readingCiter;   // Rain c-tor and methods prevent bindexLog shorter than snapshot
      }
   }
   snapshotSetSgis_byRuleUai[ruleUai] = snapshotSetSgi;   
   snapshotsStateBindex_bySetSgi.emplace(
                                 std::pair<Nzint_t, SnapshotBindex_t>(  snapshotSetSgi,
                                                                        snapshotBindex )
   );
   return;
}


//======================================================================================================/

void CRainRuleKit:: WriteFailsOnRulesToRainfall( const RuleUaiToPtrTable_t& p_RulesByUai ) {

   /* Assigns to each "column" of rainfall bins a CRule object held by kit, and from the bindex logs,
      writes into each bin of that column false/true value of whether data failed the given rule on
      the cycle each bin "vertically" represents.  That action is iterated back ("down") to trapDepth.
   */
   auto citerSteppingLogByLog = ruleStatesLoggedAsBindex_byRuleUai.cbegin();
      // a "citer" is a const iterator ("iter")

   std::deque<Bindex_t>::const_iterator citerSteppingCycleByCycle;
   size_t iColumnWritingTo = 0u;
   Nzint_t ruleUaiOfLogRead = 0u;
   Bindex_t bindexRead = NaNBINDEX;
   bool readLogWriteRain = NaNBOOL;
   bool ruleInAutoMode = NaNBOOL;

   // clear these every call, so any change in source data is effected via a reload
   ruleHasNoSnapshot_indexedAsLogsIterate.clear(); 

   while ( citerSteppingLogByLog != ruleStatesLoggedAsBindex_byRuleUai.end() ) {
       
      ruleUaiOfLogRead = (*citerSteppingLogByLog).first;
      ruleInAutoMode = p_RulesByUai.at(ruleUaiOfLogRead)->IsInAutoMode();

      citerSteppingCycleByCycle = (*citerSteppingLogByLog).second.cbegin();

      for ( size_t iRowWritingTo = 0u;
            iRowWritingTo < trapSpanInCycles;  // rainRow index @ "back" of trap = (cyclesTrapSpan-1)
            ++iRowWritingTo ) {

          bindexRead = *citerSteppingCycleByCycle; // last line in FOR-loop steps this back thru "time"

         // For rainfall register as utilized by CRuleKit, [row][column] is [time step][rule log]
         rainfallRegister[iRowWritingTo][iColumnWritingTo] =
            (  (  ( bindexRead == static_cast<Bindex_t>(0) ) && ruleInAutoMode ) ?
                     true :
                     false ); // reads NaNs okay as ='false'

         // "Advancing" thru a bindex log is actually stepping back thru cycles (older values)
         // trapSpanInCycles is elsewhere ensured to be <= fixed size of static rainfall register
         std::advance( citerSteppingCycleByCycle, 1 );
      } 
      // For const use of container, must call at(), since operator[] is not read-only
      // Following is vector of ints having Boolean meaning       
      ruleHasNoSnapshot_indexedAsLogsIterate.push_back(
         ( (p_RulesByUai.at(ruleUaiOfLogRead)->SaySnapshotSetSgi() == 0u) ? 1 : 0 ) );

     //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/ 
      std::advance(citerSteppingLogByLog, 1);   // advance citer to next log
      ++iColumnWritingTo;
   }   
   return;  // Results of all Logs in Rule Kit loaded into Rainfall, ready for trap action
}

//======================================================================================================/

RuleTrapResult_t CRainRuleKit::EnableTrapAndSayResult( const RuleUaiToPtrTable_t& p_RulesById ) {

   // Zero value = "null", no rule met stated condition (e.g., no rule returned "0" within trap depth)
   Nzint_t uaiOfAutoModeRuleHavingMostFailsInTrapSpan = 0u;
   Nzint_t uaiOfAutoModeRuleToDiscardItsOldSnapshots = 0u;

   // side-effect of following call is to refresh 'ruleLoggedHasNoSnapshot'
   WriteFailsOnRulesToRainfall( p_RulesById );

   // See Method Note [1]
   std::vector<BinSum_t>      sumsOfFails_indexedAsLogsIterate( p_RulesById.size(), 0 ); 
   BinSum_t                   sumSumsOfFails = 0;

   for ( size_t cyclesBack=0u;
         cyclesBack < trapSpanInCycles;
         ++cyclesBack ) {

      for ( size_t iLogReadFrom = 0;
            iLogReadFrom < sumsOfFails_indexedAsLogsIterate.size(); // i.e., number of rules in kit
            ++iLogReadFrom ) {

         // For rainfall register as utilized by CRuleKit, [row][column] is [time step][rule log]      
         if ( rainfallRegister[cyclesBack][iLogReadFrom] == true ) {
            ++sumsOfFails_indexedAsLogsIterate[iLogReadFrom];
         } 
      } // close on each rule in kit (rainfall "column")
   } // close on each cycle spanned by trap (rainfall "row")

   // sum fails to signal whether trap is empty or not
   sumSumsOfFails = std::accumulate(   sumsOfFails_indexedAsLogsIterate.begin(),
                                       sumsOfFails_indexedAsLogsIterate.end(),
                                       0 );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Check trap and act if trap is not found empty

   if ( sumSumsOfFails > 0 ) {    // trap not empty?

//------------------------------------------------------------------------------------------------------/
// Action to find UAI of Rule (having no prior snapshot) that failed most often over the trap length
 
      Nzint_t uaiOfRuleIndicatedByMark = 0u;

      // See Method Note [1]
      size_t indiciesCount_datumToMark =
         std::distance(
            sumsOfFails_indexedAsLogsIterate.begin(),                   // Put "datum" at "begin"
            std::max_element( sumsOfFails_indexedAsLogsIterate.begin(), // Put "mark" at max element...
                              sumsOfFails_indexedAsLogsIterate.end()    // ...given full rng of vector
            )
         );

      auto parallelIter = ruleUais_indexedAsLogsIterate.begin();  // Init parallel iterator to get UAI

      std::advance( parallelIter, indiciesCount_datumToMark );    // Slide parallel iterator to "mark"

      uaiOfAutoModeRuleHavingMostFailsInTrapSpan = *parallelIter; // UAI of the "trapped rule"

      std::transform(   ruleHasNoSnapshot_indexedAsLogsIterate.begin(),    // See Method Note [2]
                        ruleHasNoSnapshot_indexedAsLogsIterate.end(),
                        sumsOfFails_indexedAsLogsIterate.begin(),
                        sumsOfFails_indexedAsLogsIterate.begin(),
                        std::plus<BinSum_t>() );

//------------------------------------------------------------------------------------------------------/
// Action to find UAI of Rule (regardless of any snapshot) that failed least often over the trap length

      indiciesCount_datumToMark =
         std::distance(
            sumsOfFails_indexedAsLogsIterate.begin(),
            std::min_element( sumsOfFails_indexedAsLogsIterate.begin(),   // Put "mark" at min element
                              sumsOfFails_indexedAsLogsIterate.end()
            )
         );

      parallelIter = ruleUais_indexedAsLogsIterate.begin();    // Reset parallel iterator to get UAI

      std::advance( parallelIter, indiciesCount_datumToMark ); // Slide parallel iterator to "mark"

      uaiOfRuleIndicatedByMark = *parallelIter;                // UAI of rule w/ least fails over trap

      // Rule to get its snapshots wiped, Method Note [3] 
      uaiOfAutoModeRuleToDiscardItsOldSnapshots =
         (  (uaiOfRuleIndicatedByMark != uaiOfAutoModeRuleHavingMostFailsInTrapSpan) ?
            uaiOfRuleIndicatedByMark :
            0u );

// End trap action
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      
   }
   return SRuleTrapResult( uaiOfAutoModeRuleHavingMostFailsInTrapSpan,
                           uaiOfAutoModeRuleToDiscardItsOldSnapshots
   );

/*
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
Method Notes

[1]   Per S.O., typically faster that following entities are local vs. object members or class statics.
      Rule UAIs associated to columns in rainfallRegister (a 2-D array) are in order the p_RulesByUai
      pointer table (an unordered map) was iterated in WriteFailsOnRulesToRainfall() (i.e., an 
      "unordered" order totally up to compiler), and NOT in order rules were emplaced into p_RulesByUai
      (i.e., NOT in order CRule objects were added to the CRuleKit object.).   

[2]   This action increments the zero sumOfFails of an auto-mode rule with no fails and no snapshots to
      be sumOfFails = 1, so an auto-mode rule with zero fails inside the trapDepth, but having snapshots
      out (which means it had a fail at some point in the past, but one older than the back edge of the
      moving-in-time trap) will have its sumOfFails remain = 0, setting it up to be found by calling
      std::min_element() across the (transformed) sums.

[3]   The auto-mode rule having least number of 'fails' within trapDepth gets its snapshots wiped out
      iff it is not also the auto-mode rule with the most fails.
      This action has two benefits: (1) helps avoid potential memory leak of creating snapshots with
      no way snapshots are eliminated other than resolution/deletion of CCase objects, and (2) keeps
      the snapshot of a 'fail' on a rule more timely to whenever that rule finally "falls in the trap"
      (i.e., finally becomes the auto-mode rule with the most 'fails') and initiates a CCase object.
*/

}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Public Methods

TraceGuiStatesOldToNew_t CRainRuleKit::SayGuiStatesFromSnapshotInSet( Nzint_t snapshotSetSgi ) const {

   auto tableIter = snapshotsStateBindex_bySetSgi.find( snapshotSetSgi ); 

   if  ( tableIter == snapshotsStateBindex_bySetSgi.end() ) {
      // no match found for ID given. Return empty vector
      return TraceGuiStatesOldToNew_t();
   }
   // Else, tableIter now points at table element (a std::pair) keyed by snapshotSetId

   std::array<EGuiState, FIXED_KRONO_SNAPSHOT_SIZE> statesOldestToNewest;
 
   /* Table holds snapshots as fixed-length arrays of bindex values ordered from the newest to oldest,
      as that is order they have in the originating bindex log ( a std::deque ). But, snapshots are to
      display on GUI as time-series GuiFpn_t values ordered oldest (leftmost) to newest (rightmost).
      So...
   */
   auto forwardReadingCiter = (*tableIter).second.cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter < statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_RULEBINS[ *forwardReadingCiter ];
      std::advance( forwardReadingCiter, 1 );
   }
   // nameless return of type, so compiler invokes move semantics (?)
   return TraceGuiStatesOldToNew_t( statesOldestToNewest.data(),
                                    statesOldestToNewest.data() +
                                    (sizeof( statesOldestToNewest ) / sizeof( EGuiState )) );
}

//======================================================================================================/

TraceGuiStatesOldToNew_t
CRainRuleKit::SayGuiStatesFromBindexLogUnderRuleUai( Nzint_t ruleUai ) {

   /* $$$ c-tor of CKrono precludes numCyclesInKrono > FIXED_RAIN_CYCLESLOGGING_MAX. Only guard against
      oversizing logs is having only Kronos call this method, using their size   $$$ TBD to refactor
   */

   TraceGuiStatesOldToNew_t statesOldestToNewest( FIXED_KRONO_SNAPSHOT_SIZE, EGuiState::Undefined );

   // Must backfill any extention w/ interim "bum" values that are PLOTTABLE, so use oldest value held
/*   if ( numCyclesInKrono > ruleStatesLoggedAsBindexByRuleUai[ruleUai].size() ) {

      ruleStatesLoggedAsBindexByRuleUai[ruleUai].resize( numCyclesInKrono, BINDEX_RULE_UNAVAIL );
   }
*/  
   /* bindexLog holds realtime series of bindex values ordered from the newest to oldest.
      But, realtime series display on GUI as GuiFpn_t values ordered oldest (left) to newest (right).
   */
   auto forwardReadingCiter = ruleStatesLoggedAsBindex_byRuleUai[ruleUai].cbegin();
   for ( auto  reverseWritingIter = statesOldestToNewest.rbegin();
               reverseWritingIter != statesOldestToNewest.rend();
               ++reverseWritingIter ) {

      *reverseWritingIter = GUISTATES_RULEBINS[ *forwardReadingCiter ];

// $$$ w/o next line sure crash if length of krono > of bindexLog - TBD refactor to make impossible $$$
      if ( forwardReadingCiter != ruleStatesLoggedAsBindex_byRuleUai[ruleUai].cend() ) {
         std::advance(forwardReadingCiter, 1);
      }
   }
   return statesOldestToNewest;
}

//======================================================================================================/

std::vector<EGuiState>
CRainRuleKit::SayNewestRuleStates_GuiTopToBottom( const std::vector<Nzint_t>& uaiRef_inGuiOrder ) const {

   std::vector<EGuiState> reply(0);

   for ( auto uai : uaiRef_inGuiOrder ) {

      reply.push_back( GUISTATES_RULEBINS[  bindexEnteringMovingHour_byRuleUai.at(uai) ] );
   }
   return reply;    // For speed, banking on compiler NRVO of a STL container class (std::vector)
}

//======================================================================================================/

NGuiKey CRainRuleKit::SayKeyToOverviewHistogram( void ) const {

   return u_Histogram_ruleKitOverview->SayGuiKey();
}

//======================================================================================================/

NGuiKey CRainRuleKit::SayKeyToHistogramOfRule( Nzint_t ruleUai ) const {

   RuleHistoOwnershipTable_t::const_iterator pairCiter =
      u_HistogramsForEachRuleInKit_byUai.find( ruleUai );

   return   (  ( pairCiter  == u_HistogramsForEachRuleInKit_byUai.end() ) ?
               NGuiKey(0) :
               pairCiter->second->SayGuiKey()
   ); 
}


//======================================================================================================/

EGuiState CRainRuleKit::SayGuiStateFromNewestBindexUnderRuleUai( Nzint_t ruleUai ) const {

   return
      GUISTATES_RULEBINS[ ruleStatesLoggedAsBindex_byRuleUai.at( ruleUai ).at(0) ];
} 
 

//======================================================================================================/

int CRainRuleKit::GetTrapSpanInSecs( void ) const { return trapSpanInSecs; }

//======================================================================================================/


EGuiReply CRainRuleKit::SetTrapSpanInSecs( int secsSpanGiven ) {

// $$$ TBD to reform this as using cycles not secs for the range check $$$

   if ( ! ( secsSpanGiven < (static_cast<int>(FIXED_RAINFALL_SIZE) * SourceRef.SaySecsPerCycle()) ) ) {
      return EGuiReply::FAIL_set_askedStatisticBeyondDepthLimit;
   }

   trapSpanInSecs = secsSpanGiven;

   // C++ int divison floors quotient to the largest integer that fits:
   cyclesPerTrapEnable = ( trapSpanInSecs / SourceRef.SaySecsPerCycle() );
   trapSpanInCycles = static_cast<size_t>(cyclesPerTrapEnable);
   return EGuiReply::OKAY_allDone; 
}

//======================================================================================================/

std::pair<Nzint_t,bool> CRainRuleKit::CycleRulesInKitAndSayResults(  time_t timestampNow,
                                                                     bool beginNewClockHour,
                                                                     bool beginNewCalendarDay,
                                                                     RuleUaiToPtrTable_t&
                                                                     lookupRulesHeld )  {

// Default UAI is 0 -> cycle complete with no rule "trapped", whether trap enabled or not
   Nzint_t uaiOfTrappedRule = 0u;
   Nzint_t snapshotSetSgi = 0u;
   Nzint_t ruleUai = 0u;
   Bindex_t ruleResult = NaNBINDEX;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// First, cycle pop/push of a new rule result through bindex logs of each rule held in kit

   ruleStatesNewest_indexedAsLogsIterate.clear();
   ruleFailedNow_anyMode_indexedAsLogsIterate.clear();

   auto ruleTableIter = lookupRulesHeld.begin();               // non-const due to cycle call
   while ( ruleTableIter != lookupRulesHeld.end() ) {

      ruleUai = (*ruleTableIter).first;      
      ruleResult = (*ruleTableIter).second->CycleAndSayBindex(); // Bindex convolves ea. rule's validity

      // *PRIOR* to pop/push of bindex log, write data from it into LUT so to pass to histograms
      bindexEnteringMovingHour_byRuleUai[ruleUai] = ruleResult;
      bindexLeavingMovingHour_byRuleUai[ruleUai] =
         ruleStatesLoggedAsBindex_byRuleUai[ruleUai][lastIndexInMovingHour];

      // Log has most recent result at lowest index ("front"); oldest result at highest index ("back")
      ruleStatesLoggedAsBindex_byRuleUai[ruleUai].pop_back();
      ruleStatesLoggedAsBindex_byRuleUai[ruleUai].push_front( ruleResult );

      if (  ( ruleResult == BINDEX_RULE_AUTOMODEFAIL ) &&
            ( ! (*ruleTableIter).second->HasSnapshotSet() ) ) { // no snapshot already?

         snapshotSetSgi = (*ruleTableIter).second->SaveAntecedentSnapshotsAndSaySetSgi();
         TimeAxisRef.CreateSnapshotForSetSgi( snapshotSetSgi ); 
         SaveRuleSnapshotUnderSetSgi( ruleUai, snapshotSetSgi );
      }

      ruleStatesNewest_indexedAsLogsIterate.push_back( GUISTATES_RULEBINS[ruleResult] );

      ruleFailedNow_anyMode_indexedAsLogsIterate.push_back( FAILEDINANYMODE_RULEBINS[ruleResult] );

      std::advance(ruleTableIter,1);  // adv iterator to next rule held in kit (lookup table)

   }  // Exit while-loop that iterates rule lookup table

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Check for hierarchical effects

   std::transform(   ruleFailedNow_anyMode_indexedAsLogsIterate.begin(),
                     ruleFailedNow_anyMode_indexedAsLogsIterate.end(),
                     rulePinnedToUnitOutput_indexedAsLogsIterate.begin(),
                     pinnedRuleFailedNow_anyMode_indexedAsLogsIterate.begin(),
                     std::multiplies<int>() );

   bool rulePinnedToUnitOutputFailedThisCycle =
      (  std::accumulate(  pinnedRuleFailedNow_anyMode_indexedAsLogsIterate.begin(),
                           pinnedRuleFailedNow_anyMode_indexedAsLogsIterate.end(),
                           0
                        ) == 0 ? false : true
      );

   pinnedRuleFailedNow_anyMode_indexedAsLogsIterate.assign( numRulesInKit, 0 );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Update histograms

   u_Histogram_ruleKitOverview->Cycle( timestampNow,
                                       beginNewClockHour,
                                       beginNewCalendarDay,
                                       bindexEnteringMovingHour_byRuleUai, // See Method Note [1]
                                       bindexLeavingMovingHour_byRuleUai );

   for ( auto& pairByRef_ruleHisto : u_HistogramsForEachRuleInKit_byUai ) {

      pairByRef_ruleHisto.second->Cycle(  timestampNow,
                                          beginNewClockHour,
                                          beginNewCalendarDay,
                                          bindexEnteringMovingHour_byRuleUai[pairByRef_ruleHisto.first],
                                          bindexLeavingMovingHour_byRuleUai[pairByRef_ruleHisto.first]
      );
   }
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Next, is this cycle to also enable trap action?

   --cyclesUntilTrapEnable;   // side-effect of EVERY call: decrement counter that enables trap  

   if ( cyclesUntilTrapEnable < 1 ) { 

      RuleTrapResult_t trapResult = EnableTrapAndSayResult( lookupRulesHeld ); // Do it!

      // Did trap return a snapshot set to delete? [yes or no is up to CycleTrapAndSayResult()]
      if ( trapResult.uaiOfRuleToWipeOfSnapshots != 0u ) {  // == 0 means "no"

         // this call actually deletes snapshots only if the rule has no CCase outstanding on it
         EApiReply ruleReply =
            lookupRulesHeld[trapResult.uaiOfRuleToWipeOfSnapshots]->DestroySnapshotSet();

         if ( ruleReply == EApiReply::Okay_tallyOne) {
            TimeAxisRef.DestroySnapshotForSetSgi(
               lookupRulesHeld[trapResult.uaiOfRuleToWipeOfSnapshots]->SaySnapshotSetSgi() );
         } 
      }
      // Pass either zero or ID of trapped rule.  For non-zero, kit will mask and create a new CCase
      uaiOfTrappedRule = trapResult.uaiOfRuleTrappedForNewCase;
      cyclesUntilTrapEnable = cyclesPerTrapEnable;    // reset counter
   }

   return std::pair<Nzint_t,bool>(  uaiOfTrappedRule, // if non-0, leaves CCase creation to Rule Kit
                                    rulePinnedToUnitOutputFailedThisCycle
   );

/* START Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Histograms are owned by Rainfalls versus being owned by source objects (i.e., ISeqElement subclass
      obj) because only a rainfall Cycle() method has readily (i.e., locally) all the data a Histogram
      cycle requires.

'' END Method Notes ''''' */
}


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
