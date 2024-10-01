// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare abstract base and concrete classes for objects (parts) that are accessed via a View object
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef VIEWPARTS_HPP
#define VIEWPARTS_HPP

#include "guiShadow.hpp"   // brings "customTypes.hpp"

#include <functional>
#include <queue>

// Forward declares
class AFact;
class ARainfall;
class ASubject;

class CCase;
class CController;
class CFormula;
class CKnobSint;
class CPointAnalog;
class CRainAnalog;
class CRainFact;
class CRainRuleKit;
class CRule;
class CRuleKit;
class CSeqTimeAxis;
class CView;

class ISeqElement;

typedef std::vector<GuiFpn_t>                ValuesBuffer_t;
typedef ValuesBuffer_t::reverse_iterator     ValuesRevIter_t;

typedef std::vector<time_t>                  TimesBuffer_t;
typedef TimesBuffer_t::reverse_iterator      TimesRevIter_t;

// For source functions (std::function as wrapper on lambda calling a getter on object sourcing trace)
typedef std::function< float(void) >         FloatGetr_t;
typedef std::function< bool(void) >          BoolGetr_t;

const std::string                            MSG_TROUBLE    = "INVALID";


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
//  Class for GUI "feature"


class AFeature : public IGuiShadow {

   public:

   // Methods
      // Any ABC must define a virtual public dtor so subclass dtors get called on its pointers
      virtual ~AFeature( void ) {  };

      GuiPackFeatureFull_t                SayFullGuiPack( void );
      GuiPackFeatureDyna_t                SayDynamicGuiPack( void );
      void                                Update( void );

   protected:

   // Handle
      const ASubject&                     SubjectRef;

   // Fields
      std::vector<NGuiKey>                knobKeysAllSources;
      const EDataLabel                    label;         // goes next to message "box" on GUI
      const EDataUnit                     units;
      std::string                         messageText;   // string, to allow as words or timestamp
      const Nzint_t                       ownUai;
      EGuiState                           stateDisplayed;
      const NGuiKey                       histogramKey_source;
      static bool                         bufferReserved;

   // Methods
      AFeature(   EApiType,
                  ASubject&,
                  CView&,
                  Nzint_t,
                  EDataLabel,
                  EDataUnit,
                  NGuiKey );

      virtual void                        Regen( void ) = 0;

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Idea is that AFeature holds ref to its data source object, but a rtn-type specific lambda still
      be defined in the concrete subclass (CFeatureAnalog or CFeatureBool) at run-time to specify which
      getter(s) are being called on that source object (i.e., can't hard-code that statically!).

[2]   Making SourceRef public avoids having to replicate in IFeature all the ISeqElement fields/methods
      needed to get knob labels, values, triggersPerCycle

[3]   Feature UAI cannot be auto-assigned by the class, but must be given by GUI designer to each object
      as a c-tor argument (which class checks for duplication error) since designer might be laying out
      GUI features in client code (e.g., JavaScript) using those UAI numbers.


^^^^ END CLASS NOTES */

};

class CFeatureAnalog : public AFeature {

   public:

   // Methods
      CFeatureAnalog(   ASubject&,
                        CView&,
                        Nzint_t,
                        CPointAnalog&,
                        FloatGetr_t );

      CFeatureAnalog(   ASubject&,
                        CView&,
                        Nzint_t,
                        CFormula&,
                        FloatGetr_t );

      ~CFeatureAnalog( void );


   private:

      ISeqElement&                  SourceRef_baseClass; // See Class Note [1]
      FloatGetr_t                   ReadSource;
      static std::ostringstream     floatStreamer;
 
      virtual void                  Regen( void ) override;
      std::string                   WriteOutFloat( float );

/* BEGIN Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Constructor makes LendKnobKeysTo(-) call requiring concrete class of source, but base class alone
      is sufficient for subsequent call during runtime.  Else, would need separate ptrs to each concrete
       analog class (all initialized null except one) with null-test switch code to select between them.

'' END Class Notes ''''' */ 

};


class CFeatureFact : public AFeature {

   public:

   // Methods
      CFeatureFact(  ASubject&,
                     CView&,
                     Nzint_t,
                     AFact&,
                     BoolGetr_t );

