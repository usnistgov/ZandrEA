// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements CRule concrete class for all Rule objects 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "rule.hpp"

#include "rainfall.hpp"
#include "controlParts.hpp"      // rule kit and r-t krono knobs
#include "state.hpp"             // call methods on rule inputs (facts)
#include "subject.hpp"           // call getters on subject
#include "case.hpp"              // call CreateCase() in rule kit cycle
#include "agentTask.hpp"         // register rule kit to sequence
#include "knowBase.hpp"          // call d-tor on CKnowBaseH5 u-pointer
#include "knowParts.hpp"         // call CHypo to add nodes to knowledge base
#include "viewParts.hpp"

#include <algorithm>
#include <numeric>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CRule

// Static memory

Nzint_t CRule::sgiForNextSnapshotSet = 1u; // All CRule objects share one common pool of snapshot IDs

Nzint_t CRule::GenerateSgiForNewSnapshotSet( void ) {

   // unsigned ints do not overflow badly but simply wrap back to zero
   return sgiForNextSnapshotSet++;
}  

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// C-tor and d-tor

CRule::CRule(  CController& arg0,  
               CRuleKit& arg1,
               Nzint_t arg2,
               EAlertMsg arg3,
               EAlertMsg arg4,
               EAlertMsg arg5,
               EAlertMsg arg6,
               std::vector<AFact*> arg7,
               std::vector<AFact*> arg8,
               std::function<bool(void)> arg9,
               std::function<bool(void)> arg10,
               bool arg11 )
            // std::function<int (const CRule&, const SEnergyCosts&)>
               :  RuleKitRef (arg1),
                  u_Knob (nullptr),
                  p_AssocHypo(),
                  p_OperandsIf (arg7),
                  p_OperandsThen (arg8),
                  TestIf (arg9),
                  TestThen (arg10),
                  EstmCostOfFault (
                     std::function<int( const CRule&, const SEnergyPrices& )>
                        ( []( const CRule& lArg0, const SEnergyPrices& lArg1 ) -> int
                           { return -1; }  // so, rtns -1 as cost when no cost estimate is available
                        )
                  ),
                  p_RtTracesOfAntecedents_byKey( TabulateRealtimeAccessToAntecedents() ),
                  knobKeys_ownedAndAntecedent(0),
                  assocHypoUai(),
                  msg_ruleFocus (arg3),
                  msg_IfTest (arg4),
                  msg_ThenTest (arg5),
                  msgUponFailure (arg6), 
                  numOperandsIf ( arg7.size() ),
                  numOperandsThen ( arg8.size() ),
                  caseModeOffset (0u),
                  idleModeOffset (0u), // not in idleMode, not in caseMode, so starting in autoMode
                  ruleUai (arg2),
                  snapshotSetSgi (0u),
                  bindexNow (NaNBINDEX),
                  holdingSnapshots (false),
                  resultIf (NaNBOOL),
                  resultThen (NaNBOOL),
                  isRuleAtIdle (false),
                  isRulePinnedToUnitOutput (arg11),
                  valid (NaNBOOL) {

   u_Knob = std::make_unique<CKnobBool>(  arg0,
                                          arg1,
                                          EDataLabel::Knob_rule_idleMode,
                                          EDataSuffix::None,
                                          (  [&]( bool userInputVetted ) -> void {
                                                // Operand set takes bool
                                                SetIdleModeTo( userInputVetted );
                                                return;
                                             } 
                                          ),
                                          isRuleAtIdle
   ); 

   knobKeys_ownedAndAntecedent.push_back( u_Knob->SayGuiKey() ); // $$$ TBD antecedents wanted here $$$
   arg1.AddRuleToKit( this );
}
 

CRule::~CRule( void ) { /* empty */ }


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CRule private methods


