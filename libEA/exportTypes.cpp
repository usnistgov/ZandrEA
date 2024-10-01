// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements customized types exported to be used by EA front end
   Principal contents: "Key" type for use by runtime API, Enumerated types, "GUI pack" structs
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "customTypes.hpp"
#include <assert.h>

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Implementation for NGuiKey

NGuiKey::NGuiKey( void )
                  : key (0ull) {
}


NGuiKey::~NGuiKey( void ) {      // Nothing here, but by Rule-of-5 compiler might need this regardless

}


NGuiKey::NGuiKey( unsigned long long set )
                  : key (set) {
}


NGuiKey::NGuiKey( const NGuiKey& srcRef )
                  : key ( srcRef.key ) {
}


NGuiKey::NGuiKey( NGuiKey&& srcRvalueRef )
                  : key ( std::move(srcRvalueRef.key) ) {

      srcRvalueRef.key = 0ull;   // not doing this caused Trouble, believe me!
}


NGuiKey& NGuiKey::operator=( const NGuiKey& rhsLvalueRef ) {

//Alternative (?)
//   The operator= uses copy constructor to build a new object, which will get exchanged with *this
//   and released (with the old this inside) at function exit.
//      std::swap( key, rhs.key );
//      return *this;

   key = rhsLvalueRef.key;
   return *this;
}

NGuiKey& NGuiKey::operator=( NGuiKey&& rhsRvalueRef ) {

   key = rhsRvalueRef.key;
   rhsRvalueRef.key = 0ull;
   return *this;
}


bool operator<( NGuiKey lhs, NGuiKey rhs ) { return lhs.key < rhs.key; }
 
bool operator==( NGuiKey lhs, NGuiKey rhs ) { return lhs.key == rhs.key; }

bool operator!=( NGuiKey lhs, NGuiKey rhs ) { return lhs.key != rhs.key; }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
//  "Typed" structures used for interactivity between View and Client (GUI)

