// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares Includes and customized types exported to be used by EA front end
   Principal contents: "Key" type for use by runtime API, Enumerated types, "GUI pack" structs
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef EXPORTTYPES_HPP
#define EXPORTTYPES_HPP

#include <string>
#include <vector>
#include <utility>
#include <queue>
#include <ctime>

// A type being "free" is one not tied to an exported enum interpretation when returned in vectors, etc.
// (i.e., "just a number")
typedef double                      GuiFpn_t;        // for all free floating point numbers sent to GUI
typedef int                         GuiSin_t;        // for all free signed integers sent to GUI
typedef unsigned int                GuiUin_t;        // for all free unsigned integers sent to GUI

//======================================================================================================/
// A type-safe class for ID numbers [Emil Ernerfeldt, 2014]


class NGuiKey {

   public:

      NGuiKey( void );

      ~NGuiKey( void );

      explicit NGuiKey( unsigned long long );

      NGuiKey( const NGuiKey& );                      // Copy c-tor, See Class Note [1]

      NGuiKey( NGuiKey&& );                           // Move c-tor, See Class Note [1]

      NGuiKey& operator=( const NGuiKey& );           // See Class Note [2]
      NGuiKey& operator=( NGuiKey&& );

      friend bool operator<( NGuiKey, NGuiKey );      // So class will work in ordered containers
      friend bool operator==( NGuiKey, NGuiKey );
      friend bool operator!=( NGuiKey, NGuiKey );

      inline unsigned long long Peek( void ) const { return key; };

   private:

      unsigned long long key;                         // non-const, to support move semantics

/* Start Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Copy c-tors should typ not be made 'explicit'.  E.g., non-explicit allows:  NGuiKey id = 3;
      which avoids creating a temporary NGuiKey obj, because compiler is allowed call copy c-tor instead
      of having to do: NGuiKey id = NGuiKey( 3 ); (i.e., non-explicit allows "implicit" copy c-tor call).
      If copy c-tor had been declared explicit, that would always need to be: NGuiKey id( NGuiKey(3) );

[2]   .cpp overloads operator=() with copy-ctor, so no NGuiKey object can have its key value reassigned!

''' End Class Notes '''''' */

};

// Enable std namespace to recognize NGuiKey as a valid "key" type for STL maps (hash tables) etc.
namespace std {
   template <>
   struct hash<NGuiKey> {

      size_t operator()( const NGuiKey& x ) const {

         return hash<unsigned long long>()( x.Peek() );
      }
   };
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

class XStlString : public std::string { };
class XStlVector_guiKey : public std::vector<NGuiKey> { };
class XStlVector_guiDbl : public std::vector<GuiFpn_t> { };
class XStlVector_string : public std::vector<std::string> { };
class XStlVector_time : public std::vector<time_t> { };
class XStlQueue_string : public std::queue<std::string> { };

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^/

/* Each GUI 'zone' (tab, page, whatever) for a particular Subject is to display a box giving various,
   knob-changeable parameters of the Subject, as populated by these string-value pairs:
*/
//typedef std::pair<std::string, GuiFpn_t>        SubjectParamPair_t;
//typedef std::vector<SubjectParamPair_t>         SubjectParamPack_t;

enum class EGuiReply : unsigned char {

   OKAY_allDone = 0,
   OKAY_done_caseDestroyedByUser_discardKey,
   OKAY_done_kronoDestroyedByUser_discardKey,

   FAIL_any_calledFunctionNotYetImplemented,
   FAIL_any_givenKeyNotValidForFunctionCalled,

   FAIL_get_calledDynamicUpdateBeforeFullGuiPack,
   FAIL_get_calledDynamicUpdateUsingKeyToStaticData,
   FAIL_get_calledHistogramNotYetRecorded,

   FAIL_set_askedForNewKronoBeforeExistingCleared,
   FAIL_set_askedStatisticBeyondDepthLimit,
   FAIL_set_givenContainerWrongSizeForKeyGiven,
   FAIL_set_givenDataLoggingSizeNotWithinBounds,
   FAIL_set_givenDialogueAnswerNotAllowed,
   FAIL_set_givenHistogramModeNotAvailable,
   FAIL_set_givenRuleIdNotInRuleKit,
   FAIL_set_givenTimestampSameAsPrevious,
   FAIL_set_givenValueOutOfRangeAllowed,
   FAIL_set_givenValueNotInSelectionSet,
   FAIL_set_typeGivenNotTypeTaken,

   WARN_nullReply_FixApi,
   WARN_ranSeqToExitWithObjectsYetToCycle_fixApi

};


enum class EGuiType : unsigned char {

   Undefined,                  // Typically only to fill field(s) in an empty view/control struct
   Case,
   Display_ruleKit,
   Domain,
   Feature_analog,
   Feature_fact,