bool CRule::PullValidity( void ) {

   bool update = ( p_OperandsIf.at(0)->IsValid() && p_OperandsThen.at(0)->IsValid() );

   for (size_t i=1; i<numOperandsIf; ++i) {
      update = ( update && p_OperandsIf.at(i)->IsValid() );
   }
   for (size_t i=1; i<numOperandsThen; ++i) {
      update = ( update && p_OperandsThen.at(i)->IsValid() );
   }
   valid = update;
   return valid; 
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CRule Public Methods

const RtTraceAccessTable_t& CRule::SayRealtimeAccessTable( void ) const {

   return p_RtTracesOfAntecedents_byKey;
}


const std::vector<NGuiKey>& CRule::SayCrefToKnobKeys( void ) const {

   return knobKeys_ownedAndAntecedent;
}


std::string CRule::SayRuleUaiText( void ) const {

   return ( "Rule-" + std::to_string( ruleUai ) );
}

//======================================================================================================/

EAlertMsg CRule::SayFocusMsg( void ) const { return msg_ruleFocus; }

EAlertMsg CRule::SayTest_If( void ) const { return msg_IfTest; }

EAlertMsg CRule::SayTest_Then( void ) const { return msg_ThenTest; }

EAlertMsg CRule::SayFailureMsg( void ) const { return msgUponFailure; }

//======================================================================================================/

Nzint_t CRule::SaveAntecedentSnapshotsAndSaySetSgi( void ) {

   snapshotSetSgi = GenerateSgiForNewSnapshotSet();

   for ( auto pairByValue : p_RtTracesOfAntecedents_byKey ) {

      pairByValue.second->CaptureSnapshotForSetSgi( snapshotSetSgi );
   }
   return snapshotSetSgi;
}

//======================================================================================================/

Nzint_t CRule::SayRuleUai( void ) const { return ruleUai; }


NGuiKey CRule::SayKeyToOwnKnob( void ) const { return u_Knob->SayGuiKey(); }


Nzint_t CRule::SaySnapshotSetSgi( void ) const { return snapshotSetSgi; }


Bindex_t CRule::CycleAndSayBindex( void ) {

   /* Bindex values resulting from cycling a CRule object:
      Note:
         Rules are left by User in "auto mode" (can create a new CCase) or put in "idle mode" (rule
         itself continues showing results and logging them to its Rainfall, but an "idle" CRule cannot
         create new cases [e.g., "idle" is used for upstream parameter tuning, etc.]). So, Rules are
         ALWAYS "on" (i.e., generating results from "valid" data given to them by AFact operand objects).
         Rule "valid" is 'true' or 'false' based on return of PullValidity() from operand AFact objects.
         "Valid" reflects only on data input to EA, not on quality of parameter settings within it.
 
         A Rule in "auto mode" is put into "case mode" by the CCase object it creates, put back into
         "auto" upon CCase destructor.  Case mode prevents further (i.e., duplicate) cases on same Rule.
      So:
         inputs valid, auto mode, 'true' on If-stmt, 'false' on Then-stmt         -> "autoModeFail" = 0
         inputs valid, auto mode, 'true' on If-stmt, 'true' on Then-stmt          -> "autoModePass" = 1
         inputs valid, auto mode, 'false' on If-stmt, Then-stmt not tested        -> "autoModeSkip" = 2

         at least one input invalid, rule goes untested                           -> "dataInvalid"  = 3
 
         inputs valid, case mode, input 'true' on If-stmt,'false' on Then-stmt    -> "caseModeFail" = 4
         inputs valid, case mode, input 'true' on If-stmt,'true' on Then-stmt     -> "caseModePass" = 5
         inputs valid, case mode, input 'false' on If-stmt, Then-stmt not tested  -> "caseModeSkip" = 6

         inputs valid, idle mode, 'true' on If-stmt,'false' on Then-stmt          -> "idleModeFail" = 7
         inputs valid, idle mode, 'true' on If-stmt,'true' on Then-stmt           -> "idleModePass" = 8
         inputs valid, idle mode, 'false' on If-stmt, Then-stmt not tested        -> "idleModeSkip" = 9

         input data unavailable (e.g., rainfall backfilling upon start-up)        -> "unavailable" = 10 
   */

   return bindexNow =   (  PullValidity() ?
                           (  TestIf() ?
                              (  TestThen() ?
                                    static_cast<Bindex_t>( 1u + idleModeOffset + caseModeOffset ) :
                                    static_cast<Bindex_t>( 0u + idleModeOffset + caseModeOffset )
                              ) :
                              static_cast<Bindex_t>( 2u + idleModeOffset + caseModeOffset )
                           ) :
                           static_cast<Bindex_t>( 3u )
                        );
}


bool CRule::IsPinnedToUnitOutput( void ) const { return isRulePinnedToUnitOutput; }


bool CRule::HasDiagnostics( void ) const { return !p_AssocHypo.empty(); }


bool CRule::HasSnapshotSet( void ) const { return holdingSnapshots; }


bool CRule::IsInAutoMode( void ) const { return ( (caseModeOffset + idleModeOffset) == 0u ); }


void CRule::LendHistogramKeysOfAntecedentsTo( std::vector<NGuiKey>& borrowerRef ) const {

   for ( auto ptr : p_OperandsIf ) { ptr->LendHistogramKeysTo( borrowerRef ); }

   for ( auto ptr : p_OperandsThen ) { ptr->LendHistogramKeysTo( borrowerRef ); }

   return;
}


void CRule::SetCaseModeTo( bool doesCaseWantRuleInCaseMode ) {

/* Only Case constructors call this method with arg = TRUE, as they themselves aren't called unless rule
   is in "auto mode" (i.e., not at "idle").  Only Case destructors call it with arg = FALSE.
*/  
   caseModeOffset = ( doesCaseWantRuleInCaseMode ? 4u : 0u );
   return;
}


void CRule::SetIdleModeTo( bool doesUserWantRuleInIdleMode ) {

   idleModeOffset = ( ( doesUserWantRuleInIdleMode ?
                           ( ( caseModeOffset != 0u ) ?
                              0u :
                              7u ) :      // Rule only goes to "idle" if not already in "case mode"
                           0u )
   );
   isRuleAtIdle = ( idleModeOffset == 7u );
   return;
}


//======================================================================================================/

RtTraceAccessTable_t CRule::TabulateRealtimeAccessToAntecedents( void ) {
   
   RtTraceAccessTable_t keyToPtrTable;

   for ( auto p_fact : p_OperandsIf ) {

      p_fact->LendRealtimeAccessTo( keyToPtrTable );
   }
   for ( auto p_fact : p_OperandsThen ) {

      p_fact->LendRealtimeAccessTo( keyToPtrTable );
   }
   return keyToPtrTable;
}

//======================================================================================================/

EApiReply CRule::DestroySnapshotSet( void ) {

   EApiReply reply = EApiReply::Okay_tallyZero;

   if ( caseModeOffset == 0u ) {

      for ( auto pairByValue : p_RtTracesOfAntecedents_byKey ) {

         pairByValue.second->DestroySnapshotForSetSgi( snapshotSetSgi );
      }
      reply = EApiReply::Okay_tallyOne;
   }   
   /* The CRule's own snapshot held by CRainKit, so must be cleared by separate action of caller (kit).
      That action also re-zeros the CRule snapshotSetId field.
   */
   return reply;
}

//======================================================================================================/

void CRule::AssociateHypo( CHypo* const p_Hypo ) {

   p_AssocHypo.push_back( p_Hypo );

   assocHypoUai.push_back( p_Hypo->SayId() );

   return;
}

//======================================================================================================/

void CRule::BuildRuleIntoKbase( CKnowBaseH5& KbaseRef ) { 

   for (auto pp_AssocHypo=p_AssocHypo.begin(); pp_AssocHypo!=p_AssocHypo.end(); ++pp_AssocHypo) {

      (*pp_AssocHypo)->BuildRuleAndHypoIntoKbase( ruleUai, assocHypoUai, KbaseRef );
   }
   return;
}

//======================================================================================================/

int CRule::SayDollarPerDayFaultCost( const SEnergyPrices& CostRef) {
/*
   Call a lambda defined at runtime for each CRule object meant to have cost estimation capability
   (i.e., not defaulting to return a -1).
*/
   return EstmCostOfFault( *this, CostRef );
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CRuleKit

CRuleKit::CRuleKit(  CSequence& bArg0,
                     ASubject& bArg1,
                     EDataLabel bArg2,    // Descriptive kit name  (e.g., "Short-span" or "Long-span")
                     int arg0,            // CRainRuleKit trap depth in secs (more intuitive than cycles)
                     CController& arg1,
                     int bArg3 )          // tpc applied to kit and CRule objects in kit, default = 1  
                     :  ISeqElement(   bArg0,
                                       bArg1,
                                       EApiType::RuleKit,
                                       bArg2,
                                       EDataUnit::Bindex_rule, // these 4 vvv apply to ea rule, not kit
                                       EDataRange::Bindex_rule,
                                       EDataSuffix::None,
                                       EPlotGroup::Alone,
                                       bArg3,
                                       BASETRIGGRP_RULEKIT
                        ),
                        u_RainRuleKit (   std::make_unique<CRainRuleKit>(
                                          *this,
                                          knobKeys_ownedAndAntecedent,
                                          bArg1,
                                          static_cast<size_t>( std::max( arg0/secsPerCycle, 1 ) ) )
                        ),
                        u_Kbase ( std::make_unique<CKnowBaseH5>(
                           bArg1.SayRootTextForDiskFilenames() + "kbase.h5" )
                        ),
                        u_GuiDisplay ( std::make_unique<CDisplayRuleKit>(
                                          bArg1,
                                          *this,
                                          bArg1.SayViewRef() )
                        ),
                        p_Rules_byUai(0),
                        u_RealtimeTracesForRulesInKit_byUai(),
                        CaseKitRef ( bArg1.SayCaseKitRef() ),
                        CtrlrRef (arg1),
                        ViewRef ( bArg1.SayViewRef() ),
                        p_KnobSelectingRuleUai (nullptr),
                        p_TracesInRealtimeKrono_byKey(),
                        u_PanesInRealtimeKrono_byKey(),
                        u_RealtimeKrono(),
                        ruleUais_guiTopToBottom(0),
                        knobKeysOfRuleKitItself(0),
                        knobKeyOfEachRuleInKit_guiTopToBottom(0),
                        histogramKeyOfEachRuleInKit_guiTopToBottom(0),
                        kitSgiFromSubject ( bArg1.GenerateAndSaySgiForNewRuleKit() ),
                        uaiOfRuleInRtKrono_zeroIfNoneOrAll (0u),
                        areAllRulesNotInCaseModePutToIdle (false),
                        isRealtimeKronoShowingAllRules (false),
                        kitFinalized (false) {

   CalcOwnTriggerGroup();
   bArg0.Register(this);
   AttachOwnKnobs( arg1 );
   LendKnobKeysTo( knobKeys_ownedAndAntecedent );

/* Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
 [1]  C-tor currently "floors" to minumum (i.e fastest) practical value any impractical inputs for
      seconds per rule cycle and for rule trap depth.  That was done instead of adding check code for
      throwing exceptions.  Consider for refactoring later. 

'' End Method Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/
}


CRuleKit::~CRuleKit( void ) {

   // Empty CRuleKit dtor
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CRuleKit Private/Protected Methods


std::string CRuleKit::GenerateKronoCaption_OneRule( Nzint_t uaiOfRule ) const {

// No const string initially available to object, as number of rule kits on a subject is runtime info

   return ( SubjRef.SayDomainAndOwnNameAsText() +
            " Rule Kit #" + std::to_string( kitSgiFromSubject ) +
            " Rule-" + std::to_string( uaiOfRule )  +
            " at " + std::to_string( secsPerCycle ) + "secs/cycle" );
}


std::string CRuleKit::GenerateKronoCaption_AllRulesInKit( void ) const {

// No const string initially available to object, as number of rule kits on a subject is runtime info

   return ( SubjRef.SayDomainAndOwnNameAsText() +
            " Rule Kit #" + std::to_string( kitSgiFromSubject ) +
            " All Rules" +
            " at " + std::to_string( secsPerCycle ) + "secs/cycle" );
}


//======================================================================================================/

void CRuleKit::AttachOwnKnobs( CController& ctrlrRef ) {

  /* AKnob subclasses test GUI input to range allowed prior to calling setter lambdas, so value
     given by User either gets ignored with O.O.R. error sent, or arrives at lambda arg as "vetted" */  

   u_KnobsOwned.push_back( std::make_unique<CKnobBool>(                           
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_ruleKit_idleModeAll,
                              EDataSuffix::None,
                              (  [&]( bool userInputVetted ) -> void {

                                    IdleAllRulesNotInCaseMode( userInputVetted );
                                    return; } 
                              ),
                              areAllRulesNotInCaseModePutToIdle )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobBool>(                           
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_ruleKit_loadRtKrono_resultsOfAllRules,
                              EDataSuffix::None,
                              (  [&]( bool userInputVetted ) -> void {

                                    LoadRealtimeKronoWithResultsFromAllRulesInKit( userInputVetted );
                                    return; } 
                              ),
                              isRealtimeKronoShowingAllRules )
   );

   u_KnobsOwned.push_back( std::make_unique<CKnobSelectNzint>(                           
                              ctrlrRef,
                              *this,
                              EDataLabel::Knob_ruleKit_loadRtKrono_logicOfOneRule,
                              EDataUnit::Identifier_ruleUai,
                              (  [&]( Nzint_t ruleUai_vettedToSelectionSetOnly ) -> EGuiReply {
                                    return   LoadRealtimeKronoWithLogicChainOfRule(
                                       ruleUai_vettedToSelectionSetOnly // i.e., could be 0 (null)
                                    );
                                 }
                              ),
                              uaiOfRuleInRtKrono_zeroIfNoneOrAll )
   );

// Later, need ptr specifically to CKnobSelectNzint obj so to load in its selections after kit finalized
   p_KnobSelectingRuleUai = u_KnobsOwned.back().get();

// Load Keys to all Rule Kit Knobs into field sent off in GuiPack
   for ( auto& u_KnobRef : u_KnobsOwned ) {

      knobKeysOfRuleKitItself.push_back( u_KnobRef->SayGuiKey() );
   }

   return;
}

//======================================================================================================/

EGuiReply CRuleKit::LoadRealtimeKronoWithLogicChainOfRule(  Nzint_t uaiOfRule ) {

// Clear Realtime Krono of any existing content
 
    u_RealtimeKrono.reset( nullptr );  // R-t Krono d-tor calls ClearKitOfRealtimeKronoParts()

   if ( uaiOfRule == 0 ) { return EGuiReply::OKAY_allDone; } 

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Load realtime Trace access from Rule. Declare locals used later when arranging Panes on Krono

   p_TracesInRealtimeKrono_byKey = p_Rules_byUai[uaiOfRule]->SayRealtimeAccessTable();
   
   std::vector<CPaneRealtime*> p_analogPanesCreated(0);
   std::vector<CPaneRealtime*> p_factPanesCreated(0);
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Load into Trace access table (by emplace) raw pointer to Trace of given Rule
   $$$ unlike ISeqElement subclasses, Rules do not own their own Rainfalls or Traces, TBD to review $$$
*/ 
   NGuiKey ruleTraceKey( u_RealtimeTracesForRulesInKit_byUai[uaiOfRule]->SayGuiKey() );

   std::pair<RtTraceAccessTable_t::iterator, bool> traceTableVerbReply =
      p_TracesInRealtimeKrono_byKey.emplace(
         std::pair< NGuiKey, CTraceRealtime*>(
            ruleTraceKey,
            u_RealtimeTracesForRulesInKit_byUai[uaiOfRule].get()
         )
      );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Declare one local name to use for multiple allocations of new Panes on the heap, first constructing
   a Pane exclusively for the rule :
*/ 
   std::unique_ptr<CPaneRealtime> u_RealtimePane_locallyScopedOnHeap =
      std::make_unique<CPaneRealtime>( p_TracesInRealtimeKrono_byKey[ruleTraceKey],
                                       ViewRef
   );

   NGuiKey rulePaneKey( u_RealtimePane_locallyScopedOnHeap->SayGuiKey() ); // needed near end of method

   std::pair<RtPaneOwnershipTable_t::iterator, bool> paneTableVerbReply =
      u_PanesInRealtimeKrono_byKey.emplace(
         std::pair< NGuiKey, std::unique_ptr<CPaneRealtime> >(
            rulePaneKey,
            u_RealtimePane_locallyScopedOnHeap.release()   // nulls the local ptr
         )
      );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Traverse the lookup table of Traces, assigning each Trace to a "compatible" Pane., or creating a
   new Pane
*/
   bool traceMatchedAndAddedToPane = NaNBOOL;
   for ( const auto& pairRef_trace : p_TracesInRealtimeKrono_byKey ) {

      if ( pairRef_trace.first == ruleTraceKey ) { continue; }  // Rule result keeps its own pane

      traceMatchedAndAddedToPane = false;

      // Iter over created Panes; redeclare on each lap of for-loop in case emplacing invalidates it (?)
      auto pairIter_pane = u_PanesInRealtimeKrono_byKey.begin(); 

      while ( ! traceMatchedAndAddedToPane ) {

         traceMatchedAndAddedToPane =
            pairIter_pane->second->AddTraceIfCompatible( pairRef_trace.second );

         if ( traceMatchedAndAddedToPane ) {
            // go to next available Trace via for-loop, which also puts Pane iterator back to begin()
            break; }
 
         // else, find the Trace a compatible Pane among others already created ...  
         std::advance( pairIter_pane, 1 );

         if ( pairIter_pane != u_PanesInRealtimeKrono_byKey.end() ) {
            continue;   // continue in 'while' thru remaining panes...
         } 
         // ... or, if no more panes, create a new pane based upon currently iterated trace :

         u_RealtimePane_locallyScopedOnHeap = 
            std::make_unique<CPaneRealtime>( pairRef_trace.second,
                                             ViewRef
            );

         NGuiKey keyOfNewPane( u_RealtimePane_locallyScopedOnHeap->SayGuiKey() ); 

         paneTableVerbReply =
            u_PanesInRealtimeKrono_byKey.emplace(
               std::pair< NGuiKey, std::unique_ptr<CPaneRealtime> >(
                  keyOfNewPane,
                  u_RealtimePane_locallyScopedOnHeap.release()   // nulls the local ptr
            )
         );

         traceMatchedAndAddedToPane = true;

      } // close 'while' that matches traces to panes         
   }  // close 'for', all panes needed are created and ready to be arranged on Krono by apiType

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Temporarily group all created panes separately, based upon apiType (i.e., fact vs. analog) 

   for ( const auto& pairRef_pane : u_PanesInRealtimeKrono_byKey ) {

      if ( pairRef_pane.second->SayApiType() == EApiType::Pane_realtime_fact ) {
         p_factPanesCreated.push_back( pairRef_pane.second.get() );
      }
      else if ( pairRef_pane.second->SayApiType() != EApiType::Pane_realtime_rule ) {
         // i.e., if pane is not either a fact-pane or a rule-pane, pane must be analog-pane 
         p_analogPanesCreated.push_back( pairRef_pane.second.get() );
      }
   }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Create Krono (with rule pane at top) and add Panes to it in following order:
      Top -> bottom : Rule result, all binary ("fact") panes, all analog ("data") panes.
*/
   u_RealtimeKrono =
      std::make_unique<CKronoRealtime>(
         *this,
         CtrlrRef,
         ViewRef,
         GenerateKronoCaption_OneRule( uaiOfRule ),
         u_PanesInRealtimeKrono_byKey[rulePaneKey].get() // sets rule-pane as topmost pane on krono 
   );

   // Add Panes to Krono downward from top (rule) pane, fact panes first
   for ( auto p_factPane : p_factPanesCreated ) {
      u_RealtimeKrono->AddPaneBelowExistingPanes( p_factPane );
   }          
   for ( auto p_analogPane : p_analogPanesCreated ) {
      u_RealtimeKrono->AddPaneBelowExistingPanes( p_analogPane );
   }

   uaiOfRuleInRtKrono_zeroIfNoneOrAll = uaiOfRule;

   return EGuiReply::OKAY_allDone;
}


//======================================================================================================/

EGuiReply CRuleKit::LoadRealtimeKronoWithResultsFromAllRulesInKit( bool userInput ) {

// Clear Realtime Krono of any existing content
 
    u_RealtimeKrono.reset( nullptr );  // R-t Krono d-tor calls ClearKitOfRealtimeKronoParts()

   if ( userInput == false ) { return EGuiReply::OKAY_allDone; } 

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Construct one Pane to display all Traces of Rules in kit, initialized with Trace of Rule at top of GUI
 
   std::unique_ptr<CPaneRealtime> u_RealtimePane_locallyScopedOnHeap =
      std::make_unique<CPaneRealtime>(
         u_RealtimeTracesForRulesInKit_byUai[ ruleUais_guiTopToBottom[0] ].get(),
         ViewRef
      );

   NGuiKey keyOfRulesPane( u_RealtimePane_locallyScopedOnHeap->SayGuiKey() );

   std::pair<RtPaneOwnershipTable_t::iterator, bool> paneTableVerbReply =
      u_PanesInRealtimeKrono_byKey.emplace(
         std::pair< NGuiKey, std::unique_ptr<CPaneRealtime> >(
            keyOfRulesPane,
            u_RealtimePane_locallyScopedOnHeap.release()   // nulls the local ptr
         )
      );

   bool traceMatchedAndAddedToPane = NaNBOOL;

   for ( size_t iRead=1; iRead<ruleUais_guiTopToBottom.size(); ++iRead ) { // pane already has UAI @ [0]

      traceMatchedAndAddedToPane =
         u_PanesInRealtimeKrono_byKey[keyOfRulesPane]->
            AddTraceIfCompatible(
               u_RealtimeTracesForRulesInKit_byUai[ ruleUais_guiTopToBottom[iRead] ].get()
            );
   }       

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Create Krono using the Pane to initialize it:

   u_RealtimeKrono =
      std::make_unique<CKronoRealtime>(
         *this,
         CtrlrRef,
         ViewRef,
         GenerateKronoCaption_AllRulesInKit(),
         u_PanesInRealtimeKrono_byKey[keyOfRulesPane].get() // sets rule-pane as topmost pane on krono 
   );

   // since no further panes added to Krono, top pane becomes only pane on it 

   isRealtimeKronoShowingAllRules = true;
   uaiOfRuleInRtKrono_zeroIfNoneOrAll = 0u;

   return EGuiReply::OKAY_allDone;
}

//======================================================================================================/
 
void CRuleKit::CalcOwnTriggerGroup( void ) { ownTriggerGroup = baseTriggerGroup; return; }

//======================================================================================================/

void CRuleKit::Cycle( time_t timestampNow ) {

   if ( ! kitFinalized ) { throw std::logic_error( "Attempted to run unfinalized Rule Kit" ); }

   // triggering the kit's individual CRule objects to cycle is done by the kit's rainfall object
   std::pair<Nzint_t,bool> cycleResults =
      u_RainRuleKit->CycleRulesInKitAndSayResults( timestampNow,                                                                        
                                                   cycleBeginsNewClockHour,
                                                   cycleBeginsNewCalendarDay,
                                                   p_Rules_byUai );

   // Report to Subject the part of cycle result feeding hierarchical logic
   SubjRef.SubmitTrueIfGotFailOnPinnedRule( kitSgiFromSubject, cycleResults.second );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Manage CCase object creation based upon cycle results

  Nzint_t uaiTrappedRule = cycleResults.first;

   if ( uaiTrappedRule == 0u ) {    // Value of '0' signals that no rule "trapped" on present Cycle

      return;     // End of a CRuleKit cycle unless a rule is "trapped"
   } 

   SubjRef.TagAndPostAlertToDomain( timestampNow,
                                    EDataLabel::Case_alertToGui,                                   
                                    p_Rules_byUai[uaiTrappedRule]->SayFailureMsg() );

   if ( p_Rules_byUai[uaiTrappedRule]->IsInAutoMode() ) {

      CaseKitRef.CreateAndOwnCase(  *this,
                                    SubjRef,
                                    timestampNow,
                                    triggerCount,
                                    secsPerCycle,
                                    *p_Rules_byUai[uaiTrappedRule],
                                    *u_RealtimeTracesForRulesInKit_byUai[uaiTrappedRule]
      );
      areAllRulesNotInCaseModePutToIdle = false; // the knob idling all rules needs to know this
   }
   return;                                             
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CRuleKit Public Methods

std::vector<std::string> CRuleKit::SayRuleLabels_GuiTopToBottom( void ) const {

   std::vector<std::string> reply(0);

   for ( auto uaiByValue : ruleUais_guiTopToBottom ) {

      reply.push_back(  p_Rules_byUai.at(uaiByValue)->SayRuleUaiText() +
                        ", re: " +
                        IGuiShadow::LookUpText( p_Rules_byUai.at(uaiByValue)->SayFocusMsg() )
      );
   }
   return reply;
}


std::vector<std::string> CRuleKit::SayRuleTexts_If_GuiTopToBottom( void ) const {

   std::vector<std::string> reply( ruleUais_guiTopToBottom.size() );

   auto readCiter = ruleUais_guiTopToBottom.begin();
   for ( auto& writeIter : reply ) {
      writeIter = IGuiShadow::LookUpText( p_Rules_byUai.at(*readCiter)->SayTest_If() );
      std::advance(readCiter,1);
    }
   return reply;
}

std::vector<std::string> CRuleKit::SayRuleTexts_Then_GuiTopToBottom( void ) const {

   std::vector<std::string> reply( ruleUais_guiTopToBottom.size() );

   auto readCiter = ruleUais_guiTopToBottom.begin();
   for ( auto& writeIter : reply ) {
      writeIter = IGuiShadow::LookUpText( p_Rules_byUai.at(*readCiter)->SayTest_Then() );
      std::advance(readCiter,1);
   }
   return reply;
}


std::vector<EGuiState> CRuleKit::SayNewestRuleStates_GuiTopToBottom( void ) const {

   return u_RainRuleKit->SayNewestRuleStates_GuiTopToBottom( ruleUais_guiTopToBottom );
}


std::vector<NGuiKey> CRuleKit::SayRuleKitKnobKeys( void ) const {

   return knobKeysOfRuleKitItself;  // Compiler chooses NRVO over PBV (?)
}



std::vector<NGuiKey> CRuleKit::SayKnobKeyOfEachRule_GuiTopToBottom ( void ) const {

   return knobKeyOfEachRuleInKit_guiTopToBottom;  // Compiler chooses NRVO over PBV (?)
}


std::vector<NGuiKey> CRuleKit::SayHistogramKeyOfEachRule_GuiTopToBottom ( void ) const {

   return histogramKeyOfEachRuleInKit_guiTopToBottom;  // Compiler chooses NRVO over PBV (?)
}


std::string CRuleKit::SayKitCaption( void ) const {

// No const string initially available to object, as number of rule kits on a subject is runtime info 

   return ( SubjRef.SayDomainAndOwnNameAsText() +
            " Rule kit " + std::to_string( kitSgiFromSubject ) +
            " of " + SubjRef.SayNumRuleKitsAsText() +
            " with " + std::to_string( p_Rules_byUai.size() ) + " rules" +
            " at " + std::to_string( secsPerCycle ) + " s/cycle" );
}


std::string CRuleKit::SayKitSgiOfNumberAsText( void ) const {

// No const string initially available to object, as number of rule kits on a subject is runtime info 

   return ( "Rule kit " + std::to_string( kitSgiFromSubject ) +
            " of " + SubjRef.SayNumRuleKitsAsText() );
}



CCaseKit& CRuleKit::SayCaseKitRef( void ) const { return CaseKitRef; }


CController& CRuleKit::SayCtrlrRef( void ) const { return CtrlrRef; }


CSeqTimeAxis& CRuleKit::SayTimeAxisRef( void ) const { return *SeqRef.u_TimeAxis; }


Nzint_t CRuleKit::CheckRuleUaiFreeThenKeep( Nzint_t uaiGivenToCRuleCtor ) {

   // Called by CRule c-tor initialization list; checks that ruleId will be unique within kit intended
   // $$$ TBD to wack this for a better rule-ruleKit construction order $$$

   if ( uaiGivenToCRuleCtor > 0 ) {  // UAI not < 1
     if ( p_Rules_byUai.find(uaiGivenToCRuleCtor) == p_Rules_byUai.end() ) { // UAI not already assigned

         return uaiGivenToCRuleCtor;
      }
   }
   throw std::logic_error( "Attempted bad or duplicate rule UAI in rule kit" );
   return 0;
}


NGuiKey CRuleKit::SayRealtimeKronoKey( void ) const {

 return (  ( u_RealtimeKrono == nullptr ) ?
               NGuiKey(0) :
               u_RealtimeKrono->SayGuiKey() );
}


NGuiKey CRuleKit::SayHistogramKey( void ) const { return u_RainRuleKit->SayKeyToOverviewHistogram(); }


//======================================================================================================/

void CRuleKit::AddRuleToKit( CRule* const p_Rule ) {

   if ( kitFinalized ) { throw std::logic_error( "Attempted adding rule to finalized kit" ); }

   if ( p_Rules_byUai.size() < FIXED_RAIN_NUMRULESINKIT_MAX ) {

      p_Rules_byUai.emplace( std::pair<Nzint_t, CRule*>( p_Rule->SayRuleUai(), p_Rule ) );
      ruleUais_guiTopToBottom.push_back( p_Rule->SayRuleUai() );
      return;
   }
   throw std::logic_error( "Too many rules assigned to one rule kit" );       // intentionally uncaught
   return;   
}

//======================================================================================================/

void CRuleKit::ClearKitOfRealtimeKronoParts( void ) {

   // Clear Rule Kit's standing tables of Traces and Panes:
   // All r-t Traces are immortal and owned elsewhere, so clear local ptrs but do not deallocate 
   p_TracesInRealtimeKrono_byKey.clear();

   // r-t Panes are mortal and owned locally, so let smart ptrs self-deallocate upon clearing vector
   u_PanesInRealtimeKrono_byKey.clear();

   // in case Krono had been showing results from "all rules in kit" :
   isRealtimeKronoShowingAllRules = false;

   uaiOfRuleInRtKrono_zeroIfNoneOrAll = 0u;
   return;
 }


//======================================================================================================/

EGuiReply CRuleKit::IdleAllRulesNotInCaseMode( bool userWantsIdleMode ) {

   if ( userWantsIdleMode ) {
      // rules not currently in "case mode" will go to (or remain in) "idle mode"
      for ( auto& iter : p_Rules_byUai ) { iter.second->SetIdleModeTo( true ); }
      areAllRulesNotInCaseModePutToIdle = true;
      return EGuiReply::OKAY_allDone;
   }

   // else, only rules currently in "idle mode" will go to "auto mode"
   for ( auto& iter : p_Rules_byUai ) { iter.second->SetIdleModeTo( false ); }
   areAllRulesNotInCaseModePutToIdle = false;
   return EGuiReply::OKAY_allDone;   
}


//======================================================================================================/

void CRuleKit::FinalizeRuleKitAndBuildKbase( void ) {

// ensure Rule UAIs are displayed lesser UAI (top of GUI) to greater (bottom of GUI)

   std::sort(  ruleUais_guiTopToBottom.begin(),
               ruleUais_guiTopToBottom.end(),
               std::less<Nzint_t>()            
   );

// Finalize Rainfall to span Rules in finalized kit.  Rain will also instantiate a histogram for each rule

   u_RainRuleKit->FinalizeRainfallOnTheRulesAddedToKit(  *this,
                                                         SubjRef.SayViewRef(),
                                                         p_Rules_byUai,
                                                         ruleUais_guiTopToBottom ); // provides the sort

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Build ownership table of Traces for Rules in finalized kit

   for ( auto pairByValue_rule : p_Rules_byUai ) {

      std::pair<RtRuleTraceOwnershipTable_t::iterator, bool> ruleTraceTableVerbReply =
         u_RealtimeTracesForRulesInKit_byUai.emplace(
            std::pair<Nzint_t, std::unique_ptr<CTraceRealtime> >(
               pairByValue_rule.first,    // first in pair is the Rule's UAI
               std::make_unique<CTraceRealtime>(   SubjRef.SayViewRef(),
                                                   *u_RainRuleKit,
                                                   *pairByValue_rule.second,
                                                   pairByValue_rule.second->SayCrefToKnobKeys()
               )
         )
      );
   }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// CRule class has one own Knob, a separate instance held by each rule in finalized kit

   histogramKeyOfEachRuleInKit_guiTopToBottom.clear();
   knobKeyOfEachRuleInKit_guiTopToBottom.clear();

   for ( auto uaiByValue : ruleUais_guiTopToBottom ) {

      histogramKeyOfEachRuleInKit_guiTopToBottom.push_back(
         u_RainRuleKit->SayKeyToHistogramOfRule( uaiByValue )
      );
      knobKeyOfEachRuleInKit_guiTopToBottom.push_back(
         p_Rules_byUai[uaiByValue]->SayKeyToOwnKnob()
      );
   }

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// CRuleKit class has three knobs, one of subclass CKnobSelectNzint which requires:

   p_KnobSelectingRuleUai->DefineValuesSelectable( ruleUais_guiTopToBottom );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Build out 3-D Knowledge Base to span Rules (and associated Hypotheses and Evidences) in finalized kit

   for ( auto& p_ruleIter : p_Rules_byUai ) { 

      p_ruleIter.second->BuildRuleIntoKbase( *u_Kbase );
   }
   kitFinalized = true;
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