      ~CFeatureFact( void );

   private:

      AFact&                       SourceRef_factClass;
      BoolGetr_t                   ReadSource;

      virtual void                 Regen( void ) override;

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract base class for all "Traces"


class ATrace : public IGuiShadow { 

   public:
   // Methods

      ~ATrace( void );

      virtual GuiPackTraceFull_t       SayFullGuiPack( void ) const = 0;      // See Class Note [1]
      virtual GuiPackTraceDyna_t       SayDynamicGuiPack( void ) const = 0;

      int                              SaySecondsPerIndex( void ) const;   // See Class Note [1]
      ERealName                        SayHostSubjectName( void ) const;
      EDataUnit                        SayUnits( void ) const;             // for compat. check on traces
      EDataRange                       SayRange( void ) const;             // for compat. check on traces
      EPlotGroup                       SayPlotGroup( void ) const;

   protected:

   // Handles
      const ISeqElement&            SourceRef;        // avoids some branching on which ptr is non-null
      const ASubject&               SubjectRef;
      CView&                        ViewRef;
      const std::vector<NGuiKey>&   knobKeys_sourceRef;
      NGuiKey                       histogramKey_source;    // See Class Note [2]

      const ARainfall&              RainRef_upcast;

   // Fields
      const std::string             nameText;
      const Nzint_t                 ruleUai; // ruleUai local to CRuleKit; = 0 if Trace not of a Rule

      /* Fist c-tor is called by both CTraceRealtime (except for Rules) and all CTraceSnapshot constructions,
         so some params (e.g., rule UAI) are passed explicitly while appearing callable from passed handles
      */ 
      ATrace(  EApiType,
               const ARainfall&,
               const ASubject&,  
               CView&,
               const std::vector<NGuiKey>&,
               NGuiKey,
               Nzint_t );

      ATrace(  EApiType,
               const CRainRuleKit&,
               const CRule&,
               const ASubject&,  
               CView&,
               const std::vector<NGuiKey>& );

/* Begin Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Virtual because the fields needed to load GuiPacks only exist at concrete subclass level, but method
      must be visible at abstract ATrace level.  Pure ( "= 0" ) because there is no abstracted "default".
      
[2]   It works having ATrace take its histogram key PBV (vs. PBR) in ctor call ONLY because histograms are
      initiated (inside the rainfall of a ISeqElement subclass having one) BEFORE that ("source") object's
      CTraceRealtime object is instantiated. That is NOT similar for knobKeys_sourceRef, because the 
      source's AttachOwnKnobs() routine cannot be run until the end of its (subclass) ctor, which is AFTER
      its CTraceRealtime object instantiated. 

'' End Class Notes */

};

//======================================================================================================/
// Concrete subclass for a realtime Trace

class CTraceRealtime : public ATrace { 

   public:
   // Methods

      CTraceRealtime(   CView&,
                        CRainAnalog&,                    // cannot be const, re. snapshot capture/destroy
                        const std::vector<NGuiKey>& );   // keys to source's own and antecedent knob(s)

      CTraceRealtime(   CView&,
                        CRainFact&,
                        const std::vector<NGuiKey>& );

      CTraceRealtime(   CView&,
                        CRainRuleKit&,
                        const CRule&,
                        const std::vector<NGuiKey>& );

      ~CTraceRealtime( void );

      virtual GuiPackTraceFull_t    SayFullGuiPack( void ) const override;
      virtual GuiPackTraceDyna_t    SayDynamicGuiPack( void ) const override;

      void                          CaptureSnapshotForSetSgi( Nzint_t );
      void                          DestroySnapshotForSetSgi( Nzint_t );


   private:

      friend class CTraceSnapshot;

   // Handles
      CRainAnalog* const            p_RainAnalog;
      CRainFact* const              p_RainFact;
      CRainRuleKit* const           p_RainRuleKit;
};


//======================================================================================================/
// Concrete subclass for a Snapshot Trace

class CTraceSnapshot : public ATrace { 

   public:
   // Methods

      CTraceSnapshot(   const CTraceRealtime&,
                        Nzint_t );

      ~CTraceSnapshot( void );

