// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares abstract and concrete classes for all rainfalls and (basic) statistics kit
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef RAINFALL_HPP
#define RAINFALL_HPP

#include "customTypes.hpp"
#include <unordered_map>

// fwd declarations
class AFact;
class ASubject;
class CFormula;
class CHistogramAnalog;
class CHistogramFact;
class CHistogramRule;
class CHistogramRuleKit;
class CPointAnalog;
class CRule;
class CRuleKit;
class CSeqTimeAxis;
class CView;
class ISeqElement; 

//======================================================================================================/
/* Types needed in Rainfalls but not needing global definition (i.e., not needed by other, non-rainfall
   class hdrs via customTypes.hpp).
*/

typedef bool Bin_t;    // One bin in any evaluated row of rainfall bins is "true", ALL others "false"

//cycle-by-cycle (i.e., time-series) log of index of the "true" bin ("bindex") in row of rainfall bins :
typedef std::deque<Bindex_t>                                   BindexLog_t;
typedef std::unordered_map<Nzint_t, BindexLog_t>               RuleToLogTable_t;
typedef std::array<Bin_t,FIXED_RAIN_ANALOGVALUE_NUMBINS>       BinRowAnalogValue_t;
typedef std::array<BinRowAnalogValue_t, FIXED_RAINFALL_SIZE>   Rainfall_t;

/* So, rainfall[row][col] -> [ index of source object cycle (i.e., "time") ][ index of binned value ]
   Methods writing data into rainfall put "newest" row at lowest index ("front"); "oldest" row at
   highest index ("back") [so "front" of "rainfall" is its "top" (the "cloud"), its "back" is "ground"] 
*/

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rainfall-related data structures

struct SBinParamsAnalogValue {     // Unlike facts or rules, analog bin params are runtime (non-const) info

   const std::array<float,FIXED_RAIN_ANALOGVALUE_NUMBINS>   labels;
   const float                                              binWidth;   
   const float                                              binHalfWidth;
   const float                                              minVariance;

/* Struct holds both width & half-width due to number of hits on each
   Binning makes min variance = (binning process uncertainty)^2 = (binWidth/2)^2 = binwidth^2/4
   Uncertainty = binWidth/2 instead of binWidth, because "process" is to add valueRaw + (binWidth/2),
   then round that sum to label value of bin "floor".

   Thus, for practical purposes, a bin's "label" can be said to be "center value" of that bin.
*/

   SBinParamsAnalogValue( std::array<float,FIXED_RAIN_ANALOGVALUE_NUMBINS> arg0,
                           float arg1 )
                           :  labels (arg0),
                              binWidth (arg1),
                              binHalfWidth (arg1/2.0f),
                              minVariance ( (arg1*arg1)/4.0f ) {
   }
};

typedef std::unordered_map< EDataRange, SBinParamsAnalogValue >  AnalogValueBinsTable_t;


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract base class for all rainfalls

class ARainfall {

   public:

   // Handle
      ISeqElement&   SourceRef;  // for classes holding ARainfall handle and needing its source obj

   // Methods
      // Interface must define an empty virtual public dtor so subclass dtors get called on its ptrs
      virtual ~ARainfall( void );

      EGuiReply                     ResizeLoggingToAtLeastSecsAgo( int );
      bool                          IsSnapshotSgiValid( Nzint_t ) const;
      void                          DestroySnapshotForSetSgi( Nzint_t );
 
   protected:

   // Handles


   // Fields

      static Rainfall_t             rainfallRegister;      // One actual "rainfall" that all instances use
      static size_t                 numIndiciesInSnapshot;

      RuleToLogTable_t              ruleStatesLoggedAsBindex_byRuleUai;
      BindexLog_t                   statesLoggedAsBindex;
      BindexLog_t                   valuesLoggedAsBindex; // used only by CRainAnalog subclass
      SnapshotBindexBank_t          snapshotsStateBindex_bySetSgi; // See Class Note [2]
      SnapshotBindexBank_t          snapshotsValueBindex_bySetSgi; // used only by CRainAnalog subclass
      std::map<size_t, Nzint_t>     spansInUse;      // <first, second> = <spanInCycles, num "users">
      const size_t                  movingHourSpanInCycles;    // used by histograms and long-term statistics
      const size_t                  lastIndexInMovingHour;     // used by histograms and long-term statistics
      const size_t                  numCyclesDuringSnapshot;
      size_t                        numCyclesLogging;
      const int                     secsPerCycle;
      const int                     numPastesEachCycleToSnapshot;
      const int                     secsLoggingMax;
      int                           secsLogging; // not passed, but post-calc'd as numCycles*secsPerCycle
      const EApiType                ownApiType;

