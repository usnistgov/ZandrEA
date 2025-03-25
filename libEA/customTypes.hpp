// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare custom types used within the "back end" of an instance of the EA software (an "Application")
   These types are related to AFDD surveillance.  File diagnosticTypes.hpp has diagnostics-related types
   File exportTypes.hpp has custom types used by EA "front end" when calling "back end" via Runtime API.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef CUSTOMTYPES_HPP
#define CUSTOMTYPES_HPP

#include <memory>
#include <limits>
#include <array>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <unordered_map>

#include <string>       // ***** Needed only for ECHO TESTING during development *****
#include <iostream>     // ***** Needed only for ECHO TESTING during development *****

// only way exportTypes.hpp gets included anywhere else is thru this file, so declspec is set properly
#include "exportTypes.hpp"

//======================================================================================================/
// Primary P.O.D. typedefs

typedef unsigned int Nzint_t;       // See File Note [2]
typedef unsigned char Bindex_t;     // analog or state information mapped as an index to a Rainfall bin

//======================================================================================================/
// General constants used throughout project (change to constexpr upon upgade to VS2015 )

const float             PI_F        = 3.14159265358979f;

const Nzint_t           BASETRIGGRP_POINT = 1;
const Nzint_t           BASETRIGGRP_FORMULA = 2;
const Nzint_t           BASETRIGGRP_CHART = 3;
const Nzint_t           BASETRIGGRP_FACT = 4;
const Nzint_t           BASETRIGGRP_RULEKIT = 99;

const double            NaNDBL      = std::numeric_limits<double>::quiet_NaN();
const float             NaNFLOAT    = std::numeric_limits<float>::quiet_NaN();
const bool              NaNBOOL     = std::numeric_limits<bool>::quiet_NaN();
const char              NaNCHAR     = std::numeric_limits<char>::quiet_NaN();
const int               NaNINT      = std::numeric_limits<int>::quiet_NaN();
const unsigned int      NaNUINT     = std::numeric_limits<unsigned int>::quiet_NaN();
const size_t            NaNSIZE     = std::numeric_limits<size_t>::quiet_NaN();
const Nzint_t           NaNNZINT     = std::numeric_limits<Nzint_t>::quiet_NaN();
const Bindex_t          NaNBINDEX   = std::numeric_limits<Bindex_t>::quiet_NaN();
// for Nzint_t, zero "0" acts as null, NaN value

//======================================================================================================/
// PRINCIPAL arbitrary Parameters

/* Explanatory note:
   Upon each "bell", "Agent" (MVC Controller) "triggers" one round of its "Sequence" (MVC Model,
   e.g., the agent's "task").  So, bell period (in seconds) = trigger period. ISeqElement objects (more
   of the Model) receiving trigger will "cycle" (execute) or ignore that trigger, depending upon each
   object's constant "tpc" (triggers/cycle) value.  Thus, various analysis/rule rates are accommodated,
   with the trigger (i.e. bell) rate (in sec^-1) being the fastest rate possible for the application. 
*/

// Note: 3600 sec/hr; 86400 secs/day; 1440 mins/day; 604800 secs/week; 31,449,600 secs/year

const int      FIXED_CLOCK_SECSPERBELL = 60; // NOTE: ALL other const "secs" = int multiples of this one
const int      FIXED_SEQUENCE_SECSPERTRIGGER = FIXED_CLOCK_SECSPERBELL; // Agent triggers seq upon bell
               // Thus:  8640 triggers/day @ 10s/trigger; 1440 triggers/day @ 60s/trigger
const int      FIXED_CLOCK_TRIGGERCOUNTER_RESET = 14400; // CSV file length max = 10 days at 60s/trigger 

/* Current design has one, statically allocated 2-D rainfall array made available to all objects of the
   concrete subclasses of ARainfall. Data logging length aboard those obj can exceed actual rainfall
   array size.  The writing-in process to the analytical activities the rainfall enables is bounded by
   the rainfall array itself, not length of the various logs written into it.  Data log can exceed
   rainfall array size to provide longer-span realtime Traces to the GUI (via a Krono object).
*/  
const size_t   FIXED_RAINFALL_SIZE = 360u;      // = 1 hr. at 10s/trigger, = 6 hrs. at 60s/trigger

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Parameters for data logging
 
const size_t   FIXED_DATALOG_SIZECYCLES_MAX = 1440;   // arbitrary (so logs <= 24 hours @ 60 secs/cycle)

const int      FIXED_DATALOG_SECSLOGGING_MIN = 3600;  // need logs at least 1 hour for histograms
const size_t   FIXED_DATALOG_SIZECYCLES_MIN =   FIXED_DATALOG_SECSLOGGING_MIN /
                                                   FIXED_SEQUENCE_SECSPERTRIGGER;

const int      FIXED_DATALOG_SECSLOGGING_MAX =  FIXED_DATALOG_SIZECYCLES_MAX *
                                                   FIXED_SEQUENCE_SECSPERTRIGGER;