      virtual GuiPackTraceFull_t    SayFullGuiPack( void ) const override;
      virtual GuiPackTraceDyna_t    SayDynamicGuiPack( void ) const override;

   private:

   // Handles
      CRainAnalog* const            p_RainAnalog;
      CRainFact* const              p_RainFact;
      CRainRuleKit* const           p_RainRuleKit;

   // Fields
      const Nzint_t                 sgiOfSnapshotSetToDisplay;

};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract base class for a Pane, which holds one or more Traces of compatible types.

class APane : public IGuiShadow {

   public:

      ~APane( void );

      GuiPackPane_t        SayGuiPack( void ) const;
      int                  SaySecondsPerCycle( void ) const;    // for matching Panes to a Krono

   protected:

   // Handle
      CView&                     ViewRef;

   // Fields
      std::vector<NGuiKey>       keysOfTracesInPane;
      const size_t               numTracesMax;
      const int                  xAxisSecsPerIndex;
      const EDataUnit            yAxisUnits;
      const EDataRange           yAxisRange;
      const ERealName            hostSubject;
      const EPlotGroup           plotGroup;


   // Methods
      APane(   const CTraceRealtime* const,
               CView& );

      APane(   const CTraceSnapshot* const,
               CView& );

 };


class CPaneSnapshot : public APane {

   public:

      CPaneSnapshot( const CTraceSnapshot* const,               // See Class Note [1]
                     CView& );

      ~CPaneSnapshot( void );

      bool AddTraceIfCompatible( const CTraceSnapshot* const );

/*
''' START Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   C-tor argument is the "founder" Trace for the Pane, setting its base-class fields so that only
      similar Traces will be "compatible" with the Pane.

''' END Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
*/ 


};

//======================================================================================================/
// Concrete subclasses of APane (only req'd to prevent accidental mixing of realtime and snapshot data)

class CPaneRealtime : public APane {

   public:

      CPaneRealtime( const CTraceRealtime* const,               // See Class Note [1]
                     CView& );

      ~CPaneRealtime( void );

      bool AddTraceIfCompatible( const CTraceRealtime* const );


/*' START Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   C-tor argument is the "founder" Trace for the Pane, setting its base-class fields so that only
      similar Traces will be "compatible" with the Pane.

''' END Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
*/

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Chronology ("Krono") objects, each holding one or more panes of various styles. */

class AKrono : public IGuiShadow {

   public:

      ~AKrono( void );

      GuiPackKronoFull_t            SayFullGuiPack( void ) const;
      virtual GuiPackKronoDyna_t    SayDynamicGuiPack( void ) const = 0;
      size_t                        SayNumCyclesLookingBack( void ) const;
      int                           GetSecsLookingBack( void ) const;
      void                          SetSecsLookingBack( int );
  
   protected:

   // Handles
      CSeqTimeAxis&                 TimeAxisRef;
      CView&                        ViewRef;
      int                           secsLookingBack;  // $$$ TBD to impl; must precede knob c-tor $$$
      std::unique_ptr<CKnobSint>    u_Knob;

   // Fields
      std::vector<NGuiKey>       paneIdsTopToBottom;
      const std::string          caption;
      int                        secsPerIndex_sharedTimeAxis;
      size_t                     numIndicies_sharedTimeAxis;
      const Nzint_t              snapshotSetSgi;         // = 0 for realtime Krono

   // Methods
      AKrono(  EApiType,
               CRuleKit&,
               CController&,
               CView&,
               std::string,
               Nzint_t );

/*' START Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Virtualized only to allow "FAIL" return if a snapshot krono is asked for dynamic update.

''' END Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
*/

//======================================================================================================/

 };

class CKronoRealtime : public AKrono {

   public:

      CKronoRealtime(   CRuleKit&,
                        CController&,                 // for span knob
                        CView&,                       // Holds View ref for call by class d-tor
                        const std::string,            // Caption, gets passed in from CRule caller 
                        const CPaneRealtime* const ); // Top pane always one having the rule results

      ~CKronoRealtime( void );

      virtual GuiPackKronoDyna_t    SayDynamicGuiPack( void ) const override;
      void                          AddPaneBelowExistingPanes( const CPaneRealtime* const );