   Histogram_analog,
   Histogram_fact,
   Histogram_rule,
   Histogram_ruleKit,
   HistogramBars_analogBins,
   HistogramBars_analogStates,
   HistogramBars_factStates,
   HistogramBars_ruleStates,
   HistogramBars_rulesUai,

// See Class Note [1]
   Knob_takesGuiFpnAsBoolean,
   Knob_takesGuiFpnAsInteger,
   Knob_takesGuiFpnAsFloat,
   Knob_takesGuiUin,
   Knob_selectsGuiFpnFromList,

   Krono_realtime,
   Krono_snapshot,
   Pane_analog,
   Pane_fact,
   Pane_rule,
   Subject,
   Trace_analog,
   Trace_fact,
   Trace_rule


// BEGIN Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/*
[1]   Four types of "Knobs" are DECLARED for use by the GUI, but which of those are ACTUALLY known to
      GUI, (due to finding them in the "GuiPack" structs), is a design option determined by the mapping
      in the EApiType:EGuiType lookup table (i.e., there are in general many more "API types" than "GUI
      types", and the GUI developer might not want so many "knob" types to have to code around).

*/
// END Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
};


enum class EPointName : unsigned int {

   Undefined,
   Binary_systemOccupied,
   Binary_zoneOccupied,
   Command_damper_mixingBox,
   Command_damper_outsideAir,
   Command_damper_vav,
   Command_fanSpeed,
   Command_valve_chw,
   Command_valve_hw,
   FlowRateVolume_air_ahu,
   FlowRateVolume_air_ahu_setpt,
   FlowRateVolume_air_vav,
   FlowRateVolume_air_vav_setpt,
   Position_damper_mixingBox,
   Position_damper_outsideAir,
   Position_damper_vav,
   Position_valve_chw,
   Position_valve_hw,
   Pressure_static_air_inlet,
   Pressure_static_air_supply,
   Pressure_static_air_supply_setpt,
   Temperature_air_discharge,
   Temperature_air_inlet,
   Temperature_air_mixed,
   Temperature_air_outside,
   Temperature_air_return,
   Temperature_air_supply,
   Temperature_air_supply_setpt,
   Temperature_air_zone,
   Temperature_air_zone_setpt_clg,
   Temperature_air_zone_setpt_htg
};


enum class EGuiState : unsigned char {

   Undefined,

   // Mutually exclusive stati (e.g., GUI colors) of analog values, Features, Rules, and Facts

   Analog_invalid,
   Analog_unavailable,
   Analog_valid,

   Fact_false,
   Fact_invalid,
   Fact_true,
   Fact_unavailable,

   Feature_false,
   Feature_invalid,
   Feature_neutral,
   Feature_true,