const size_t   START_DATALOG_SIZECYCLES = FIXED_DATALOG_SIZECYCLES_MIN;
const int      START_DATALOG_SECSLOGGING = START_DATALOG_SIZECYCLES * FIXED_SEQUENCE_SECSPERTRIGGER;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Parameters for charting

const size_t   START_SHEWCHART_CYCLESUSING = 3;          // must be >= 1, $$$ TBD chg to seconds $$$
const int      START_TRACKCHART_SECSBETWEENRESETS = 360; // trackers use reset interval in secs

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Parameters for hierarchical logic passed between Subjects

const size_t   FIXED_SUBJECT_CYCLESPINNEDFAILHISTORY = 3;   // 

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Parameters for diagnostic Cases

const size_t   FIXED_KRONO_SPANCYCLES_MIN = 5;        // arbitrary
const int      FIXED_KRONO_SNAPSHOT_SPANSECS = 900;   // "Snapshot" = preceding 15 minutes (arbitrary)

const size_t   FIXED_CASEKIT_NUMCASESOUT_MAX = 10u;
const size_t   FIXED_KRONO_SNAPSHOT_SIZE = static_cast<size_t>(   FIXED_KRONO_SNAPSHOT_SPANSECS /
                                                                  FIXED_SEQUENCE_SECSPERTRIGGER );
const int      FIXED_SEQELEMENT_TRIGGERSPERCYCLE_MAX =
                  static_cast<int>( FIXED_KRONO_SNAPSHOT_SIZE );     // min 1 cycle/snapshot

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Parameters defining numbers of rainfall bins used to evaluate analog data, facts, and rules

const size_t   FIXED_RAIN_ANALOGVALUE_NUMBINS = 256u;   // poss values Bindex_t = size(unsigned char)
const size_t   FIXED_RAIN_ANALOGSTATE_NUMBINS = 3u;
const size_t   FIXED_RAIN_FACTSTATE_NUMBINS = 4u;
const size_t   FIXED_RAIN_RULESTATE_NUMBINS = 11u;

const int      FIXED_RULEKIT_SECSBETWEENTRAPS_MIN = FIXED_DATALOG_SECSLOGGING_MIN;
const int      START_RULEKIT_SECSBETWEENTRAPS = FIXED_RULEKIT_SECSBETWEENTRAPS_MIN;

// rules/kit <= rainfall width
const size_t FIXED_RAIN_NUMRULESINKIT_MAX = FIXED_RAIN_ANALOGVALUE_NUMBINS;
const size_t FIXED_PANE_ANALOG_NUMTRACES_MAX = 3;
const size_t FIXED_PANE_FACT_NUMTRACES_MAX = 999;     // No limit on API end, GUI will have one
const size_t FIXED_PANE_RULE_NUMTRACES_MAX = FIXED_RAIN_NUMRULESINKIT_MAX;

//======================================================================================================/
// Object initializing values

// "Pinned" means the Rule must be passed or skipped for its Subject's unit output to be "okay" 
const bool     FIXED_RULE_UNITOUTPUT_PINNED = true;
const bool     FIXED_RULE_UNITOUTPUT_NOTPINNED = false;

const float    INIT_HVACPARAM_AHU_FRZSTAT_DEGF = 35.0f;
const float    INIT_HVACPARAM_AHU_OAFRAC_MIN = 0.30f;

const size_t   INIT_MAPBUCKETS_DOMAIN_NUMSUBJECTS = 10;
const size_t   INIT_MAPBUCKETS_DOMAIN_NUMOUTPUTSPERSUBJ = 4;

const std::array<int,3> INIT_KNOB_MINDEFMAX_DATALOG_SECSLOGGING = { FIXED_DATALOG_SECSLOGGING_MIN, START_DATALOG_SECSLOGGING, FIXED_DATALOG_SECSLOGGING_MAX };

// Shewhart passband half-width, in number of std. devs off data mean
const std::array<float,3> INIT_MINDEFMAX_SHEWHART_ZPASS = {0.0f, 3.0f, 5.0f};

const std::array<int,3> INIT_MINDEFMAX_FACTSUSTAINED_MINCYCLES = {1, 90, 360};

// Bilateral stubbornness (in successive chart apps with no trip) for chart to flip
// between "isSteady" being "true" or "false"
const std::array<int,3> INIT_MINDEFMAX_SHEWHART_TRIPFREEMARGIN = {0, 3, 5};

// An Apply() call ("APP") of chart happens every cycle unless "invalid" is passed from source object
// (e.g., 5 APPS @ 60s/cycle means 5 minutes between resets
const std::array<int,3> INIT_MINDEFMAX_TRACKING_APPSBETWEENRESETS = {0, 5, 10};

// Fraction of -WARN that sum in register N will cancel register P = +WARN being a called a "rise" -and-
// fraction of +WARN that sum in register P will cancel register N = -WARN being a called a "fall"
const std::array<float,3> INIT_MINDEFMAX_TRACKING_LAGFRAC = {0.20f, 0.50f, 1.0f}; 