   private:

      CRuleKit&                     RuleKitRef;
 };

//======================================================================================================/

class CKronoSnapshot : public AKrono {

   public:

      CKronoSnapshot(   CRuleKit&,
                        CController&,                 // for span knob
                        CView&,                       // Holds View ref for call by class d-tor
                        const std::string&,           // Caption, gets passed in from CCase caller 
                        Nzint_t,                      // snapshot set SGI
                        const CPaneSnapshot* const ); // Top pane always one having the rule results

      ~CKronoSnapshot( void );

      virtual GuiPackKronoDyna_t    SayDynamicGuiPack( void ) const override;
      void                          AddPaneBelowExistingPanes( const CPaneSnapshot* const );

 };



/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Interface for a Rule Kit Display, which gives GUI the info it needs to display info on each CRule
   in a Subject's CRuleKit.
*/

class CDisplayRuleKit : public IGuiShadow {

   public:

     CDisplayRuleKit(   ASubject&,
                        CRuleKit&,
                        CView& );

      ~CDisplayRuleKit( void );


      GuiPackRuleKitFull_t       SayFullGuiPack( void ) const;
      GuiPackRuleKitDyna_t       SayDynamicGuiPack( void ) const;
      std::string                SayRuleKitCaption( void ) const;

   private:

      const CRuleKit&            RuleKitRef;
 
 };


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Histogram structs and classes

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Histogram "slice" structures

struct SHistoSliceAnalog {

   BinSumsAnalogValue_t    binSums_analogValue;      // sum of booleans in each rainfall bin ("column")
   BinSumsAnalogState_t    binSums_analogState;
   const size_t            numCyclesInSpan;
   time_t                  timeOfFrontEdge;
   const BinSum_t          count_allCycles;
   const int               secsPerCycle;
   const ETimeSpan         span;

   SHistoSliceAnalog(  size_t,        // one-hour depth from source rainfall
                       int,           // secPerCycle of source object
                       ETimeSpan,
                       bool = false );

/* ''' START Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   If bins are a 'closed set' on all possible values/states, then count_allCycles remains a constant
      equal to cyclesDeep. Then, count_allCycles is redundant in generating bar heights by the array
      divide using std::transform as called by SayGuiPack().  A RuleKit slice is different, its multiple
      binSums fields not (usefully) being made up as sets closed on cyclesDeep, so an "action"
      consistent across all slice classes is chosen (small price paid in redundant struct members).
      Holding count_***cycles asfields avoids indefinitely repeated calls to std::accumulate to sum
      across each binSums vector to generate a sumBinSums temp variable. This note is typical for all
      slice classes.

''' END Class Notes ''' */

};

//======================================================================================================/

struct SHistoSliceFact {

   BinSumsFactState_t      binSums_factState;
   const size_t            numCyclesInSpan;
   time_t                  timeOfFrontEdge;
   const BinSum_t          count_allCycles;
   const int               secsPerCycle;
   const ETimeSpan         span;

   SHistoSliceFact( size_t,        // source rainfall one-hour depth in cycles
                    int,           // source secsPerCycle
                    ETimeSpan,
                    bool = false  );

};

//======================================================================================================/
// slice-type to show a histogram of only one individual Rule; thus histogram needs only one "info mode"

struct SHistoSliceRule {

   BinSumsRuleState_t      binSums_ruleState;
   const size_t            numCyclesInSpan;
   time_t                  timeOfFrontEdge;
   const BinSum_t          count_allCycles;
   const int               secsPerCycle;
   const ETimeSpan         span;

   SHistoSliceRule( size_t,        // source rainfall one-hour depth in cycles
                    int,           // source secsPerCycle
                    ETimeSpan,
                    bool = false );

};

//======================================================================================================/
// slice-type to show a histogram of all rules in one RuleKit; thus Histogram needs multiple "info modes"

struct SHistoSliceRuleKit {