   // Methods
      ARainfall(  ISeqElement&,
                  EApiType );
 

      static void                   ReadBindexLogIntoRainfall( const std::deque<Bindex_t>& );


/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   In lieu of a SaySecsLogged() method, which would then need a subsequent hit on a separate method
      to resize.  All trace/pane/krono classes have 1:1 correspondence of indexed element:seq trigger.
      That is, their "tpc" is always 1, and a rainfall's Display...() methods must accommodate any
      source tps that is different (i.e., larger than 1).

[2]   Non-static, as each instance must have its own bank of snapshots that each belong to a snapshot
      "set" specific to 'fail' results from a particular CRule object.  Only one setId is issued, and
      only that set of rule/fact/point snapshots are taken and "banked", whenever any CRule has a
      'fail' result.  The set remains in existence 

^^^^^ END CLASS NOTES */
     
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete class for rainfalls following analog (i.e., "continuously" valued) POINTS or FORMULA results

class CRainAnalog : public ARainfall {

   public:
   // Methods

      CRainAnalog(   CPointAnalog&,    // CPointBinary does not hold a rainfall, its Fact object does
                     std::vector<NGuiKey>&,  // knob keys of src obj and its antecedents -> histogram
                     ASubject&,
                     EDataRange );

      CRainAnalog(   CFormula&,
                     std::vector<NGuiKey>&,
                     ASubject&,
                     EDataRange );

      ~CRainAnalog( void );

      static const SBinParamsAnalogValue& lookup_AnalogValueBins( const EDataRange& );

      TraceGuiNumbersOldToNew_t     SayGuiNumbersFromSnapshotInSet( Nzint_t ) const;
      TraceGuiStatesOldToNew_t      SayGuiStatesFromSnapshotInSet( Nzint_t ) const;
      TraceGuiNumbersOldToNew_t     SayGuiNumbersFromBindexLog( void );
      TraceGuiStatesOldToNew_t      SayGuiStatesFromBindexLog( void );
      NGuiKey                       SayHistogramKey( void ) const; // See Class Note [3]
      GuiFpn_t                      SayGuiNumberFromNewestBindex( void ) const;
      EGuiState                     SayGuiStateFromNewestBindex( void ) const;


      float                         NowX( void ) const;
      float                         NowY( void ) const;
      float                         OldY_atDepth( size_t ) const;
      float                         MeanY_toDepth( size_t ) const;   // See File Note [1]
      float                         StdDevY_toDepth( size_t ) const; // See File Note [1]
      float                         SayCenterBinLabel( void ) const;
      float                         SayBinWidth( void ) const;
      EGuiReply                     AddNewStatisticsUserSpanningCycles( size_t );
      EGuiReply                     ShiftSpanOfStatisticsForOneUserFromTo( size_t, size_t );
      bool                          IsValidOverCycles( size_t ) const;
      void                          CaptureSnapshotForSetSgi( Nzint_t );

      void                          Cycle(   time_t,     // timestamp now
                                             bool,       // new calendar day?                
                                             bool,       // new clock hour?
                                             float,      // analog value from source obj
                                             bool );     // value is valid at source?


   private:

      static AnalogValueBinsTable_t          analogValueBinsLookup;

   // Handles
      const SBinParamsAnalogValue&           binParamsRef;
      std::unique_ptr<CHistogramAnalog>      u_Histogram_analog;    // See Class Note [1]
 
   // Fields
      std::unordered_map<size_t, float>      yMeansByDepth;
      std::unordered_map<size_t, float>      yVariancesByDepth;
      std::unordered_map<size_t, float>      yStdDevsByDepth;
      float                                  xNow;
      float                                  yNow;
      float                                  yMaxHeld;      // use of following fields is TBD
      float                                  yMinHeld;
      float                                  xMaxSeen;
      float                                  xMinSeen;
      bool                                   binOverUnderSeen;
      bool                                   firstCycle; // See Class Note [2]
      bool                                   validAtSource;
 