SGuiPackDomain::SGuiPackDomain(  NGuiKey arg1,
                                 EGuiType arg2,
                                 std::string arg3,
                                 std::vector<NGuiKey> arg4 )
                                 :  getterReply( EGuiReply::OKAY_allDone ),
                                    ownKey (arg1),
                                    ownType (arg2),
                                    ownNameText (arg3),
                                    subjectKeys( arg4 ) {
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// this c-tor called when Reply is OKAY

SGuiPackFeatureFull::SGuiPackFeatureFull( EGuiType arg1,
                                          NGuiKey arg2,
                                          GuiUin_t arg3,
                                          std::string arg4,
                                          std::string arg5,
                                          std::string arg6,
                                          EGuiState arg7,
                                          NGuiKey arg8,
                                          std::vector<NGuiKey> arg9 )
                                          :  getterReply (EGuiReply::OKAY_allDone),
                                             ownType (arg1),
                                             ownKey (arg2),
                                             featureUai (arg3),
                                             labelText (arg4),
                                             unitsText (arg5),
                                             messageText (arg6),
                                             messageState (arg7),
                                             sourceHistogramKey (arg8),
                                             ownKnobKeys (arg9) {
}

// this c-tor called when Reply is a FAIL mode

SGuiPackFeatureFull::SGuiPackFeatureFull( EGuiReply arg0 )
                                          :  getterReply( arg0 ),
                                             ownType (EGuiType::Undefined),
                                             ownKey (0),
                                             featureUai (0),
                                             labelText (""),
                                             unitsText (""),
                                             messageText (""),
                                             messageState (EGuiState::Undefined),
                                             sourceHistogramKey (0),
                                             ownKnobKeys(0) {
}


SGuiPackFeatureDyna::SGuiPackFeatureDyna( std::string arg6,   // c-tor called when Reply is OKAY
                                          EGuiState arg7 )
                                          :  getterReply (EGuiReply::OKAY_allDone),
                                             messageText (arg6),
                                             messageState (arg7) {
}

SGuiPackFeatureDyna::SGuiPackFeatureDyna(   EGuiReply arg0 )    // c-tor called when Reply is a FAIL mode
                                          :  getterReply ( arg0 ),
                                             messageText (""),
                                             messageState (EGuiState::Undefined) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackKnob::SGuiPackKnob(   EGuiType arg1,
                              NGuiKey arg2,    // c-tor called when Reply is OKAY
                              std::string arg3,
                              std::string arg4,
                              std::vector<GuiFpn_t> arg5,
                              std::vector<GuiFpn_t> arg6,
                              GuiFpn_t arg7 )
                              :  getterReply (EGuiReply::OKAY_allDone),
                                 ownType (arg1),
                                 ownKey (arg2),
                                 labelText (arg3),
                                 unitsText (arg4),
                                 rangeMinMax_emptyIfBool (arg5),
                                 definedSelection_emptyIfNA (arg6),
                                 valueNow_numerIfBool (arg7) {
}


SGuiPackKnob::SGuiPackKnob(   EGuiReply arg0 )    // c-tor called when Reply is a FAIL mode
                              :  getterReply ( arg0 ),
                                 ownType (EGuiType::Undefined),
                                 ownKey (0),
                                 labelText (""),
                                 unitsText (""),
                                 rangeMinMax_emptyIfBool(),
                                 definedSelection_emptyIfNA(),
                                 valueNow_numerIfBool (NaNDBL) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackCaseFull::SGuiPackCaseFull( EGuiType arg1,
                                    NGuiKey arg2,    // c-tor called when Reply is OKAY
                                    NGuiKey arg3,
                                    std::string arg4,
                                    std::vector<std::string> arg5,
                                    std::vector<std::string> arg6,
                                    std::vector<std::string> arg7 )
                                    :  getterReply (EGuiReply::OKAY_allDone),
                                       ownType (arg1),
                                       ownKey (arg2),
                                       snapshotKronoKey (arg3),
                                       caseName (arg4),
                                       reportText_byCR (arg5), 
                                       promptText_byCR (arg6),
                                       optionText_each (arg7) {
}

SGuiPackCaseFull::SGuiPackCaseFull( EGuiReply arg0 )    // c-tor called when Reply is a FAIL mode
                                    :  getterReply (arg0),
                                       ownType (EGuiType::Undefined),
                                       ownKey (0),
                                       snapshotKronoKey (0),
                                       caseName (""),
                                       reportText_byCR (),
                                       promptText_byCR (),
                                       optionText_each (0) {
}

SGuiPackCaseDyna::SGuiPackCaseDyna( std::vector<std::string> arg5,
                                    std::vector<std::string> arg6,
                                    std::vector<std::string> arg7)
                                    :  getterReply (EGuiReply::OKAY_allDone),
                                       reportText_byCR (arg5),
                                       promptText_byCR (arg6),
                                       optionText_each (arg7) {
}

SGuiPackCaseDyna::SGuiPackCaseDyna( EGuiReply arg0 )
                                    :  getterReply (arg0),
                                       reportText_byCR(),
                                       promptText_byCR(),
                                       optionText_each(0) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackHistogram::SGuiPackHistogram(  EGuiType arg1,    // c-tor called when Reply is OKAY
                                       NGuiKey arg2,
                                       EGuiType arg3,
                                       size_t arg4,
                                       std::vector<std::string> arg5,
                                       std::vector<GuiFpn_t> arg6,
                                       GuiFpn_t arg7,
                                       GuiFpn_t arg8,
                                       std::vector<std::string> arg9,
                                       std::vector<std::string> arg10,
                                       std::vector<std::string> arg11,
                                       size_t arg12,
                                       size_t arg13,
                                       std::vector<NGuiKey> arg14  )
                                       :  getterReply (EGuiReply::OKAY_allDone),
                                          ownType (arg1),
                                          ownKey (arg2),
                                          barTypeDisplayed (arg3),
                                          numBarsDisplayed (arg4),
                                          captionText_byCR (arg5),
                                          barHeights_leftToRight (arg6),
                                          leftEndBarNumericLabel_nanIfBarsNotAnalog (arg7),
                                          eachBarNumericLabelIncr_nanIfBarsNotAnalog (arg8),
                                          barLabelsAsText_leftToRight_emptyIfBarsAnalog (arg9),
                                          modeOptionsText_each (arg10),
                                          spanOptionsText_each (arg11),
                                          modeNow_index (arg12),
                                          spanNow_index (arg13),
                                          knobKeys (arg14) {
}

SGuiPackHistogram::SGuiPackHistogram(  EGuiReply arg0 )    // c-tor called when Reply is a FAIL mode
                                       :  getterReply (arg0),
                                          ownType (EGuiType::Undefined),
                                          ownKey (0),
                                          barTypeDisplayed (EGuiType::Undefined),
                                          numBarsDisplayed (0u),
                                          captionText_byCR (0),
                                          barHeights_leftToRight (0),
                                          leftEndBarNumericLabel_nanIfBarsNotAnalog (NaNUINT),
                                          eachBarNumericLabelIncr_nanIfBarsNotAnalog (NaNUINT),
                                          barLabelsAsText_leftToRight_emptyIfBarsAnalog (0),
                                          modeOptionsText_each (0),
                                          spanOptionsText_each (0),
                                          modeNow_index (NaNSIZE),
                                          spanNow_index (NaNSIZE),
                                          knobKeys (0) {
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

SGuiPackRuleKitFull::SGuiPackRuleKitFull( EGuiType arg1,
                                          NGuiKey arg2,
                                          std::string arg3,
                                          std::vector<NGuiKey> arg4,
                                          std::vector<std::string> arg5,
                                          std::vector<std::string> arg6,
                                          std::vector<std::string> arg7,
                                          std::vector<EGuiState> arg8,
                                          std::vector<NGuiKey> arg9,
                                          std::vector<NGuiKey> arg10,
                                          NGuiKey arg11,
                                          NGuiKey arg12 )
                                          :  getterReply (EGuiReply::OKAY_allDone),
                                             ownType (arg1),
                                             ownKey (arg2),
                                             captionText (arg3),
                                             ruleKitKnobKeys (arg4),      
                                             ruleLabels_topToBottom (arg5),
                                             ruleTexts_if_topToBottom (arg6),
                                             ruleTexts_then_topToBottom (arg7),
                                             ruleStates_topToBottom (arg8),
                                             ruleKnobKeys_topToBottom (arg9),
                                             ruleHistogramKeys_topToBottom (arg10),
                                             ruleKitHistogramKey (arg11),
                                             realtimeKronoKey_zeroIfNone (arg12) {

}


SGuiPackRuleKitFull::SGuiPackRuleKitFull( EGuiReply arg0 )
                                          :  getterReply (arg0),
                                             ownType (EGuiType::Undefined),
                                             ownKey (0),
                                             captionText (""),
                                             ruleKitKnobKeys(0),
                                             ruleLabels_topToBottom(0),
                                             ruleTexts_if_topToBottom(0),
                                             ruleTexts_then_topToBottom(0),
                                             ruleStates_topToBottom(0),
                                             ruleKnobKeys_topToBottom(0),
                                             ruleHistogramKeys_topToBottom(0),
                                             ruleKitHistogramKey (NGuiKey(0)),
                                             realtimeKronoKey_zeroIfNone (NGuiKey(0)) {

}


SGuiPackRuleKitDyna::SGuiPackRuleKitDyna(   std::vector<EGuiState> arg8,
                                            NGuiKey arg11 )
                                            :   getterReply (EGuiReply::OKAY_allDone),
                                                ruleStates_topToBottom (arg8),
                                                realtimeKronoKey_zeroIfNone (arg11) {
}


SGuiPackRuleKitDyna::SGuiPackRuleKitDyna( EGuiReply arg0 )
                                          :  getterReply (arg0),
                                             ruleStates_topToBottom(0),
                                             realtimeKronoKey_zeroIfNone (NGuiKey(0)) {
}



//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackKronoFull::SGuiPackKronoFull(  EGuiType arg1,
                                       NGuiKey arg2,
                                       std::string arg3,
                                       std::vector<NGuiKey> arg4,
                                       std::vector<NGuiKey> arg5,
                                       std::vector<time_t> arg6 )
                                       :  getterReply (EGuiReply::OKAY_allDone),
                                          ownType (arg1),
                                          ownKey (arg2),
                                          captionText (arg3),
                                          knobKeys (arg4),
                                          paneKeys_topToBottom (arg5),
                                          timestamps_olderToNewer (arg6) {
}


SGuiPackKronoFull::SGuiPackKronoFull( EGuiReply arg0 )
                                    :  getterReply ( arg0 ),
                                       ownType (EGuiType::Undefined),
                                       ownKey (0),
                                       captionText(""),
                                       knobKeys(0),
                                       paneKeys_topToBottom(0),
                                       timestamps_olderToNewer(0) {
}


SGuiPackKronoDyna::SGuiPackKronoDyna( time_t arg)
                                    :  getterReply (EGuiReply::OKAY_allDone),
                                       timestampNow (arg) {
}


SGuiPackKronoDyna::SGuiPackKronoDyna( EGuiReply arg0 )
                                    :  getterReply( arg0 ),
                                       timestampNow(0) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackPane::SGuiPackPane(   EGuiType arg1,
                              NGuiKey arg2,
                              std::string arg3,
                              GuiFpn_t arg4,
                              GuiFpn_t arg5,
                              std::vector<NGuiKey> arg6 )
                              :  getterReply (EGuiReply::OKAY_allDone),
                                 ownType (arg1),
                                 ownKey (arg2),
                                 yAxisUnitsText (arg3),
                                 yAxisMin (arg4),
                                 yAxisMax (arg5),
                                 traceKeys(arg6) {
}


SGuiPackPane::SGuiPackPane(  EGuiReply arg0 )
                           :  getterReply (arg0),
                              ownType (EGuiType::Undefined),
                              ownKey (0),
                              yAxisUnitsText(""),
                              yAxisMin (NaNDBL),
                              yAxisMax (NaNDBL),
                              traceKeys(0) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/


SGuiPackTraceFull::SGuiPackTraceFull(  EGuiType arg1,
                                       NGuiKey arg2,
                                       std::string arg3,
                                       std::vector<GuiFpn_t> arg4,
                                       std::vector<EGuiState> arg5,
                                       NGuiKey arg6,
                                       std::vector<NGuiKey> arg7 )
                                       :  getterReply (EGuiReply::OKAY_allDone),
                                          ownType (arg1),
                                          ownKey (arg2),
                                          tag (arg3),
                                          numbers_olderToNewer (arg4),
                                          states_olderToNewer (arg5),
                                          sourceHistogramKey (arg6),
                                          knobKeys(arg7) {
}


SGuiPackTraceFull::SGuiPackTraceFull(  EGuiReply arg0 )
                                       :  getterReply( arg0 ),
                                          ownType (EGuiType::Undefined),
                                          ownKey (0),
                                          tag (""),
                                          numbers_olderToNewer(0),
                                          states_olderToNewer(0),
                                          sourceHistogramKey (NGuiKey(0)),
                                          knobKeys(0) {
}


SGuiPackTraceDyna::SGuiPackTraceDyna(  GuiFpn_t arg1,
                                       EGuiState arg2 )
                                       :  getterReply (EGuiReply::OKAY_allDone),
                                          numberNow (arg1),
                                          stateNow (arg2) {
}


SGuiPackTraceDyna::SGuiPackTraceDyna(  EGuiReply arg0 )
                                       :  getterReply( arg0 ),
                                          numberNow(NaNDBL),
                                          stateNow (EGuiState::Undefined) {
}


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

SGuiPackSubjectBasic::SGuiPackSubjectBasic(  EGuiType arg1,
                                             NGuiKey arg2,
                                             NGuiKey arg3,
                                             std::string arg4,
                                             std::vector<std::string> arg5,
                                             std::vector<NGuiKey> arg6,
                                             std::vector<NGuiKey> arg7,
                                             std::vector<NGuiKey> arg8 )
                                             :  getterReply (EGuiReply::OKAY_allDone),
                                                ownType (arg1),
                                                ownKey (arg2),
                                                hostDomainKey (arg3),
                                                ownNameText (arg4),
                                                infoText_byCR (arg5),
                                                featureKeys (arg6),
                                                paramKnobKeys (arg7),
                                                ruleKitKeys (arg8) {
}

SGuiPackSubjectBasic::SGuiPackSubjectBasic(  EGuiReply arg2 )
                                             :  getterReply (arg2),
                                                ownType (EGuiType::Undefined),
                                                ownKey (0),
                                                hostDomainKey (0),
                                                ownNameText (""),
                                                infoText_byCR (0),
                                                featureKeys (0),
                                                paramKnobKeys (0),
                                                ruleKitKeys (0) {
}



SGuiPackSubjectCases::SGuiPackSubjectCases(  std::vector<NGuiKey> arg0,
                                             std::vector<std::string> arg1 )
                                             :  getterReply ( EGuiReply::OKAY_allDone ),
                                                currentCaseKeys (arg0),
                                                currentCaseNames (arg1) {
}


SGuiPackSubjectCases::SGuiPackSubjectCases(  EGuiReply arg )
                                 :  getterReply (arg),
                                    currentCaseKeys(0),
                                    currentCaseNames(0) {
}


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