   std::vector<BinSum_t>            binSums_validsOnEachRule_barsLeftToRight;
   std::vector<BinSum_t>            binSums_testsOnEachRule_barsLeftToRight;
   std::vector<BinSum_t>            binSums_failsOnEachRule_barsLeftToRight;
   const size_t                     numRulesInKit;
   const size_t                     numCyclesInSpan;
   time_t                           timeOfFrontEdge;
   const BinSum_t                   count_allCycles;
   const int                        secsPerCycle;
   const ETimeSpan                  span;

   SHistoSliceRuleKit( size_t,        // number of rules in rule kit
                       size_t,        // source rainfall one-hour depth in cycles
                       int,           // source secsPerCycle
                       ETimeSpan );

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Functor needed in generating (some) histogram bar heights

template <typename TTOutput>
struct  FDivideIfDefinedElseZero {

   FDivideIfDefinedElseZero<TTOutput>( void ) { }

   ~FDivideIfDefinedElseZero<TTOutput>( void ) { }

    TTOutput operator()( const BinSum_t element_1stOperand, const BinSum_t element_2ndOperand ) {

      return ( ( element_2ndOperand == 0 ) ?       // Allows negative bar heights on signed-int input
                  static_cast<TTOutput>( 0 ) :
                  (  static_cast<TTOutput>(element_1stOperand) /
                        static_cast<TTOutput>(element_2ndOperand)
                  )
      );
   }

   // Simply write additional operator() overloads for further input types or signing constraints

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Abstract base class for all histograms

class AHistogram : public IGuiShadow {

   public:

      virtual ~AHistogram( void );

      virtual GuiPackHistogram_t    SayGuiPack( void ) = 0;
      virtual EGuiReply             SetModeActiveToOptionIndex( size_t );
      EGuiReply                     SetSpanActiveToOptionIndex( size_t );
      std::string                   SayIdentifyingText( void ) const;      // used by exported API (?)


   protected:

      AHistogram( const ASubject&,
                  CPointAnalog&,
                  const std::vector<NGuiKey>&,
                  size_t );

      AHistogram( const ASubject&,
                  CFormula&,
                  const std::vector<NGuiKey>&,
                  size_t );

      AHistogram( const ASubject&,
                  AFact&,
                  const std::vector<NGuiKey>&,
                  size_t );

      AHistogram( const ASubject&,
                  CRule&,
                  CRuleKit&,
                  const std::vector<NGuiKey>&,
                  size_t );

      AHistogram( const ASubject&,
                  CRuleKit&,
                  const std::vector<NGuiKey>&,
                  size_t );

   // Handles
      const ISeqElement&                  SourceRef;  // access to source secsPerCycle, etc.
      CView&                              ViewRef;    // See Class Note [1]
      const std::vector<NGuiKey>&         knobKeys_sourceRef;

   // Fields
      static std::array<ETimeSpan,3>      spansSupported;
      static std::array<std::string,3>    spanLabels;
      const std::string                   captionTextLine_sourceInfo;
      const std::string                   captionTextLine_barLabelLegend_1; // see Class Note [3]
      const std::string                   captionTextLine_barLabelLegend_2; 
      const size_t                        movingHourSpanInSourceCycles;
      const int                           secsPerSourceCycle;
      EInfoMode                           modeActive;
      ETimeSpan                           spanActive;
      size_t                              modeActive_index;
      size_t                              spanActive_index;
      bool                                firstCycle;

   // Methods
      std::vector<std::string>            GenerateCaptionText( time_t );

/* CLASS NOTES '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   $$$ TBD whether this really needed.  Holding ref not actually necessary just to register an object
      to View, but might be used to release resources via d-tor.

[2]   User is to see GUI has rendered over the histogram an informative 'caption' consisting of four
      carriage-return ("CR") lines of text, each generated from respective top-to-bottom elements
      of a std::vector<std::string> caption.  Elements are:
         caption[0] = top line, from captionTextLine_sourceInfo field
         caption[1] = second line, mode of histogram (a constant only for Facts (which have one mode)) 
         caption[2] = third line, span of histogram (lookback time) per ETimeSpan defns available
         caption[3] = fourth line, saying time of histogram "front edge"
         caption[4] = captionTextLine_barLabelLegend_1, which can be empty
         caption[5] = captionTextLine_barLabelLegend_2, which can be empty 
      The only surely static information in caption is the source-description part of the top line
      Second and third lines are user-selectable via EInfoMode and ETimeSpan, respectively

[3]   Empty unless concrete subclass needs it (typ when there are so many bars that very abbreviated
      bar labels are needed, so translation lines in caption are also needed).  In that case the
      subclass c-tor loads it.      
*/

};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete subclass for histograms of analog data

class CHistogramAnalog : public AHistogram {