// fraction in P and N registers allowed to "stale off" when not increased during a cycle
const std::array<float,3> INIT_MINDEFMAX_TRACKING_STALEFRAC = {0.20f, 0.50f, 1.0f};

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Min is zero, max is given
   If experience proves separate min/max needed for any of the params below, then revise c-tor of
   relevant classes to initialize min and max param values in addition to the "now" (current) value.
   and add MIN and MAX constants below as done above. 
*/
const float FIXED_PARAM_ZERO = 0.0f;
const float FIXED_PARAM_FRACTION_SHUT = 0.0f;
const float FIXED_PARAM_FRACTION_FULL = 1.0f;
const float FIXED_PARAM_PERCENT_SHUT = 0.0f;
const float FIXED_PARAM_PERCENT_FULL = 100.0f;

const std::array<float,3> INIT_MINDEFMAX_RELATE_HYSTER_ANYFRACTION = {0.0f, 0.0f, 0.1f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT =  {0.0f, 0.0f, 10.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_HYSTER_CFM_0TO3K =   {0.0f, 0.0f, 150.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120 = {0.0f, 0.0f, 4.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_HYSTER_IWG_0TO3 =    {0.0f, 0.0f, 0.1f};


const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_ANYFRACTION =        {-0.1f, 0.0f, 0.1f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT  =        {-10.0f, 0.0f, 10.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_CFM_0TO3K =          {-150.0f, 75.0f, 150.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY =   {-4.0f, 2.0f, 4.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_ZERO =   {-4.0f, 0.0f, 4.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_HARD =   {-4.0f, -2.0f, 4.0f};
const std::array<float,3> INIT_MINDEFMAX_RELATE_SLACK_IWG_0TO3 =           {-0.1f, 0.0f, 0.1f};

// "Tracking" = summing into P and N registers data beyond +/- halfband around a "guide" variable,
// which can be same as "observed" variable (i.e., can be autoregressive)

const std::array<float,3> INIT_MINDEFMAX_TRACKING_HALFBAND_ANYFRACTION =   {0.01f, 0.02f, 0.03f};
const std::array<float,3> INIT_MINDEFMAX_TRACKING_HALFBAND_ANYPERCENT =    {1.0f, 2.0f, 3.0f};
const std::array<float,3> INIT_MINDEFMAX_TRACKING_HALFBAND_CFM_0TO3K =     {10.0f, 50.0f, 90.0f};
const std::array<float,3> INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120 =   {0.5f, 0.5f, 3.0f};
const std::array<float,3> INIT_MINDEFMAX_TRACKING_HALFBAND_IWG_0TO3 =      {0.1f, 0.3f, 0.6f};

// warn = (units-of-x)-minutes beyond halfband, summed over the reset period set by APPSBETWEENRESETS
  // i.e., number of (/1.0)-mins summed over reset
const std::array<float,3> INIT_MINDEFMAX_TRACKING_WARN_ANYFRACTION = {0.0f, 1.0f, 5.0f};
const std::array<float,3> INIT_MINDEFMAX_TRACKING_WARN_ANYPERCENT = {0.0f, 100.0f, 500.0f};

// number of cfm-minutes summed over reset
const std::array<float,3> INIT_MINDEFMAX_TRACKING_WARN_CFM_0TO3K = {0.0f, 300.0f, 1000.0f};

// number of degF-minutes summed over reset
const std::array<float,3> INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120 = {0.0f, 5.0f, 30.0f};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Constants and types related to binning of input data into a "Rainfall"

/*
Type accumulating a "bin sum" down (i.e., column-wise) all rows of rainfall
Also for holding the sum across all bin sums (a "sum bin sums" = full count)
since arithmetic is also done in this type, better to use signed vs. unsigned
*/
typedef int BinSum_t;
const BinSum_t FIXED_RULERAIN_FAILSINEMPTYTRAP_MAX = 5; // Max Rule fails in a trap considered "empty"

// Each "bin row" of Boolean bins represents one "cycle" ("sample", "time step") within rainfall
// Analog use is "widest" use, so it fixes width of rainfall. Other uses read/write rows of fewer bins

typedef std::array<BinSum_t,FIXED_RAIN_ANALOGVALUE_NUMBINS >   BinSumsAnalogValue_t;
typedef std::array<BinSum_t,FIXED_RAIN_ANALOGSTATE_NUMBINS>    BinSumsAnalogState_t;
typedef std::array<BinSum_t,FIXED_RAIN_FACTSTATE_NUMBINS>      BinSumsFactState_t;
typedef std::array<BinSum_t,FIXED_RAIN_RULESTATE_NUMBINS>      BinSumsRuleState_t;


const std::array<EGuiState, FIXED_RAIN_ANALOGSTATE_NUMBINS>
   GUISTATES_ANALOGSTATEBINS = { {  EGuiState::Analog_invalid,
                                    EGuiState::Analog_valid,
                                    EGuiState::Analog_unavailable } };

const std::array<std::string, FIXED_RAIN_ANALOGSTATE_NUMBINS>
   GUILABELS_ANALOGSTATEBINS = { {  "Invalid",
                                    "Valid",
                                    "Unavail" } };

const std::array<EGuiState, FIXED_RAIN_FACTSTATE_NUMBINS>
   GUISTATES_FACTBINS =   { { EGuiState::Fact_false,
                              EGuiState::Fact_true,
                              EGuiState::Fact_invalid,
                              EGuiState::Fact_unavailable } };

const std::array<std::string, FIXED_RAIN_FACTSTATE_NUMBINS>
   GUILABELS_FACTBINS =   { { "False",
                              "True",
                              "Invalid",
                              "Unavail" } };

const std::array<EGuiState, FIXED_RAIN_RULESTATE_NUMBINS>
   GUISTATES_RULEBINS = { {   EGuiState::Rule_autoMode_fail,
                              EGuiState::Rule_autoMode_pass,
                              EGuiState::Rule_autoMode_skip,
                              EGuiState::Rule_invalid,
                              EGuiState::Rule_caseMode_fail,
                              EGuiState::Rule_caseMode_pass,
                              EGuiState::Rule_caseMode_skip,
                              EGuiState::Rule_idleMode_fail,
                              EGuiState::Rule_idleMode_pass,
                              EGuiState::Rule_idleMode_skip,
                              EGuiState::Rule_unavailable } };

const std::array<bool, FIXED_RAIN_RULESTATE_NUMBINS>
   FAILEDINANYMODE_RULEBINS = { {   true,
                                    false,
                                    false,
                                    false,
                                    true,
                                    false,
                                    false,
                                    true,
                                    false,
                                    false,
                                    false } };

const std::array<std::string, FIXED_RAIN_RULESTATE_NUMBINS>
   GUILABELS_RULEBINS = { {   "Fa",
                              "Pa",
                              "Sa",
                              "X",
                              "Fc",
                              "Pc",
                              "Sc",
                              "Fi",
                              "Pi",
                              "Si",
                              "U" } };

// Needed for std::find on "validity over cycles" checks by charts:
const Bindex_t BINDEX_ANALOGSTATE_INVALID = static_cast<Bindex_t>(0u);  // 1st bin, as is zero-indexed
const Bindex_t BINDEX_FACT_INVALID = static_cast<Bindex_t>(2u);         // 3rd bin, as is zero-indexed

const Bindex_t BINDEX_RULE_AUTOMODEFAIL = static_cast<Bindex_t>(0u);    // 1th bin = "autoMode fail"

// Needed to backfill resized logs :
const Bindex_t BINDEX_ANALOGSTATE_UNAVAIL = static_cast<Bindex_t>(2u);     // 3rd bin, as is zero-indexed
const Bindex_t BINDEX_ANALOGVALUE_UNAVAIL = static_cast<Bindex_t>(127u);   // arbitrarily mid of 256 bins
const Bindex_t BINDEX_FACT_UNAVAIL = static_cast<Bindex_t>(3u);            // 4th bin = "unavail"
const Bindex_t BINDEX_RULE_UNAVAIL = static_cast<Bindex_t>(10u);           // 11th bin = "unavail"

// Needed to initialize histogram slices (hopefully to cut down on compiler warnings from implied casts):
const size_t INDEX_BINSUMS_ANALOGSTATE_UNAVAIL = static_cast<size_t>(BINDEX_ANALOGSTATE_UNAVAIL);
const size_t INDEX_BINSUMS_ANALOGVALUE_UNAVAIL = static_cast<size_t>(BINDEX_ANALOGVALUE_UNAVAIL);
const size_t INDEX_BINSUMS_FACT_UNAVAIL = static_cast<size_t>(BINDEX_FACT_UNAVAIL);
const size_t INDEX_BINSUMS_RULE_UNAVAIL = static_cast<size_t>(BINDEX_RULE_UNAVAIL);

//======================================================================================================/
// Further Typedefs and Structs

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Needed by ISeqElement.hpp and case.hpp to build traces
class CTraceSnapshot;
typedef std::unordered_map<NGuiKey, CTraceSnapshot* >    SsTraceAccessTable_t;

class CTraceRealtime;
typedef std::unordered_map<NGuiKey, CTraceRealtime* >    RtTraceAccessTable_t;

class CRule;
typedef std::unordered_map<Nzint_t, CRule*>              RuleUaiToPtrTable_t;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/      

typedef std::deque<time_t>                               TimeStampBuffer_t;

typedef std::array<time_t, FIXED_KRONO_SNAPSHOT_SIZE>    SnapshotTimeAxis_t;

typedef std::unordered_map<Nzint_t, SnapshotTimeAxis_t > SnapshotTimeAxesBank_t;

typedef std::vector<std::string>                         KitAlerts_t;
typedef std::unordered_map<char, KitAlerts_t >           KitAlertsMap_t;
typedef KitAlertsMap_t::iterator                         KitAlertsMapItr_t;

typedef std::array<Bindex_t, FIXED_KRONO_SNAPSHOT_SIZE>  SnapshotBindex_t; //newest value at lowest index
typedef std::unordered_map<Nzint_t, SnapshotBindex_t>    SnapshotBindexBank_t;

typedef std::array<EGuiState, FIXED_KRONO_SNAPSHOT_SIZE>  SnapshotState_t; //newest state at lowest index
typedef std::unordered_map<Nzint_t, SnapshotState_t>      SnapshotStatesBank_t;

typedef std::vector<GuiFpn_t>    TraceGuiNumbersOldToNew_t; // newest number at highest index. Note [3]
typedef std::vector<EGuiState>   TraceGuiStatesOldToNew_t;  // newest state at highest index. Note [3] 
typedef std::vector<time_t>      KronoTimeStampsOldToNew_t; // newest time at highest index. Note [3]

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

typedef struct SClockRead {

   time_t   timestamp;
   int      triggerCount;
   int      minutesAfterMidnight;
   bool     newHour;
   bool     newDay;

   SClockRead( time_t arg0,
               int arg1,
               bool arg2,
               bool arg3 )
               :  timestamp (arg0),
                  triggerCount (0),
                  minutesAfterMidnight (arg1),
                  newHour (arg2),
                  newDay (arg3) {
   }
} ClockRead_t;


typedef struct SEnergyPrices {

   const int perKwDayClgAir;
   const int perKwDayHtgAir;
   const int perKwDayClgWtr;
   const int perKwDayHtgWtr;
   const int perKwDayElectr;

   SEnergyPrices( int arg0,
                  int arg1,
                  int arg2,
                  int arg3,
                  int arg4 )
                  :  perKwDayClgAir (arg0),
                     perKwDayHtgAir (arg1),
                     perKwDayClgWtr (arg2),
                     perKwDayHtgWtr (arg3),
                     perKwDayElectr (arg4) {
   }

} EnergyPrices_t;


//======================================================================================================/
// Enums used either as types for class members or return types kept within API (vs. exported out to GUI)

enum struct EApiReply : unsigned char {

   Undefined = 0u,
   Fail,
   Okay_tallyOne,
   Okay_tallyZero
};


enum struct EApiType : unsigned int {

   Undefined = 0u,    // Typically only to fill field(s) in an empty view/control struct
   Case,
   Chart_shewhart,
   Chart_track_autoregressive,
   Chart_track_toDataGuide,
   Chart_track_toParamGuide,
   Domain,
   Display_ruleKit,
   Fact,
   Feature_analog,
   Feature_fact,
   Formula,
   Histogram_analog,
   Histogram_fact,
   Histogram_rule,
   Histogram_ruleKit,
   HistogramBars_analogBins,
   HistogramBars_analogStates,
   HistogramBars_factStates,
   HistogramBars_ruleStates,
   HistogramBars_rulesUai,
   Knob_Boolean,
   Knob_float,
   Knob_selectNzint,
   Knob_sint,
   Krono_realtime,
   Krono_snapshot,
   Pane_realtime_analog,
   Pane_realtime_fact,
   Pane_realtime_rule,
   Pane_snapshot_analog,
   Pane_snapshot_fact,
   Pane_snapshot_rule,
   Point_analogReadAsAnalog,
   Point_analogReadAsBinary,
   Rainfall_analog,
   Rainfall_fact,
   Rainfall_ruleKit,
   Rule,
   RuleKit,
   Subject,
   Tool,
   Trace_realtime_analog,
   Trace_realtime_fact,
   Trace_realtime_rule,
   Trace_snapshot_analog,
   Trace_snapshot_fact,
   Trace_snapshot_rule
};


enum struct EInfoMode : unsigned char {

      Undefined = 0u,
      Histo_analog_statesOverAllCycles,
      Histo_analog_valuesOverValidCycles,
      Histo_fact_statesOverAllCycles,
      Histo_rule_statesOverAllCycles,
      Histo_ruleKit_failsOverTests,
      Histo_ruleKit_testsOverValidCycles,
      Histo_ruleKit_validCyclesOverAllCycles
};

enum struct ETimeSpan : unsigned char {

      Undefined = 0u,
      Histo_movingHour,
      Histo_past24hrs,
      Histo_past7days
};


enum class ERealName : unsigned int {

/* "Real" = The physically real thing, represented via an object of class CDomain or of a concrete
   subclass of ASubject.
*/
   Undefined = 0u,
   Domain_ibal,
   Subject_chwPlant,
   Subject_hwPlant,
   Subject_ahu1,
   Subject_ahu2,
   Subject_vav1,
   Subject_vav2,
   Subject_vav3,
   Subject_vav4
};



enum struct EPlotGroup : unsigned char {

   // ISeqElement objects of similar kind are occasionally grouped to plot in same Pane

   Undefined = 0u,   // for ISeqElement obj never themselves plotted (Charts, Rule kits)
   Alone,
   Free,             // Free = put in any Pane as long as same kind and units
   GroupA,
   GroupB,
   GroupC,
};


enum struct EDataRange : unsigned char {

   Undefined = 0u,
   Analog_percent,
   Analog_zeroToOne,
   Analog_zeroTo3,
   Analog_zeroTo120,
   Analog_zeroTo3k,
   Bindex_fact,
   Bindex_rule,
   Boolean,
   None
};

//======================================================================================================/

enum struct EDataLabel : unsigned int {

  // "label" is a generic descriptor. A "name" identifies a particular thing (instance) bearing a label

   Undefined = 0u,

   // remaining is alphabetical left-to-right, "_" before "a"

   Case_alertToGui,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// facts relevant to specified subject about a condition on a physically antecedent (upstream) subject

   Fact_antecedent_vav_ahuOkay,
   Fact_antecedent_vav_hwPlantOkay,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// labels for facts acquired directly as true/false from (binary) points

   Fact_direct_Bso,
   Fact_direct_Bzo,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// labels for facts derived from or between analog data points

   Fact_data_PsaiSteady,
   Fact_data_PsasSteady,

   Fact_data_QadHunting,
   Fact_data_QadSetptSteady,
   Fact_data_QadSteady,
   Fact_data_QadTrackingHigh,
   Fact_data_QadTrackingLow,

   Fact_data_QasHunting,
   Fact_data_QasSetptSteady,
   Fact_data_QasSteady,
   Fact_data_QasTrackingHigh,
   Fact_data_QasTrackingLow,

   Fact_data_Tad_GT_Tai,
   Fact_data_Tad_GT_Taz,
   Fact_data_Tad_LT_Taz,
   Fact_data_TadHunting,
   Fact_data_TadSteady,

   Fact_data_Tam_EQ_Tao,
   Fact_data_Tam_GTE_minTaoTar,
   Fact_data_Tam_GT_TasSetpt,
   Fact_data_Tam_LTE_maxTaoTar,

   Fact_data_Tao_LT_Tar,
   Fact_data_Tao_LT_TasSetpt,
   Fact_data_Tas_EQ_setpt,
   Fact_data_Tas_GT_Tam,
   Fact_data_Tas_LT_Tam,
   Fact_data_Tas_LT_Tar,

   Fact_data_TasFalling, 
   Fact_data_TasHunting,
   Fact_data_TasRising,
   Fact_data_TasSetptSteady,
   Fact_data_TasSteady,
   Fact_data_TasTrackingHigh,
   Fact_data_TasTrackingLow,

   Fact_data_Taz_GT_setptClg,
   Fact_data_Taz_LT_setptHtg,
   Fact_data_TazSetptClgSteady,
   Fact_data_TazSetptHtgSteady,
   Fact_data_TazSteady,
   Fact_data_TazTrackingHigh,
   Fact_data_TazTrackingLow,

   Fact_data_UddSteady,
   Fact_data_UdmSteady,

   Fact_data_UvcHunting,
   Fact_data_UvcSteady,
   Fact_data_UvhSteady,

   Fact_data_ZddSteady,
   Fact_data_ZdmSteady,

   Fact_data_ZvcHunting,
   Fact_data_ZvcSteady,
   Fact_data_ZvhSteady,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// facts comparing acquired data to parameters (either hard in Fact c-tor or gettered R-T from subjects)

   Fact_para_absDifTarTao_GTE_10F,
   Fact_para_fracOA_EQ_min,
   Fact_para_fracOA_GTE_min,

   Fact_para_PsasZero,
   Fact_para_Tam_GT_frzStat,
   Fact_para_QasZero,
   Fact_para_QadZero,

   Fact_para_UddFull,
   Fact_para_UdmFullOA,
   Fact_para_UdmFullRA,
   Fact_para_UvcShut,
   Fact_para_UvhShut,

   Fact_para_ZddFull,
   Fact_para_ZdmFullOA,
   Fact_para_ZdmFullRA,
   Fact_para_ZvcShut,
   Fact_para_ZvhShut,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// facts specific within particular subjects
 
   Fact_subj_ahu_chwCoolingAir,
   Fact_subj_ahu_chwHelpingEcon,
   Fact_subj_ahu_chwNeeded,
   Fact_subj_ahu_econActive,
   Fact_subj_ahu_econAtMax,
   Fact_subj_ahu_econExpected,
   Fact_subj_ahu_inputsSteady,
   Fact_subj_ahu_okayOnEconAlone,
   Fact_subj_ahu_okayOnEconPlusChw,
   Fact_subj_ahu_outputLevel,
   Fact_subj_ahu_preheatNeeded,
   Fact_subj_ahu_unitOn,
   Fact_subj_ahu_unitPreheating,

   Fact_subj_vav_inputsSteady,
   Fact_subj_vav_TazInBand,
   Fact_subj_vav_unitCoolingZone,
   Fact_subj_vav_unitHeatingZone,
   Fact_subj_vav_unitReheating,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// facts sustained as true (or false) over min cycles
 
   Fact_sustained_Bso,
   Fact_sustained_Bzo,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// labels of formulations/calculations based on data

   Formula_fraction_OA_tempProxy,
   Formula_absDif_TarTao,
   Formula_maxTaoTar,
   Formula_minTaoTar,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// labels for knobs

   Knob_chartShew_secsDataUsing,
   Knob_chartShew_tripFreeMargin,
   Knob_chartShew_zPass,

   Knob_chartTrax_appsBtwnResets,
   Knob_chartTrax_halfBand,
   Knob_chartTrax_lagFraction,
   Knob_chartTrax_actionSum,
   Knob_chartTrax_staleFraction,

   Knob_factRelate_hyster,
   Knob_factRelate_slack,
   Knob_factSustained_minCycles,

   Knob_krono_setLookback,

   Knob_rule_idleMode,

   Knob_ruleKit_idleModeAll,
   Knob_ruleKit_loadRtKrono_logicOfOneRule,
   Knob_ruleKit_loadRtKrono_resultsOfAllRules,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// labels for inputs (primitive data)

   Point_binary_system_occupied,
   Point_binary_zone_occupied,
   Point_command_damper_disch,
   Point_command_damper_mixing,
   Point_command_damper_outside,
   Point_command_valve_chw,
   Point_command_valve_hw,
   Point_command_fan_speed,
   Point_flowVolume_air_supply,
   Point_flowVolume_air_supply_setpt,
   Point_flowVolume_air_disch,
   Point_flowVolume_air_disch_setpt,
   Point_position_damper_disch,
   Point_position_damper_mixing,
   Point_position_damper_outside,
   Point_position_valve_chw,
   Point_position_valve_hw,
   Point_power_electric_fan,
   Point_power_electric_preheat,
   Point_power_electric_reheat,
   Point_pressure_static_air_inlet,
   Point_pressure_static_air_supply,
   Point_temperature_air_coldDeck,
   Point_temperature_air_discharge,
   Point_temperature_air_hotDeck,
   Point_temperature_air_inlet,
   Point_temperature_air_lvgPreheat,
   Point_temperature_air_mixed,
   Point_temperature_air_outside,
   Point_temperature_air_return,
   Point_temperature_air_supply,
   Point_temperature_air_supply_setpt,
   Point_temperature_air_zone,
   Point_temperature_air_zone_setpt_clg,
   Point_temperature_air_zone_setpt_htg,
   Point_timeDate_local,

   RuleKit,  // Rule Kits are labeled, but Rules themselves self-identify thru a SayRuleUaiText() method

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
// labels for subjects
 
   Subject_vav_pressIndep_hwReheat,
   Subject_ahu_singleDuct_vavReheat

};

//======================================================================================================/
/* "Alerts" are passed ISeqElement->ASubject->CDomain to real-time "status" window of Surveillance GUI,
   and EAlertMsg content is usually concatenated with EDataLabel, ESubjName, or other content on the
   way to FIFO queue held by CDomain for GUI to access.  Alerts are always "looked up" (converted) to
   text while still within API, so enum is NOT exported to application Client (GUI).
*/

enum struct EAlertMsg : unsigned int {

   Undefined = 0u,

   Common_givenBadKey,

   Krono_givenSecsToShowOor,

   PointAnalog_inputSameDbl,

   RainAnalog_binOverUnder,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// AHU-sdvr specific rule alerts

   Rule_ahuSdvr_1_focus,
   Rule_ahuSdvr_1_if,
   Rule_ahuSdvr_1_then,
   Rule_ahuSdvr_1_onFail,

   Rule_ahuSdvr_2_focus,
   Rule_ahuSdvr_2_if,
   Rule_ahuSdvr_2_then,
   Rule_ahuSdvr_2_onFail,

   Rule_ahuSdvr_3_focus,
   Rule_ahuSdvr_3_if,
   Rule_ahuSdvr_3_then,
   Rule_ahuSdvr_3_onFail,

   Rule_ahuSdvr_4_focus,
   Rule_ahuSdvr_4_if,
   Rule_ahuSdvr_4_then,
   Rule_ahuSdvr_4_onFail,

   Rule_ahuSdvr_5_focus,
   Rule_ahuSdvr_5_if,
   Rule_ahuSdvr_5_then,
   Rule_ahuSdvr_5_onFail,

   Rule_ahuSdvr_6_focus,
   Rule_ahuSdvr_6_if,
   Rule_ahuSdvr_6_then,
   Rule_ahuSdvr_6_onFail,

   Rule_ahuSdvr_7_focus,
   Rule_ahuSdvr_7_if,
   Rule_ahuSdvr_7_then,
   Rule_ahuSdvr_7_onFail,

   Rule_ahuSdvr_8_focus,
   Rule_ahuSdvr_8_if,
   Rule_ahuSdvr_8_then,
   Rule_ahuSdvr_8_onFail,

   Rule_ahuSdvr_9_focus,
   Rule_ahuSdvr_9_if,
   Rule_ahuSdvr_9_then,
   Rule_ahuSdvr_9_onFail,

   Rule_ahuSdvr_10_focus,
   Rule_ahuSdvr_10_if,
   Rule_ahuSdvr_10_then,
   Rule_ahuSdvr_10_onFail,

   Rule_ahuSdvr_11_focus,
   Rule_ahuSdvr_11_if,
   Rule_ahuSdvr_11_then,
   Rule_ahuSdvr_11_onFail,

   Rule_ahuSdvr_12_focus,
   Rule_ahuSdvr_12_if,
   Rule_ahuSdvr_12_then,
   Rule_ahuSdvr_12_onFail,

   Rule_ahuSdvr_13_focus,
   Rule_ahuSdvr_13_if,
   Rule_ahuSdvr_13_then,
   Rule_ahuSdvr_13_onFail,

   Rule_ahuSdvr_14_focus,
   Rule_ahuSdvr_14_if,
   Rule_ahuSdvr_14_then,
   Rule_ahuSdvr_14_onFail,

   Rule_ahuSdvr_15_focus,
   Rule_ahuSdvr_15_if,
   Rule_ahuSdvr_15_then,
   Rule_ahuSdvr_15_onFail,

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// VAV box specific rule alerts

   Rule_vav_1_focus,
   Rule_vav_1_if,
   Rule_vav_1_then,
   Rule_vav_1_onFail,

   Rule_vav_2_focus,
   Rule_vav_2_if,
   Rule_vav_2_then,
   Rule_vav_2_onFail,

   Rule_vav_3_focus,
   Rule_vav_3_if,
   Rule_vav_3_then,
   Rule_vav_3_onFail,

   Rule_vav_4_focus,
   Rule_vav_4_if,
   Rule_vav_4_then,
   Rule_vav_4_onFail,

   Rule_vav_5_focus,
   Rule_vav_5_if,
   Rule_vav_5_then,
   Rule_vav_5_onFail,

   Rule_vav_6_focus,
   Rule_vav_6_if,
   Rule_vav_6_then,
   Rule_vav_6_onFail,

   Rule_vav_7_focus,
   Rule_vav_7_if,
   Rule_vav_7_then,
   Rule_vav_7_onFail,

   Rule_vav_8_focus,
   Rule_vav_8_if,
   Rule_vav_8_then,
   Rule_vav_8_onFail,

   Rule_vav_9_focus,
   Rule_vav_9_if,
   Rule_vav_9_then,
   Rule_vav_9_onFail,

   Rule_vav_10_focus,
   Rule_vav_10_if,
   Rule_vav_10_then,
   Rule_vav_10_onFail,

   RuleKit_caseCountAtMax

};


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

enum struct EDataUnit : unsigned char {

   Undefined = 0u,
   Area_sqFt,
   Binary_Boolean,
   Bindex_rule,
   Bindex_fact,
   Coefficient,
   Count_sum,
   Count_cycle,
   Distance_feet,
   Distance_inch,
   Distance_stdDev,
   Energy_kilowattHr,
   FlowVolume_cfm,
   FlowVolume_gpm,
   Frequency_cph,
   Identifier_ruleUai,
   Index,
   None,
   Power_kiloWatt,
   PressureAbso_psia,
   PressureDiff_psid,
   PressureGage_iwg,
   PressureGage_psig,
   Ratio_fraction,
   Ratio_percent,
   Temperature_degC,
   Temperature_degF,
   TemperatureDiff_Cdeg,
   TemperatureDiff_Fdeg,
   TimeSpan_hour,
   TimeSpan_sec,
   Velocity_fpm
};


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

enum struct ESubjParam : unsigned char {

   Undefined = 0u,
   Ahu_singleDuct_vavReheat,
   Vav_reheat_electric,
   Vav_reheat_hydronic,
   Vav_reheat_none
};


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

enum struct EDataSuffix : unsigned char {

   Undefined = 0u,
   AboutMean,
   ConsecutiveNoTrip,
   LimitHigh,
   LimitLow,
   MaxSeen,
   MinSeen,
   MeanOverSecs,
   None,
   StdDevOverSecs,
   TimesTotal,
   UnitHours
};




#endif


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[2]   Nzint_t for indexing rows (vectors) in deques, & bins within those rows.  As unsigned int, it
      matches size_t type returned by size() in STL containers, but is made a distinct type here because
      per C++ stds, size_t is for expressing the size of a "thing" as the number of BYTES it
      occupies, not as a count of the "thing" itself.  So, the use intended here is to say:

            number of "things" in Nzint_t =
            bytes taken up by a range of "things" in size_t / sizeof( "thing" )

      As unsigned int, it is safe only for indicies and "counts"  
      Do no more than simple, "short" +/- math upon this type [can error upon forced upcasts to signed]

[3]   Dispayable snapshots are vector versus array so to match vector container used for realtime traces
      Oldest value at lowest index, so time plot on GUI display reads "later" going right to left.

      Rainfall obj returns empty container should its getter be called with bad ID when an ID is req'd
      So, callers expecting a time-series return will deduce bad ID from zero size and notify
      user via GUI. This approach avoids Rain object having to hold non-const ref to ISeqElement source
      object.    

--------------------------------------------------------------------------------
XXX END FILE NOTES */



//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