   Rule_autoMode_fail,
   Rule_autoMode_pass,
   Rule_autoMode_skip,
   Rule_caseMode_fail,
   Rule_caseMode_pass,
   Rule_caseMode_skip,
   Rule_idleMode_fail,
   Rule_idleMode_pass,
   Rule_idleMode_skip,
   Rule_invalid,
   Rule_unavailable

};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
//  "Typed" structures used for all information from API to Client (GUI)


typedef struct SGuiPackDomain {

   EGuiReply                        getterReply;
   EGuiType                         ownType;
   NGuiKey                          ownKey;
   std::string                      ownNameText;
   std::vector<NGuiKey>             subjectKeys;

   SGuiPackDomain(   NGuiKey,
                     EGuiType,
                     std::string,
                     std::vector<NGuiKey>
   );

   // No 2nd c-tor for GUI pack being called by incorrect key, as domain is only pack called w/o a key

} GuiPackDomain_t;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

typedef struct SGuiPackFeatureFull {

   EGuiReply                        getterReply;
   EGuiType                         ownType;
   NGuiKey                          ownKey;
   GuiUin_t                         featureUai;
   std::string                      labelText;
   std::string                      unitsText;
   std::string                      messageText;
   EGuiState                        messageState;
   NGuiKey                          sourceHistogramKey;
   std::vector<NGuiKey>             ownKnobKeys;


   SGuiPackFeatureFull( EGuiType,
                        NGuiKey,
                        GuiUin_t,
                        std::string,
                        std::string,
                        std::string,
                        EGuiState,
                        NGuiKey,
                        std::vector<NGuiKey>  );


   explicit SGuiPackFeatureFull( EGuiReply );


} GuiPackFeatureFull_t;


typedef struct SGuiPackFeatureDyna {

   EGuiReply              getterReply;
   std::string            messageText;
   EGuiState              messageState;

   SGuiPackFeatureDyna(  std::string, EGuiState );

   explicit SGuiPackFeatureDyna( EGuiReply );

} GuiPackFeatureDyna_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


typedef struct SGuiPackKnob {

   EGuiReply                     getterReply;
   EGuiType                      ownType;
   NGuiKey                       ownKey;
   std::string                   labelText;
   std::string                   unitsText;
   std::vector<GuiFpn_t>         rangeMinMax_emptyIfBool;
   std::vector<GuiFpn_t>         definedSelection_emptyIfNA;
   GuiFpn_t                      valueNow_numerIfBool;

   SGuiPackKnob(  EGuiType,
                  NGuiKey,
                  std::string,
                  std::string,
                  std::vector<GuiFpn_t>,
                  std::vector<GuiFpn_t>,
                  GuiFpn_t );

   explicit SGuiPackKnob( EGuiReply );

} GuiPackKnob_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


typedef struct SGuiPackCaseFull {

   EGuiReply                     getterReply;
   EGuiType                      ownType;
   NGuiKey                       ownKey;
   NGuiKey                       snapshotKronoKey;   // snapshot pane and trace keys are found via krono
   std::string                   caseName;

   // each string in a vector formats its text as would carriage returns ("CR"), but CRs not included
   std::vector<std::string>      reportText_byCR;
   std::vector<std::string>      promptText_byCR;
   std::vector<std::string>      optionText_each;


   SGuiPackCaseFull( EGuiType,
                     NGuiKey,
                     NGuiKey,
                     std::string,
                     std::vector<std::string>,
                     std::vector<std::string>,
                     std::vector<std::string> );

   explicit SGuiPackCaseFull( EGuiReply );

} GuiPackCaseFull_t;


typedef struct SGuiPackCaseDyna {

   EGuiReply                        getterReply;

   // each string in a vector formats its text as would carriage returns ("CR"), but CRs not included
   std::vector<std::string>         reportText_byCR;
   std::vector<std::string>         promptText_byCR;
   std::vector<std::string>         optionText_each;

   SGuiPackCaseDyna( std::vector<std::string>,
                     std::vector<std::string>,
                     std::vector<std::string> );

   explicit SGuiPackCaseDyna( EGuiReply );


} GuiPackCaseDyna_t;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

typedef struct SGuiPackHistogram {

   EGuiReply                        getterReply;   // c-tor defn inserts as "okay" unless error resulted
   EGuiType                         ownType;
   NGuiKey                          ownKey;
   EGuiType                         barTypeDisplayed;
   size_t                           numBarsDisplayed;
   std::vector<std::string>         captionText_byCR;
   std::vector<GuiFpn_t>            barHeights_leftToRight;
   GuiFpn_t                         leftEndBarNumericLabel_nanIfBarsNotAnalog;
   GuiFpn_t                         eachBarNumericLabelIncr_nanIfBarsNotAnalog;
   std::vector<std::string>         barLabelsAsText_leftToRight_emptyIfBarsAnalog;
   std::vector<std::string>         modeOptionsText_each;
   std::vector<std::string>         spanOptionsText_each;
   size_t                           modeNow_index;
   size_t                           spanNow_index;
   std::vector<NGuiKey>             knobKeys;

   SGuiPackHistogram(   EGuiType,
                        NGuiKey,
                        EGuiType,
                        size_t,
                        std::vector<std::string>,
                        std::vector<GuiFpn_t>,
                        GuiFpn_t,
                        GuiFpn_t,
                        std::vector<std::string>,
                        std::vector<std::string>,
                        std::vector<std::string>,
                        size_t,
                        size_t,
                        std::vector<NGuiKey>
 );

   explicit SGuiPackHistogram( EGuiReply );

} GuiPackHistogram_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

typedef struct SGuiPackRuleKitFull {

   EGuiReply                                    getterReply;
   EGuiType                                     ownType;
   NGuiKey                                      ownKey;
   std::string                                  captionText;
   std::vector<NGuiKey>                         ruleKitKnobKeys;
   std::vector<std::string>                     ruleLabels_topToBottom;
   std::vector<std::string>                     ruleTexts_if_topToBottom;
   std::vector<std::string>                     ruleTexts_then_topToBottom;
   std::vector<EGuiState>                       ruleStates_topToBottom;
   std::vector<NGuiKey>                         ruleKnobKeys_topToBottom;
   std::vector<NGuiKey>                         ruleHistogramKeys_topToBottom;
   NGuiKey                                      ruleKitHistogramKey;
   NGuiKey                                      realtimeKronoKey_zeroIfNone;

   SGuiPackRuleKitFull( EGuiType,
                        NGuiKey,
                        std::string,
                        std::vector<NGuiKey>,
                        std::vector<std::string>,
                        std::vector<std::string>,
                        std::vector<std::string>,
                        std::vector<EGuiState>,
                        std::vector<NGuiKey>,
                        std::vector<NGuiKey>,
                        NGuiKey,
                        NGuiKey
   );

   explicit SGuiPackRuleKitFull( EGuiReply );

} GuiPackRuleKitFull_t;


typedef struct SGuiPackRuleKitDyna {

   EGuiReply                                    getterReply;
   std::vector<EGuiState>                       ruleStates_topToBottom;
   NGuiKey                                      realtimeKronoKey_zeroIfNone;

   SGuiPackRuleKitDyna( std::vector<EGuiState>,
                        NGuiKey );

   explicit SGuiPackRuleKitDyna( EGuiReply );

} GuiPackRuleKitDyna_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


typedef struct SGuiPackKronoFull {

   EGuiReply                     getterReply;
   EGuiType                      ownType;
   NGuiKey                       ownKey;
   std::string                   captionText;
   std::vector<NGuiKey>          knobKeys;
   std::vector<NGuiKey>          paneKeys_topToBottom;
   std::vector<time_t>           timestamps_olderToNewer;

   SGuiPackKronoFull(   EGuiType,
                        NGuiKey,
                        std::string,
                        std::vector<NGuiKey>,
                        std::vector<NGuiKey>,
                        std::vector<time_t> );

   explicit SGuiPackKronoFull( EGuiReply );

} GuiPackKronoFull_t;


typedef struct SGuiPackKronoDyna {

   EGuiReply                  getterReply;
   time_t                     timestampNow;

   explicit SGuiPackKronoDyna( time_t );

   explicit SGuiPackKronoDyna( EGuiReply );

} GuiPackKronoDyna_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

typedef struct SGuiPackPane {

   EGuiReply                     getterReply;
   EGuiType                      ownType;
   NGuiKey                       ownKey;
   std::string                   yAxisUnitsText;
   GuiFpn_t                      yAxisMin;
   GuiFpn_t                      yAxisMax;
   std::vector<NGuiKey>          traceKeys;

   SGuiPackPane(  EGuiType,
                  NGuiKey,
                  std::string,
                  GuiFpn_t,
                  GuiFpn_t,
                  std::vector<NGuiKey> );

   explicit SGuiPackPane( EGuiReply );

} GuiPackPane_t;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


typedef struct SGuiPackTraceFull {

   EGuiReply                     getterReply;   // c-tor defn inserts as "okay" unless error resulted
   EGuiType                      ownType;
   NGuiKey                       ownKey;
   std::string                   tag;
   std::vector<GuiFpn_t>         numbers_olderToNewer;
   std::vector<EGuiState>        states_olderToNewer;
   NGuiKey                       sourceHistogramKey;
   std::vector<NGuiKey>          knobKeys;


   SGuiPackTraceFull(   EGuiType,
                        NGuiKey,
                        std::string,
                        std::vector<GuiFpn_t>,
                        std::vector<EGuiState>,
                        NGuiKey,
                        std::vector<NGuiKey> );

   explicit SGuiPackTraceFull( EGuiReply );

} GuiPackTraceFull_t;


typedef struct SGuiPackTraceDyna {

   EGuiReply                     getterReply;
   GuiFpn_t                      numberNow;
   EGuiState                     stateNow;

   explicit SGuiPackTraceDyna(   GuiFpn_t,
                                 EGuiState );

   explicit SGuiPackTraceDyna( EGuiReply );

} GuiPackTraceDyna_t;



typedef struct SGuiPackSubjectBasic {

   EGuiReply                                 getterReply;
   EGuiType                                  ownType;
   NGuiKey                                   ownKey;
   NGuiKey                                   hostDomainKey;
   std::string                               ownNameText;
   std::vector<std::string>                  infoText_byCR; // Getter makes subj label the 1st line
   std::vector<NGuiKey>                      featureKeys;
   std::vector<NGuiKey>                      paramKnobKeys;
   std::vector<NGuiKey>                      ruleKitKeys;

   SGuiPackSubjectBasic(   EGuiType,
                           NGuiKey,
                           NGuiKey,
                           std::string,
                           std::vector<std::string>,
                           std::vector<NGuiKey>,
                           std::vector<NGuiKey>,
                           std::vector<NGuiKey>
   );

   explicit SGuiPackSubjectBasic( EGuiReply );


} GuiPackSubjectBasic_t;



typedef struct SGuiPackSubjectCases {

   EGuiReply                       getterReply;
   std::vector<NGuiKey>            currentCaseKeys;
   std::vector<std::string>        currentCaseNames;

   SGuiPackSubjectCases(   std::vector<NGuiKey>,
                           std::vector<std::string> );

   explicit SGuiPackSubjectCases( EGuiReply );

} GuiPackSubjectCases_t;

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