   public:

      CHistogramAnalog( const ASubject&,              // needed for caption info and view ref
                        CPointAnalog&,                // source object
                        const std::vector<NGuiKey>&,  // keys to knobs of src obj and its antecedents
                        size_t,                       // hour depth of source rainfall
                        float,                        // left (lowest-valued) bin label, see Class Note [1]
                        float );                      // label incr with each bin

      CHistogramAnalog( const ASubject&,
                        CFormula&,
                        const std::vector<NGuiKey>&,
                        size_t,
                        float,
                        float );

      ~CHistogramAnalog( void );

      virtual GuiPackHistogram_t    SayGuiPack( void ) override;
      virtual EGuiReply             SetModeActiveToOptionIndex( size_t ) override;
      void                          Cycle(   time_t,
                                             bool,
                                             bool,
                                             Bindex_t,
                                             Bindex_t,
                                             Bindex_t,
                                             Bindex_t );

   private:

   // Fields
      static std::array<EInfoMode,2>                                    modesSupported_analog;
      static std::array<std::string,2>                                  modeLabels_analog;
      static std::array<std::string,FIXED_RAIN_ANALOGSTATE_NUMBINS>     barLabels_analogStates;

      std::deque<SHistoSliceAnalog>       sliceLog_past24clockHrs;
      std::deque<SHistoSliceAnalog>       sliceLog_past7calendarDays;
      SHistoSliceAnalog                   realtimeSlice_movingHour;
      const float                         labelLeftBar_analogValue;  // See Class Note [3]
      const float                         labelIncrPerBar_analogValue;

   // Methods
      std::vector<GuiFpn_t>         GenerateBarHeightsFromSlice( const SHistoSliceAnalog& );
      SHistoSliceAnalog             GenerateSliceSummedOnPast24hrs( void );
      SHistoSliceAnalog             GenerateSliceSummedOnPast7days( void );
      void                          InitializeAnalogSliceOnValue( SHistoSliceAnalog&,
                                                                  Bindex_t );


/* Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   SBinParams& as arg would need a rainfall.hpp include, causing self-referential header circularity

[2]   Having "slices" take numCyclesLogged as an arg passed down thru successive c-tor calls (vs. it
      being initialized locally with a global const) allows for future dev of specialized slice obj. 

[3]   For analog value 'mode', decided better to send GUI developer just two floats (left bin label
      and label incr) vs. a ref to a static vector of strings, letting him figure out a comprehensive
      bar-labeling format suitable for the available GUI 'real estate'.

''' End Class Notes''' */

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete subclass for histograms of Facts

class CHistogramFact : public AHistogram {

   public:

      CHistogramFact(   const ASubject&,              // needed for caption info and view ref
                        AFact&,                       // source
                        const std::vector<NGuiKey>&,  // keys to knobs of src obj and its antecedents
                        size_t );                     // hour depth of source rainfall

      ~CHistogramFact( void );

      virtual GuiPackHistogram_t    SayGuiPack( void ) override;
      void                          Cycle(   time_t,
                                             bool,
                                             bool,
                                             Bindex_t,
                                             Bindex_t );

   private:

   // Fields
      static std::array<EInfoMode,1>                                    modesSupported_fact;
      static std::array<std::string,1>                                  modeLabels_fact;
      static std::array<std::string,FIXED_RAIN_FACTSTATE_NUMBINS>       barLabels_fact;

      std::deque<SHistoSliceFact>      sliceLog_past24clockHrs;
      std::deque<SHistoSliceFact>      sliceLog_past7calendarDays;
      SHistoSliceFact                  realtimeSlice_movingHour;