   // Methods
      static SBinParamsAnalogValue     SpecifyBinParamsForAnalogValues( float,
                                                                        float );
      void                             UpdateLogging_Analog(   time_t,
                                                               bool,
                                                               bool );
      void                             UpdateStatistics_Analog( void );

/* Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Composition (vs. aggregation) by inbuilt object vs. by a (smart) pointer to off-built object.
      Of the two main advantages to composing by pointer [(1) being able to "null" the member in some
      instances of the class, (2) being able to "swap" member in and out], neither are needed here.
      Aggregation (used when lifetime differs between member and class instance) is ALWAYS by pointer.

[2]   firstCycle only put in subclasses that need it and will properly reset it upon first cycle.

[3]   Not in ARainfall because the data it accesses (histogramKey) cannot be in ARainfall; the key cannot
      be reassigned, only copied into field from c-tor of a concrete histogram kit class (?)      

'''End Class Notes '''*/

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete class for rainfall following an object of any AFact subclass

class CRainFact : public ARainfall {

   public:
   // Methods

      CRainFact(  AFact&,
                  std::vector<NGuiKey>&,  // knob keys of src obj and its antecedents -> histogram
                  ASubject& );

      ~CRainFact( void );

      TraceGuiStatesOldToNew_t      SayGuiStatesFromSnapshotInSet( Nzint_t ) const;
      TraceGuiStatesOldToNew_t      SayGuiStatesFromBindexLog( void );
      NGuiKey                       SayHistogramKey( void ) const;
      EGuiState                     SayGuiStateFromNewestBindex( void ) const;
      Bindex_t                      BindexWas_atCycles( size_t ) const;
      void                          CaptureSnapshotForSetSgi( Nzint_t );

      void                          Cycle(   time_t,     // time now
                                             bool,       // new calendar day?
                                             bool,       // new clock hour?
                                             bool,       // data value from source obj
                                             bool );     // source obj valid?

   private:

   // Handles
      std::unique_ptr<CHistogramFact>        u_Histogram_fact;
 
   // Fields
      Bindex_t                               bindexNow;
      bool                                   lastValidClaim;

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete class for rainfalls sourced by CRule objects

typedef std::unordered_map<Nzint_t, std::unique_ptr<CHistogramRule>>       RuleHistoOwnershipTable_t;
typedef std::unordered_map<Nzint_t,Bindex_t>                               RuleToBindexTable_t;
typedef std::unordered_map<Nzint_t, int>                                   RuleToCostTable_t;
typedef std::unordered_map<Nzint_t, Nzint_t>                               RuleToSnapshotSetTable_t;

typedef struct SRuleTrapResult {

   const Nzint_t  uaiOfRuleTrappedForNewCase;
   const Nzint_t  uaiOfRuleToWipeOfSnapshots;

   SRuleTrapResult(  Nzint_t arg0,
                     Nzint_t arg1 )
                     :  uaiOfRuleTrappedForNewCase (arg0),
                        uaiOfRuleToWipeOfSnapshots (arg1) {
   }
} RuleTrapResult_t;

//======================================================================================================/

class CRainRuleKit : public ARainfall {

   public:
   // Methods

      CRainRuleKit(  CRuleKit&,
                     std::vector<NGuiKey>&,  // knob keys of src obj and its antecedents -> histogram
                     ASubject&,
                     size_t );               // trap depth in cycles 

      ~CRainRuleKit( void );

      TraceGuiStatesOldToNew_t      SayGuiStatesFromSnapshotInSet( Nzint_t ) const;
      TraceGuiStatesOldToNew_t      SayGuiStatesFromBindexLogUnderRuleUai( Nzint_t );
      std::vector<EGuiState>     SayNewestRuleStates_GuiTopToBottom( const std::vector<Nzint_t>& ) const;
      NGuiKey                       SayKeyToOverviewHistogram( void ) const;
      NGuiKey                       SayKeyToHistogramOfRule( Nzint_t ) const;
      EGuiState                     SayGuiStateFromNewestBindexUnderRuleUai( Nzint_t ) const;
      int                           GetTrapSpanInSecs( void ) const;
      EGuiReply                     SetTrapSpanInSecs( int );
      std::pair<Nzint_t,bool>       CycleRulesInKitAndSayResults( time_t,  // time now
                                                                  bool,    // new day?
                                                                  bool,    // new hour?
                                                                  RuleUaiToPtrTable_t& );

      void                          FinalizeRainfallOnTheRulesAddedToKit(  CRuleKit&,                                                                        
                                                                           CView&,
                                                                           const RuleUaiToPtrTable_t&,
                                                                           const std::vector<Nzint_t>& );


                                                         

   private:

   // Handles
      CSeqTimeAxis&                          TimeAxisRef;
      RuleHistoOwnershipTable_t              u_HistogramsForEachRuleInKit_byUai;
      std::unique_ptr<CHistogramRuleKit>     u_Histogram_ruleKitOverview;   // See Class Note [1]
      std::vector<NGuiKey>&                  ruleKitKnobKeysRef;  // needed when RKO histogram constructed

   // Fields
      RuleToBindexTable_t        bindexEnteringMovingHour_byRuleUai;
      RuleToBindexTable_t        bindexLeavingMovingHour_byRuleUai;
      RuleToCostTable_t          ruleFailCosts_byRuleUai;
      RuleToSnapshotSetTable_t   snapshotSetSgis_byRuleUai;
      std::vector<Nzint_t>       ruleUais_indexedAsLogsIterate;         // See Class Note [2]
      std::vector<EGuiState>     ruleStatesNewest_indexedAsLogsIterate;
      std::vector<BinSum_t>      ruleHasNoSnapshot_indexedAsLogsIterate;// int, not bool, to map xform
      std::vector<bool>          rulePinnedToUnitOutput_indexedAsLogsIterate;
      std::vector<bool>          ruleFailedNow_anyMode_indexedAsLogsIterate;
      std::vector<int>           pinnedRuleFailedNow_anyMode_indexedAsLogsIterate;
      size_t                     numRulesInKit;                         // non-const runtime info
      size_t                     trapSpanInCycles;                      // non-const allows r-t changes
      int                        trapSpanInSecs;
      int                        cyclesPerTrapEnable;       // signed type for faster/safer arithmetic
      int                        cyclesUntilTrapEnable;

   // Methods

      RuleTrapResult_t           EnableTrapAndSayResult( const RuleUaiToPtrTable_t& );
      void                       SaveRuleSnapshotUnderSetSgi(  Nzint_t,
                                                               Nzint_t );
      void                       WriteFailsOnRulesToRainfall( const RuleUaiToPtrTable_t& );

/*
''' START Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Here, have composition (vs. aggregation) by ptr to object (vs. inbuilt object, as in other
      rainfall subclasses) to allow calling histogram c-tor AFTER num rules in kit is known
      (which is runtime info). 

[2]   The order by which an iterator traverses elements in a std::unordered_map CANNOT be assumed as the
      same order the elements were inserted/emplaced into it.  So, this field is used to hold ruleUai of
      iteration order compiler chose in laying out the bindex log look-up table (a std::unordered_map).
      Per S.O. post by M. Seymour (cites ISO C++11 23.2.5/8), that order, once chosen, remains unchanged
      as long as a table rehash is not triggered by any mutating operation (that is how iterators remain
      valid call-to-call).

''' END Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
*/ 

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Typedef member function pointers (MFptr) to getter methods of concrete classes declared in this file.
// Used when specifying template parameters for objects of some AFact subclasses

typedef float (CRainAnalog::*PtrRainGetr_t)(void) const;

typedef bool (CRainFact::*PtrRainGetrB_t)(void) const;

#endif

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   High-frequency getters use "cycles" instead of "secsAgo" to index the data called, thus avoiding
      stupidity of abusively repeating a casted divison with "secsPerCycle".  This requires EVERY caller
      to access callee's "secsPerCycle" each time the "secsAgo" it is using is reset by GUI action, and
      to hold resulting quotient as a field it routinely writes into the getter call.

XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