   // Methods
      std::vector<GuiFpn_t>            GenerateBarHeightsFromSlice( const SHistoSliceFact& );
      SHistoSliceFact                  GenerateSliceSummedOnPast24hrs( void );
      SHistoSliceFact                  GenerateSliceSummedOnPast7days( void );
};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete subclass for histogram of a Rule

class CHistogramRule : public AHistogram {

   public:

      CHistogramRule(   const ASubject&,              // needed for caption info and view ref
                        CRule&,                       // source for completing caption
                        CRuleKit&,                    // source for remaining fields
                        const std::vector<NGuiKey>&,  // keys to knobs of src obj and its antecedents
                        size_t );                     // hour depth of source rainfall

      ~CHistogramRule( void );

      virtual GuiPackHistogram_t    SayGuiPack( void ) override;
      void                          Cycle(   time_t,
                                             bool,
                                             bool,
                                             Bindex_t,
                                             Bindex_t );

   private:

   // Fields
      static std::array<EInfoMode,1>                                    modesSupported_rule;
      static std::array<std::string,1>                                  modeLabels_rule;
      static std::array<std::string,FIXED_RAIN_RULESTATE_NUMBINS>       barLabels_rule;

      std::deque<SHistoSliceRule>      sliceLog_past24clockHrs;
      std::deque<SHistoSliceRule>      sliceLog_past7calendarDays;
      SHistoSliceRule                  realtimeSlice_movingHour;

   // Methods
      std::vector<GuiFpn_t>            GenerateBarHeightsFromSlice( const SHistoSliceRule& );
      SHistoSliceRule                  GenerateSliceSummedOnPast24hrs( void );
      SHistoSliceRule                  GenerateSliceSummedOnPast7days( void );
};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete subclass for histograms of Rule Kits

class CHistogramRuleKit : public AHistogram {

   public:

      CHistogramRuleKit(   const ASubject&,                 // needed for caption info and view ref
                           CRuleKit&,                       // source (a Rule Kit)
                           const std::vector<NGuiKey>&,     // keys to Rule Kit knobs only
                           size_t,                          // one-hour depth of source rainfall 
                           const std::vector<Nzint_t>& );   // ref to Rule Uai in GUI-display order 

      ~CHistogramRuleKit( void );

      virtual GuiPackHistogram_t    SayGuiPack( void ) override;
      virtual EGuiReply             SetModeActiveToOptionIndex( size_t ) override;
      void                          Cycle(   time_t,
                                             bool,
                                             bool,
                                             const std::unordered_map<Nzint_t,Bindex_t>&,
                                             const std::unordered_map<Nzint_t,Bindex_t>& );
   private:

   // Fields
      static std::array<EInfoMode,3>      modesSupported_ruleKit;
      static std::array<std::string,3>    modeLabels_ruleKit;

      std::deque<SHistoSliceRuleKit>      sliceLog_past24clockHrs;
      std::deque<SHistoSliceRuleKit>      sliceLog_past7calendarDays;
      SHistoSliceRuleKit                  realtimeSlice_movingHour;

      const std::vector<std::string>      barLabels_ruleIdentifiers;    // runtime information V V V
      const std::vector<Nzint_t>&         ruleUais_barsLeftToRight;
      const size_t                        numRulesInKit;

   // Methods
      std::vector<std::string>        ConfigureBarLabelsFromRuleKit( const std::vector<Nzint_t>& ); 
      std::vector<GuiFpn_t>           GenerateBarHeightsFromSlice( const SHistoSliceRuleKit& );
      SHistoSliceRuleKit              GenerateSliceSummedOnPast24hrs( void );
      SHistoSliceRuleKit              GenerateSliceSummedOnPast7days( void );

/* Class Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   

''' End Class Notes''' */

};

#endif

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   Traces self-register with the pane passed by reference to argument list of their constructor.  The
      Register() call is in the c-tor of the trace's concrete subclass, not its ATrace interface, so that
      an analog trace can Register() only to an analog pane, and a bool trace only to a bool pane.  The
      distinction must be maintained as (1) the source data being fetched has both bool and float types,
      and (2) floats and bools require different y-axis treatments when plotted in a pane, so the
      panes must have float and bool "typing" to match which type data they show.

      Panes do NOT self-register with a page, as that would cause circular includes in headers of both.

--------------------------------------------------------------------------------
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
