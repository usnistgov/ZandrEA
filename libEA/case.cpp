// This file is in EA library of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements CCase and CCaseKit classes for AFDD diagnostic "cases"
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "rule.hpp"
#include "knowBase.hpp"
#include "mvc_ctrlr.hpp"
#include "mvc_view.hpp"
#include "case.hpp"
#include "subject.hpp"
#include "guiShadow.hpp"
#include "controlParts.hpp"   // rule kit and r-t krono knobs
#include "viewParts.hpp"      // needed to support deletion of smart ptrs for owned viewParts

#include <fstream>
#include <sys/stat.h>                        // stat in ArchivePreexists()

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>                           // stringstream
#include <algorithm>
#include <memory>
#include <numeric>
#include <iterator>

#include "portability.hpp"

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Function in class-free static memory

inline bool ArchivePreexists( const std::string& archFileName ) {

   struct stat archFileData;
   return ( stat( archFileName.c_str(), &archFileData ) == 0 );
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Const fields (spread dynamically over case instances, so static to class, not macros by compiler)

const GuiMsgInsert_t CCase::addToReport_trailer_NeedUserToFactuallyVerifySolution_lineOne =
   "EA asks to be taught which solution is correct.";

const GuiMsgInsert_t CCase::addToReport_trailer_NeedUserToFactuallyVerifySolution_lineTwo =
   "You should not delete this case until a verified solution is learned by EA.";

const GuiMsgInsert_t CCase::addToPrompt_trailer_HowDoYouWantToProceed = "How do you want to proceed?";

const GuiMsgPacket_t CCase::prompt_ShouldThisCaseBeDeleted = { {"Delete this case from list?"} };

const GuiMsgPacket_t CCase::prompt_ThatAnswerInvalidChooseOnlyAsAllowed;

const GuiMsgPacket_t CCase::prompt_TopMenu_CaseInProcess_HDYWTP = {
   { "EA ready for answers to questions it has to improve its solution." },
   addToPrompt_trailer_HowDoYouWantToProceed
   // anticipates promptMode to 'multi'
};

const GuiMsgPacket_t CCase::report_CaseBustedKnowledgeBase = {
   {"Valid alarm but cause not known to EA Knowledge Base." },
   addToPrompt_trailer_HowDoYouWantToProceed
   // anticipates promptMode to 'multi'
};

const GuiMsgPacket_t CCase::prompt_CaseSpentOut_HDYWTP = {
   { "All case evidence has been evaluated." },
   { "EA cannot provide better fault probabilities for this case." },
   addToPrompt_trailer_HowDoYouWantToProceed
   // anticipates promptMode to 'multi'
};

const GuiMsgPacket_t CCase::prompt_MappAt100Percent_HDYWTP = {
   { "A probability of 100% has been estimated." },
   { "EA cannot provide better fault probabilities for this case." },
   addToPrompt_trailer_HowDoYouWantToProceed
   // anticipates promptMode to 'multi'
};

const GuiMsgPacket_t CCase::prompt_IsMappHypoCorrect = {
   { "Is the solution offered by EA correct?" }
   // anticipates promptMode to 'no/yes/dnk'
};

const GuiMsgPacket_t CCase::prompt_IsAnyCaseHypoCorrect = {
   { "Is the correct solution any of those listed?" }
   // anticipates promptMode to 'no/yes/dnk'
};

const GuiMsgPacket_t CCase::prompt_IsFaultFixed = {
   { "Has the fault that caused this case been fixed?" }
   // anticipates promptMode to 'no/yes/dnk'
};

const GuiMsgPacket_t CCase::prompt_UserVerifyCorrectHypoByChoosing = {
   { "Which solution listed is correct?" }
   // anticipates promptMode to 'multi'
};

const GuiOptionSet_t CCase::choice_NoYes = { "No", "Yes" };

const GuiOptionSet_t CCase::choice_NoYesDnk = { "No", "Yes", "Do not know" };

const GuiOptionSet_t CCase::choice_TopMenu_CaseInProcess = {
   "Do nothing now.  Leave case as is until later.",
   "See a new question from this case.",
   "Verify the EA solution or teach EA another solution is correct.",
   "Delete this case."
};

const GuiOptionSet_t CCase::choice_ForMappAt100Percent = {
   "Do nothing now.  Leave case as is until later.",
   "Verify the EA solution or teach EA another solution is correct.",
   "Delete this case."
};

const GuiOptionSet_t CCase::choice_ForCaseVerifiedAndLearned = {
   "Do nothing now.  Leave case as is until later.",
   "Teach EA that the fault causing this case has been fixed.",
   "Delete this case"
};

const GuiOptionSet_t CCase::choice_ForCaseSpentOut = {
   "Do nothing now.  Leave case as is until later.",
   "Verify the EA solution or teach EA another solution is correct.",
   "Delete this case."
};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// static functions of class

std::string CCase::InitReportPreamble( ERealName subjName,
                                       time_t trapTime,
                                       int trapTrigger,
                                       int secsPerRuleCycle,
                                       const CRule& ruleRef ) {

   char timestampChars[26] = {'\0'};
   errno_t err = 0;
   // ctime_s writes to exactly 26 char array, format: Wed Jan 02 02:03:55 1980\n\0
   // (note: \n and \0 = one char each)
   err = ctime_s( timestampChars, 26, &trapTime ); 

   return ( "At " + LookUpText( subjName ) +
            " by sample: " + std::to_string( trapTrigger ) +
            " time: " + std::string( timestampChars ) +
            " : FAILS " +
            std::to_string( secsPerRuleCycle ) +
            " secs/test on: " +
            ruleRef.SayRuleUaiText() +
            " : " +
            LookUpText( ruleRef.SayFailureMsg() ) 
   ); 
}


GuiOptionSet_t CCase::GenerateMultipleChoiceOfLength( size_t numChoices) {

   GuiOptionSet_t reply( numChoices, "");
   char letter = 'A';
   // Careful, RBF needed here must be PBR, not PBV (element ref'ed, not copied) !!
   for ( auto& choice : reply ) { choice = std::to_string( static_cast<char>(letter++) ); }
   return reply;     
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Constructor/destructor

CCase::CCase(  CRuleKit& ruleKitRef,
               ASubject& arg0,
               time_t arg1,
               int arg6,
               int arg2,
               CRule& arg3,
               const CTraceRealtime& arg4,
               bool arg5 )
               :  IGuiShadow( EApiType::Case ),
                  u_SnapshotTracesOfObjectsAntecedentToCaseRule_byKey(),
                  u_PanesInSnapshotKrono_byKey(),
                  u_SnapshotTraceOfCaseRule( std::make_unique<CTraceSnapshot>(
                                                arg4,
                                                arg3.SaySnapshotSetSgi()
                                             )
                  ), 
                  u_SnapshotKrono (nullptr),
                  RuleRef (arg3),
                  ViewRef (arg0.SayViewRef() ),
                  p_Kbase ( ruleKitRef.u_Kbase.get() ),
                  p_ActOnAnswer(nullptr),  //  [](CCase*)->char {} 
                  hypoTally(),
                  evidTally(),
                  evidsUnevaluated(),
                  userReport(),
                  userPrompt (prompt_TopMenu_CaseInProcess_HDYWTP),
                  userOptions (choice_TopMenu_CaseInProcess),
                  answerMinMax( {0,0} ),
                  timeTrapped (arg1),
                  sumPriorCountsTrueOfAllCaseHypos (0),
                  sumJointPostCountsAccumByHyposAlive (0),
                  caseSgi ( arg0.GenerateAndSaySgiForNewCase() ),
                  caseName (  arg0.SayDomainAndOwnNameAsText() +
                              ":Case #" + SayIdentifierAsThreeDigitText( caseSgi ) +
                              "-Failing " + arg3.SayRuleUaiText()
                  ),
                  reportPreamble ( InitReportPreamble(   arg0.SayName(),
                                                         arg1,
                                                         arg6,
                                                         arg2,
                                                         arg3 )
                  ),
                  snapshotSetSgi ( arg3.SaySnapshotSetSgi() ),
                  caseRuleUai ( arg3.SayRuleUai() ),
                  uaiMappHypo (0),
                  uaiTrueHypo (0),                  
                  uaiEvidUp (0),
                  answer (NaNSIZE),
                  caseRank (NaNSIZE),
                  numHyposAlive (0),
                  numHyposPrior (0),
                  noEvidenceEvaluated (true),
                  caseBustsKbase (false),
                  mappAt100pct (false),
                  caseSpentKbase (false),
                  ruleHasDiagnostics (arg5),
                  caseVerifiedAndLearned (false),
                  waitingOnUserToAnswer (true) {

   LoadSnapshotKronoWithLogicChainOfCase( ruleKitRef );

   p_Kbase->InitializeCaseTallies(  RuleRef.SayRuleUai(),
                                    hypoTally,
                                    evidTally,
                                    sumPriorCountsTrueOfAllCaseHypos );

   numHyposPrior = hypoTally.size();
   numHyposAlive = numHyposPrior;

   for ( auto& iter : evidTally ) {
      evidsUnevaluated.push( iter.first );
   }
   UpdateBayesProbabilities();
   WriteBayesProbsToReport();
   p_ActOnAnswer = &CCase::ActOnAnswerTo_TopMenu_CaseInProcess;
   SetAnswerMinMax( userOptions.size() );
   RuleRef.SetCaseModeTo( true );
}

//======================================================================================================/

CCase::~CCase( void ) {

   RuleRef.SetCaseModeTo( false );
   ViewRef.RemoveCaseFromCaseKitLookup( ownGuiKey );
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// private methods


void CCase::LoadSnapshotKronoWithLogicChainOfCase( CRuleKit& ruleKitRef ) {


// Declare locals used later when arranging Panes on Krono

   std::vector<CPaneSnapshot*> p_analogPanesCreated(0);
   std::vector<CPaneSnapshot*> p_factPanesCreated(0);

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Load snapshot trace ownership table using realtime trace access from Rule. 

   for ( const auto pairValues_trace :       // Here, want PBV not PBR, since both in pair are POD types
         RuleRef.SayRealtimeAccessTable() ) {

      std::unique_ptr<CTraceSnapshot> u_SnapshotTrace_loopScopedToHeap =
         std::make_unique<CTraceSnapshot>(   *(pairValues_trace.second), // deref of a raw ptr PBV
                                             snapshotSetSgi
         );

      NGuiKey keyOfNewTrace( u_SnapshotTrace_loopScopedToHeap->SayGuiKey() ); 

      std::pair<SsTraceOwnershipTable_t::iterator, bool> traceTableVerbReply =
         u_SnapshotTracesOfObjectsAntecedentToCaseRule_byKey.emplace(
            std::pair< NGuiKey, std::unique_ptr<CTraceSnapshot> >(
               keyOfNewTrace,
               u_SnapshotTrace_loopScopedToHeap.release()    // xfers ownership from and nulls local ptr
            )
         );
   }  // destroys nulled local (loop) ptr
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Make local allocation to create Panes, and put Snapshot Trace of Case Rule into a Pane of its own
 
   std::unique_ptr<CPaneSnapshot> u_SnapshotPane_locallyScopedOnHeap =
         std::make_unique<CPaneSnapshot>( u_SnapshotTraceOfCaseRule.get(),
                                          ViewRef
         );

   NGuiKey rulePaneKey( u_SnapshotPane_locallyScopedOnHeap->SayGuiKey() ); // Needed in Krono c-tor

   std::pair<SsPaneOwnershipTable_t::iterator, bool> paneTableVerbReply =
      u_PanesInSnapshotKrono_byKey.emplace(
         std::pair< NGuiKey, std::unique_ptr<CPaneSnapshot> >(
            rulePaneKey,
            u_SnapshotPane_locallyScopedOnHeap.release()
         )
      );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/* Traverse the lookup table of Traces, assigning each Trace to a "compatible" Pane., or creating a
   new Pane
*/
   bool traceMatchedAndAddedToPane = NaNBOOL;
   for ( const auto& pairRef_trace :            // Here, want PBR not PBV, so can use smart-ptr for calls
         u_SnapshotTracesOfObjectsAntecedentToCaseRule_byKey ) {

      traceMatchedAndAddedToPane = false;

      // Iter over created Panes; redeclare on each lap of for-loop in case emplacing invalidates it (?)
      auto pairIter_pane = u_PanesInSnapshotKrono_byKey.begin(); 

      while ( ! traceMatchedAndAddedToPane ) {

         traceMatchedAndAddedToPane =
            pairIter_pane->second->AddTraceIfCompatible( pairRef_trace.second.get() );

         if ( traceMatchedAndAddedToPane ) {
            // go to next available Trace via for-loop, which also puts Pane iterator back to begin()
            break; }
 
         // else, find the Trace a compatible Pane among others already created ...  
         std::advance( pairIter_pane,1 );

         if ( pairIter_pane != u_PanesInSnapshotKrono_byKey.end() ) {
            continue;   // continue in 'while' thru remaining panes...
         } 
         // ... or, if no more panes, create a new pane based upon currently iterated trace :

         u_SnapshotPane_locallyScopedOnHeap = 
            std::make_unique<CPaneSnapshot>( pairRef_trace.second.get(),
                                             ViewRef
         );

         NGuiKey keyOfNewPane( u_SnapshotPane_locallyScopedOnHeap->SayGuiKey() ); 

         paneTableVerbReply =
            u_PanesInSnapshotKrono_byKey.emplace(
               std::pair< NGuiKey, std::unique_ptr<CPaneSnapshot> >(
                  keyOfNewPane,
                  u_SnapshotPane_locallyScopedOnHeap.release()   // nulls the local ptr
            )
         );
         traceMatchedAndAddedToPane = true;

      } // close 'while' that matches traces to panes
         
   }  // close 'for', all panes needed are created and ready to be arranged on Krono by apiType

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Temporarily group all created panes separately, based upon apiType (i.e., fact vs. analog) 

   for ( const auto& pairRef_pane : u_PanesInSnapshotKrono_byKey ) {

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
   u_SnapshotKrono = std::make_unique<CKronoSnapshot>(
                        ruleKitRef,
                        ruleKitRef.SayCtrlrRef(),
                        ViewRef,
                        reportPreamble,
                        snapshotSetSgi,
                        u_PanesInSnapshotKrono_byKey[rulePaneKey].get() // sets as top pane on krono 
   );

   // Add Panes to Krono downward from top (rule) pane, fact panes first
   for ( auto p_factPane : p_factPanesCreated ) {     // auto by value (no "&") here yields a pointer
      u_SnapshotKrono->AddPaneBelowExistingPanes( p_factPane );
   }          
   for ( auto p_analogPane : p_analogPanesCreated ) {
      u_SnapshotKrono->AddPaneBelowExistingPanes( p_analogPane );
   }

   return;
}

//======================================================================================================/

void  CCase::SetAnswerMinMax( size_t numChoices) { 

   answerMinMax[0] = 0;
   answerMinMax[1] = ( (numChoices > 0) ? (numChoices-1) : 0 );
   return;  
}


void CCase::ApplyBayesGivenEvidUpIs( size_t evidValue ) {

   Tally_t weightOfLatestEvidAtPostValue = 0;
   Tally_t weightOfLatestEvidAtAllValues = 0;
   Tally_t postCountsJointToIterHypoBeingTrue = 0;
   Tally_t tallyZero = 0;
   bool evidValueGivenIsNovel = false;

   sumJointPostCountsAccumByHyposAlive = 0;              // re-zero register in class

   evidTally.at(uaiEvidUp).evidStatus = evidValue;

   if ( noEvidenceEvaluated ) {
      noEvidenceEvaluated = false;
   }
   
   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   // "Weigh" the evidence just now provided (See Method Note [1] )

   for (auto& hypoIter : hypoTally) {

      if (hypoIter.second.hypoStatus == 0) { continue; } // scope of evid "weight" is hypos alive only

      p_Kbase->PanCursorToNodeAt(caseRuleUai, hypoIter.first, uaiEvidUp);  // hypoIter.first rtns hypoId
      p_Kbase->ReadCursorToImage();

      weightOfLatestEvidAtPostValue += p_Kbase->PriorCountsHypoTrueWhenEvid( evidValue );
      weightOfLatestEvidAtAllValues += p_Kbase->PriorCountsHypoTrueWhenEvid( 0 );
      weightOfLatestEvidAtAllValues += p_Kbase->PriorCountsHypoTrueWhenEvid( 1 );
      weightOfLatestEvidAtAllValues += p_Kbase->PriorCountsHypoTrueWhenEvid( 2 );
   }
   
   evidValueGivenIsNovel = ( weightOfLatestEvidAtPostValue == 0 );


   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   // Re-Walk all alive hypos, read KB's counts of given evid value being joint to each hypo being true
   
   for ( auto& hypoIter : hypoTally ) {

      if ( hypoIter.second.hypoStatus == 0 ) { continue; }

      if ( evidValueGivenIsNovel ) {                                    // See Method Note [2]
         hypoIter.second.accumPostCountsJointToThisHypoTrue += 1;
         sumJointPostCountsAccumByHyposAlive += 1;
         continue;
      }

      p_Kbase->PanCursorToNodeAt(caseRuleUai, hypoIter.first, uaiEvidUp); // hypoIter.first rtns hypoId
      p_Kbase->ReadCursorToImage();

      postCountsJointToIterHypoBeingTrue = p_Kbase->PriorCountsHypoTrueWhenEvid( evidValue );

     //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
     // Test iterated hypo for the condition that justifies killing it (See Method Note [3])

      if (  ( postCountsJointToIterHypoBeingTrue == 0 ) &&
            ( p_Kbase->HasHypoEverBeenFalseWhenEvid( evidValue ) == true ) ) {

         hypoIter.second.hypoStatus = 0;   // set iterated hypo to being likely false ("dead")
         hypoIter.second.accumPostCountsJointToThisHypoTrue = 0;
         --numHyposAlive;
         continue;
      }

      //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      // Test iterated hypo for condition immediately justifying it at 100% MAPP (See Method Note [4])

      else if (   ( postCountsJointToIterHypoBeingTrue > 0 ) &&
                  ( postCountsJointToIterHypoBeingTrue == weightOfLatestEvidAtPostValue)
                  // i.e., every time evidUp had the value given, the iterated hypo had been true
               ) {

         uaiMappHypo = hypoIter.first;
         mappAt100pct = true;
         hypoIter.second.hypoStatus = 1;
         hypoIter.second.accumPostCountsJointToThisHypoTrue += postCountsJointToIterHypoBeingTrue;

         for ( auto& hypoIterB : hypoTally ) {             // kill all other hypos not already dead

            if (hypoIterB.second.hypoStatus == 2) {

               hypoIterB.second.hypoStatus = 0;
               hypoIterB.second.accumPostCountsJointToThisHypoTrue = 0;
               continue;
            }
         }
         sumJointPostCountsAccumByHyposAlive = hypoIter.second.accumPostCountsJointToThisHypoTrue;
         break;
      }
      hypoIter.second.accumPostCountsJointToThisHypoTrue += postCountsJointToIterHypoBeingTrue;
      sumJointPostCountsAccumByHyposAlive += hypoIter.second.accumPostCountsJointToThisHypoTrue;
   }
 
/* METHOD NOTES vvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

   [1]   Real-time evaluation ("post" value) of the 'evid up' includes:
         (A)   all prior counts  in KB at that value, regardless of hypo (this info is held by evidTally)
         (B)   summing above into an accumulation "weighing" evidence by all prior counts at the post
               value, and...
         (C)   ... a second accumulation weighing all prior counts summed over each value possible.

         (B) divided by (C) = prior prob of getting the accum posts, regardless (free) of any hypothesis
         (a.k.a., the "probability of the data") as calculated later in UpdateBayesProbabilities().

   [2]   If evid has no prior counts at the given ('post') value that are joint to any case hypo being
         true, the given value is novel... the KB has never before 'seen' that evidence at that value as
         joint to any hypo being true.  But, the value has just occurred, so its probability cannot be
         considered zero.  Because the value has no priors joint to ANY case hypo being true, but ONE
         (and ONLY one) of the case hypos is expected as true, one count is distributed to each of the
         case hypos still alive.

         If the evidence item has no prior counts, at any of its possible values, joint to any case hypo
         being true, the evidence item itself is novel. Novel evidence items are not possible from
         current EA capabilities, but could arise from a future, autonomous,'evidence-mining' capability.

   [3]   User's answer to evidence query "kills" the iterated hypo upon entering this block.  A dead hypo
         is one having accumulated no counts joint to it being true, but at least one count joint to it
         being false.  Reading zero for a count of relevant evidence joint to the hypo being true, in
         itself, could merely mean the evid item has been left unevaluated by prior cases involving that
         hypo.  Finding a count joint to the hypo being false, however, would eliminate that possiblity.

         A hypo is not killed simply for not having counts true joint to the evid value User gave.

   ^^^^ END METHOD NOTES */

   return;
}


void CCase::UpdateBayesProbabilities( void ) {

   if ( caseSpentKbase || caseBustsKbase ) { return; }

   Tally_t likelihoodOfLatestEvidValueGivenIterHypoTrue_pctX10 = 0;
   Tally_t probOfLatestEvidAtPostValue_pctX10 = 0;
   short mapp_pctX10 = 0;
   size_t offsetNow = 0;
   std::vector<size_t> offsetsToSameMax(0);

   for ( auto& hypoIter : hypoTally ) {

      //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      // a priori

      if ( noEvidenceEvaluated ) { 

         hypoIter.second.priorProb_pctX10 =
            static_cast<short>(  ( hypoIter.second.priorCountsThisHypoTrue * 1000 ) /
                                 sumPriorCountsTrueOfAllCaseHypos );            // Method Note [2]

         if ( ! mappAt100pct ) {

            if ( hypoIter.second.priorProb_pctX10 >= mapp_pctX10 ) {
               if ( hypoIter.second.priorProb_pctX10 > mapp_pctX10 ) {
                  mapp_pctX10 = hypoIter.second.priorProb_pctX10;
                  uaiMappHypo = hypoIter.first;
               }
               else {
                  uaiMappHypo =
                     (hypoIter.second.priorCountsThisHypoTrue >
                        hypoTally.at(uaiMappHypo).priorCountsThisHypoTrue ?
                        hypoIter.first : uaiMappHypo);
               }
            }
            mappAt100pct = (mapp_pctX10 > 989 ? true : false);
         }
      }

      //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      // a posteriori

      else {  // Method Note [3]

         hypoIter.second.postProb_pctX10 =
            static_cast<short>( (hypoIter.second.accumPostCountsJointToThisHypoTrue * 1000) /
                                sumJointPostCountsAccumByHyposAlive );

         if ( ! mappAt100pct ) {

            if ( hypoIter.second.postProb_pctX10 >= mapp_pctX10 ) {
               if ( hypoIter.second.postProb_pctX10 > mapp_pctX10 ) {
                  mapp_pctX10 = hypoIter.second.postProb_pctX10;
                  uaiMappHypo = hypoIter.first;
               }
               else {
                  uaiMappHypo =
                     ( hypoIter.second.accumPostCountsJointToThisHypoTrue >
                        hypoTally.at(uaiMappHypo).accumPostCountsJointToThisHypoTrue ?
                           hypoIter.first : uaiMappHypo );
               }
            }
            mappAt100pct = ( mapp_pctX10 > 989 ? true : false );
         }
      }
   }

/* METHOD NOTES vvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   References:
      (1)   Puppe, F. "Systematic Introduction to Expert Systems, Knowledge Representation and Problem
            Solving Methods" (1993)

[2]   C-99 math truncates toward zero whenever shedding remainders.  Holding dividend/divisor as SLL
      and cast quotient to short so to help probs sum to 100% after the cast to float in
      WriteBayesProbsToReport()

[3]   Big Pix: Here and only here does the case move from reflecting generalities about "many instances
      past" to estimating specifics about "the (one) instance here and now".

[4]   Per Ref(1) pg. 58:  Likelihood(S|D) = |S ^^ D| / |D|  ( ^^ = intersection ( "U" upside down))
      so does Likelihood(S1 & S2 & S3 &...|D) = |(S1 & S2 & S3 &...) ^^ D| / |D|  for multi S's (?)

      In EA nomenclature: Likelihood(E|H) = |E ^^ H| / |H|
      so does Likelihood(E1 & E2 & E3 &...|H) = |(E1 & E2 & E3 &...) ^^ H| / |H|  for multi E's (?)

[5]   For now, if have >1 hypo at max prob, make MAPP the first one encountered.
      TBD to improve to hypo having highest number of counts, e.g. :

         auto& iter = hypoTally.begin();
         // take first offset to a hypo having prob = maxProb :
         std::advance(iter, offsetsToSameMax.at(0));
         idMappHypo = iter->first;

^^^^ END METHOD NOTES */

   return;
}


void CCase::WriteBayesProbsToReport( void ) {

   // Hypos are not listed in msgReport by any "rank", but by order found when traversing hypoTally

   userReport.clear();
   userReport = { { reportPreamble },
                  { "Faults possible in this case, with current probability of each:" }
   };

   std::stringstream probPercent_stream;
   char idUserSees = 'A';     // Want User to see a letter as hypo "id", but code uses the Nzint_t "id"
   char idUserSeesForMapp = 'A';
   std::string asciiInsert("0");
   for ( auto iter = hypoTally.begin(); iter != hypoTally.end(); ++iter ) {

      probPercent_stream.precision(3);

      if ( noEvidenceEvaluated ) {
         probPercent_stream << ( static_cast<float>(iter->second.priorProb_pctX10) / 10.0f );
      }
      else { probPercent_stream << ( static_cast<float>(iter->second.postProb_pctX10) / 10.0f); }

      asciiInsert = idUserSees;
      userReport.push_back(   "(" + asciiInsert + ") " +
                              iter->second.hypothesis +
                               " " + probPercent_stream.str() + " %" );

      probPercent_stream.str( std::string() );    // wipe clean the std::stringstream object

      if ( iter->first == uaiMappHypo ) { idUserSeesForMapp = idUserSees; }
      ++idUserSees;
   }
   userReport.push_back("");
   asciiInsert = idUserSeesForMapp;
   userReport.push_back("EA's current solution is (" + asciiInsert + ")" );
   return;
}


AoaReply_t CCase::ActOnAnswerTo_ShouldThisCaseBeDeleted( void ) { 

   /*
   No or Yes answer to deleting case
   Presumes PromptMode = 'B'
   */

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

      case 0:              // no, don't delete case
         reply = ACTION_CASE_IDLEDBYUSER_REGENGUI;
         break;

      case 1:              // yes, delete case
         reply = ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI;
         break;
   }
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_EvaluatePromptedEvidenceQuery( void ) {

   if ( caseSpentKbase ) { return ACTION_CASE_ALLEVIDSSPENT_NONE; } // simply to shorten call

   AoaReply_t reply = ACTION_NONE;
    
   switch ( answer ) {
  
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      case 0:  // User says evid is false

         ApplyBayesGivenEvidUpIs( 0 );
         UpdateBayesProbabilities();
         break; 

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      case 1:  // User says evid is true

         ApplyBayesGivenEvidUpIs( 1 );
         UpdateBayesProbabilities();
         break;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      case 2:   // User doesn't know (at least for now)

         reply = ACTION_CASE_EVIDSKIPPED_ROLLQUEUE;
         break;

   }  // closes switch tree

//======================================================================================================/
   caseBustsKbase = ( numHyposAlive == 0 );
   caseSpentKbase = evidsUnevaluated.empty();
   WriteBayesProbsToReport();

   if ( caseSpentKbase && !mappAt100pct ) {
      userPrompt = prompt_CaseSpentOut_HDYWTP;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut;         // load pointer by pre-C++11 way
      userOptions = choice_ForCaseSpentOut;   
      SetAnswerMinMax( userOptions.size() );
   }
   else if ( mappAt100pct ) {
      userPrompt = prompt_MappAt100Percent_HDYWTP;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_MappAt100Percent;
      userOptions = choice_ForMappAt100Percent;
      SetAnswerMinMax( userOptions.size() );
   }
   else {
      userPrompt = prompt_TopMenu_CaseInProcess_HDYWTP;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_TopMenu_CaseInProcess;
      userOptions = choice_TopMenu_CaseInProcess;
      SetAnswerMinMax( userOptions.size() );
   }
   waitingOnUserToAnswer = true;
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_CaseSpentOut(void) {

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

   case 0:    // User wants to set case at idle until later

      reply = ACTION_CASE_IDLEDBYUSER_REGENGUI;
      break;

   case 1:     // User wants to provide/verify/teach tool a case solution

      userPrompt = prompt_IsMappHypoCorrect;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_IsMappHypoCorrect;
      userOptions = choice_NoYesDnk;
      SetAnswerMinMax( userOptions.size() );
      waitingOnUserToAnswer = true;
      break;

   case 2:     // User wants to delete the focus case from kit's case list

      reply = ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI;
      break;
   }
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_TopMenu_CaseInProcess( void ) {

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

      case 0:    // User wants to set case at idle until later

         reply = ACTION_CASE_IDLEDBYUSER_REGENGUI;
         break;

      case 1:     // User wants EA to ask next question (User wants to party! Woo, woo!)

         // idiomatic as a safety before fronting/popping any queue under ISO C++
         caseSpentKbase = evidsUnevaluated.empty();

         if ( caseSpentKbase ) {
            userPrompt = prompt_CaseSpentOut_HDYWTP;
            p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut; 
            userOptions = choice_ForCaseSpentOut;
            SetAnswerMinMax( userOptions.size() );
         }
         else {
            uaiEvidUp = evidsUnevaluated.front();  evidsUnevaluated.pop();
            userPrompt.clear();
            userPrompt = { { evidTally.at(uaiEvidUp).query } };
            p_ActOnAnswer = &CCase::ActOnAnswerTo_EvaluatePromptedEvidenceQuery;
            userOptions = choice_NoYesDnk;
            SetAnswerMinMax( 3 );
         }
         waitingOnUserToAnswer = true;
         break;

      case 2:     // User wants to provide/verify/teach tool a case solution

         userPrompt = prompt_IsMappHypoCorrect;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_IsMappHypoCorrect;
         userOptions = choice_NoYesDnk;
         SetAnswerMinMax(3);
         waitingOnUserToAnswer = true; 
         break;

      case 3:     // User wants to delete the case

         reply = ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI;
         break;
      }
      return reply;
}



AoaReply_t CCase::ActOnAnswerTo_MappAt100Percent( void ) {

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

   case 0:    // User wants to set case at idle until later

      reply = ACTION_CASE_IDLEDBYUSER_REGENGUI;
      break;

   case 1:     // User wants to provide/verify/teach tool a case solution

      userPrompt = prompt_IsMappHypoCorrect;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_IsMappHypoCorrect;
      userOptions = choice_NoYesDnk;
      SetAnswerMinMax(3);
      waitingOnUserToAnswer = true;
      break;

   case 2:     /* User wants to delete case from case list, the KB having assimilated the case as a
               fault occurrence that remains unfixed (KB will reflect a higher, 'unfixed' prior
               on the hypo found true in this (deleted) case)
               */
      reply = ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI;
      break;
   }
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_CaseVerifiedAndLearned( void ) {

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

   case 0:  /* User wants to set case at idle until later, w/ KB left with the case as an
              'unfixed' occurence (KB will reflect a higher, 'unfixed' prior on the true hypo)
            */
      reply = ACTION_CASE_IDLEDBYUSER_REGENGUI;
      break;

   case 1:     // User wants to teach KB that fault ("true hypo") has been fixed

      userPrompt = prompt_IsFaultFixed;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_IsFaultFixed;
      userOptions = choice_NoYesDnk;
      SetAnswerMinMax(3);
      waitingOnUserToAnswer = true;
      break;

   case 2:     /* User wants to delete case from case list, w/ KB left with the case as an
                  'unfixed' occurence (KB will reflect a higher, 'unfixed' prior on the true hypo)
               */
      reply = ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI;
      break;
   }
   return reply;
}


void CCase::LearnFromUserVerifyingSoln( void ) {

   if ( uaiTrueHypo != 0 ) {  // See Method Note [1]

      for ( auto& hypoIter : hypoTally ) {

// Set status = true if iter at true hypo.  All hypos other than the true hypo must be false
      
         if (hypoIter.first == uaiTrueHypo) { hypoIter.second.hypoStatus = 1; }
         else { hypoIter.second.hypoStatus = 0; }

/* Loop thru the kBase nodes where each case hypo intersects each case evid.
   Increment counts appropriately.  Activate node if no prior activation (i.e., a novel "experience").
*/ 
         for ( auto& evidIter : evidTally ) {

            p_Kbase->PanCursorToNodeAt( caseRuleUai, hypoIter.first, evidIter.first );
            p_Kbase->ReadCursorToImage();

            if (hypoIter.first == uaiTrueHypo) {
               p_Kbase->IncrementHypoTrueForEvid( evidIter.second.evidStatus );
            }
            else { p_Kbase->IncrementHypoFalseForEvid( evidIter.second.evidStatus ); }

            p_Kbase->WriteImageToCursor();
         }
      }
      caseVerifiedAndLearned = true;
   }

 /* METHOD NOTES vvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

   [1]   User (not EA) verifies only one hypo as being "true" (i.e., the actual cause ("fault") of the
         case). That "true hypothesis" may or may not have been the MAPP hypothesis identified by EA.   

^^^^ END METHOD NOTES */
   return;
}


void CCase::LearnFaultHasBeenFixed(void) {

   if ( caseVerifiedAndLearned ) {

      for (auto& evidIter : evidTally) {

         p_Kbase->PanCursorToNodeAt( caseRuleUai, uaiTrueHypo, evidIter.first );
         p_Kbase->ReadCursorToImage();
         p_Kbase->DevolveCounts(); // $$$ TBD to get this 'devolve' working right $$$
         p_Kbase->WriteImageToCursor();
      }
   }
   return;
}


AoaReply_t CCase::ActOnAnswerTo_IsAnyCaseHypoCorrect( void ) { 

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

      case 0:     // User says "No", none of the hypos listed by EA is true

         caseBustsKbase = true;
         userReport = report_CaseBustedKnowledgeBase;
         userPrompt = prompt_ShouldThisCaseBeDeleted;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_ShouldThisCaseBeDeleted;
         userOptions = choice_NoYes;
         SetAnswerMinMax( 2 );
         waitingOnUserToAnswer = true;
         break;

      case 1:     // User says "Yes", one of the hypos listed by EA is true 

         WriteBayesProbsToReport();
         userPrompt = prompt_UserVerifyCorrectHypoByChoosing;
         p_ActOnAnswer = &CCase:: ActOnAnswerTo_UserVerifyCorrectHypoByChoosing;
         userOptions = GenerateMultipleChoiceOfLength( numHyposPrior );
         SetAnswerMinMax( userOptions.size() );
         waitingOnUserToAnswer = true;
         break;

      default:    // User Does Not Know (DNK) whether any of the hypos listed by EA is true

         userReport.push_back( addToReport_trailer_NeedUserToFactuallyVerifySolution_lineOne );
         userReport.push_back( addToReport_trailer_NeedUserToFactuallyVerifySolution_lineTwo );

         if ( caseSpentKbase ) {
            userPrompt = prompt_CaseSpentOut_HDYWTP;
            p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut;
            userOptions = choice_ForCaseSpentOut;
            SetAnswerMinMax( userOptions.size() );
         }
         else {
            userPrompt = prompt_TopMenu_CaseInProcess_HDYWTP;
            p_ActOnAnswer = &CCase::ActOnAnswerTo_TopMenu_CaseInProcess;
            userOptions = choice_TopMenu_CaseInProcess;
            SetAnswerMinMax( userOptions.size() );
         }
         waitingOnUserToAnswer = true;
         break;
   }     
   return reply;
}

AoaReply_t CCase::ActOnAnswerTo_IsMappHypoCorrect( void ) { 

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

      case 0:     // User says "No", MAPP hypo is not correct

         userPrompt = prompt_IsAnyCaseHypoCorrect;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_IsAnyCaseHypoCorrect;
         userOptions = choice_NoYesDnk;
         SetAnswerMinMax( 3 );
         waitingOnUserToAnswer = true;
         break;

      case 1:      // User says "Yes", MAPP hypo is correct

         uaiTrueHypo = uaiMappHypo;   // MAPP value as is, whether it reached 100% or not
         LearnFromUserVerifyingSoln();
         userPrompt = prompt_IsFaultFixed;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_IsFaultFixed;
         userOptions = choice_NoYesDnk;
         SetAnswerMinMax( 3 );
         waitingOnUserToAnswer = true;
         reply = ACTION_CASE_SOLNVERIFIED_UNMASKRULE;
         break;

      default:      // User Does Not Know (DNK) whether MAPP hypo is correct

         userReport.push_back(addToReport_trailer_NeedUserToFactuallyVerifySolution_lineOne);
         userReport.push_back(addToReport_trailer_NeedUserToFactuallyVerifySolution_lineTwo);

         if (caseSpentKbase) {
            userPrompt = prompt_CaseSpentOut_HDYWTP;
            p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut; 
            p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut;
            userOptions = choice_ForCaseSpentOut;
            SetAnswerMinMax( userOptions.size() );
         }
         else {
            userPrompt = prompt_TopMenu_CaseInProcess_HDYWTP;
            p_ActOnAnswer = &CCase::ActOnAnswerTo_TopMenu_CaseInProcess;
            userOptions = choice_TopMenu_CaseInProcess;
            SetAnswerMinMax( userOptions.size() );
         }

         waitingOnUserToAnswer = true;
         break;
   }     
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_IsFaultFixed( void ) {

   AoaReply_t reply = ACTION_NONE;

   switch ( answer ) {

   case 1:     // User says "Yes", the fault (the "true hypo") has been fixed

      LearnFaultHasBeenFixed();
      waitingOnUserToAnswer = false;
      reply = ACTION_KIT_CASEVERIFIEDANDLEARNED_FAULTFIXED_DESTROYANDTELLGUI;
      break;

   default:   // User says "No" or DNK, either way the fault is considered not fixed

      userPrompt.clear();
      userPrompt = { { addToPrompt_trailer_HowDoYouWantToProceed } };
      userOptions = choice_ForCaseVerifiedAndLearned;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseVerifiedAndLearned;
      userOptions = choice_ForCaseVerifiedAndLearned;
      SetAnswerMinMax( userOptions.size() );
      waitingOnUserToAnswer = true;
      break;
   }
   return reply;
}


AoaReply_t CCase::ActOnAnswerTo_UserVerifyCorrectHypoByChoosing( void ) {

   auto iter = hypoTally.begin();
   std::advance( iter, answer );   // Here, 'answer' counts option set lines beginning @ 0
   uaiTrueHypo = iter->first;      // Look up hypoId of User's selection in hypoTally
   LearnFromUserVerifyingSoln();

   userPrompt = prompt_IsFaultFixed;
   p_ActOnAnswer = &CCase::ActOnAnswerTo_IsFaultFixed;
   userOptions = choice_NoYesDnk;
   SetAnswerMinMax( 3 );
   waitingOnUserToAnswer = true;

   return ACTION_CASE_SOLNVERIFIED_UNMASKRULE;
}


void CCase::RegenGuiFieldsPerCaseAsIs( void ) {

   if ( ruleHasDiagnostics ) {

      if ( caseVerifiedAndLearned ) {
            
         userReport = { { "User has verified case as caused by:" },
                        { hypoTally.at(uaiTrueHypo).hypothesis },
                        { "Case will be deleted from list only when User directs or fault reported fixed."}
         };
         userPrompt = prompt_IsFaultFixed;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_IsFaultFixed;
         userOptions = choice_NoYesDnk;
         SetAnswerMinMax( 3 );
         waitingOnUserToAnswer = true;
      }
      else if ( caseSpentKbase ) {

         UpdateBayesProbabilities();
         WriteBayesProbsToReport();
         userPrompt = prompt_CaseSpentOut_HDYWTP;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_CaseSpentOut;
         userOptions = choice_ForCaseSpentOut;
         SetAnswerMinMax( userOptions.size() );
         waitingOnUserToAnswer = true;
      }
      else if ( caseBustsKbase ) {

         userReport = report_CaseBustedKnowledgeBase;
         userReport.push_back(   "EA not able to isolate any cause for " +
                                 RuleRef.SayRuleUaiText() +
                                 " failure." );
         userPrompt = prompt_ShouldThisCaseBeDeleted;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_ShouldThisCaseBeDeleted;
         userOptions = choice_NoYes;
         SetAnswerMinMax( 2 );
         waitingOnUserToAnswer = true;
      }
      else {

         UpdateBayesProbabilities();
         WriteBayesProbsToReport();
         userPrompt = prompt_TopMenu_CaseInProcess_HDYWTP;
         p_ActOnAnswer = &CCase::ActOnAnswerTo_TopMenu_CaseInProcess;
         userOptions = choice_TopMenu_CaseInProcess;
         SetAnswerMinMax( userOptions.size() );
         waitingOnUserToAnswer = true;
      }
   }
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   else {

      /* CRule object "has no diagnostics" = limited, constant report on any case it generates.
         Such a CRule object will have empty p_AssocHypo, so it has index on Rule-axis of Kbase, but no
         intersection with Hypo-axis or Evid-axis.  User can chose only no/yes on deleting such a CCase. 
      */

      userReport = { { reportPreamble },
                     { "Notification only. Case has no diagnostic available."}
      };
      userPrompt = prompt_ShouldThisCaseBeDeleted;
      p_ActOnAnswer = &CCase::ActOnAnswerTo_ShouldThisCaseBeDeleted;
      userOptions = choice_NoYes;
      SetAnswerMinMax(2);
      waitingOnUserToAnswer = true;
   }
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// public methods


GuiPackCaseFull_t CCase::SayFullGuiPack( void ) const {

// for report and prompt, c-tor args were: std::queue<std::string, std::deque<std::string>>( userPrompt )

   return SGuiPackCaseFull(   LookUpGuiType(ownApiType),
                              ownGuiKey,
                              u_SnapshotKrono->SayGuiKey(),
                              caseName,
                              userReport,
                              userPrompt,
                              userOptions );
}

GuiPackCaseDyna_t CCase::SayDynamicGuiPack( void ) const {

   return SGuiPackCaseDyna( userReport,
                            userPrompt,
                            userOptions );
}


std::string CCase::SayCaseName( void ) const { return caseName; }


Nzint_t CCase::SaySnapshotSetSgi( void ) const { return snapshotSetSgi; }


time_t CCase::SayWhenTrapped( void ) const { return timeTrapped; }


int CCase::SayDollarsPerDayCost( void ) const { return 0; } // $$$ TBD to implement $$$


AoaReply_t CCase::CheckAnswerValidThenSet( size_t answerGuiSent ) {

   if ( (answerGuiSent < answerMinMax[0]) || (answerGuiSent > answerMinMax[1]) ) {
      return ACTION_KIT_ANSWERNOTALLOWED_TELLGUI;  // 'answer' left as is
   }
   if ( ! waitingOnUserToAnswer ) {
      return ACTION_KIT_CASEUSERACCESSCLASH;       // 'answer' left as is
   }
   answer = answerGuiSent;
   waitingOnUserToAnswer = false;
   AoaReply_t reply = (this->*p_ActOnAnswer)();    // pre-C++11 style dynamic action

   switch ( reply ) {   // Intercept only those reply codes requiring case (versus the kit) to act

      case ACTION_CASE_EVIDSKIPPED_ROLLQUEUE:
         // Roll queue through to next evid unevaluated, as User DNK answer to this evid item
         evidsUnevaluated.push( uaiEvidUp );
         break;

      case ACTION_CASE_IDLEDBYUSER_REGENGUI :
         RegenGuiFieldsPerCaseAsIs();
         break;

      case ACTION_CASE_SOLNVERIFIED_UNMASKRULE:
         // case holding a verified soln, so unmask rule that created it
         //p_Rule->SetTrapMaskAs(false);
         break;

      default:    // remaining reply codes all actions to be done by CaseKit object owning Case
         break;
   }

/* METHOD NOTES vvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

   [1]   Not all actions performed by the case object can be defined within the context of a dynamically
         called method.  Follow-on dynamically defined actions may be required after the method returns.
         Those are "services" requested of code outside the method, encoded via the value of the char they
         return.  Services requiring context of the case object itself are defined in the switch tree seen
         here. Services that must be done by the case kit object (e.g., case obj deletion) are defined in a
         switch tree in the kit's caller method, per the return value of the dynamically called method.

         The default returning a reply = 'X', unless overwritten, signals kit that answer User gave has
         failed the check in the checking/setting method. 

^^^^ END METHOD NOTES */

   return reply;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CCaseKit implementation

CCaseKit::CCaseKit(  ASubject& arg0,
                     const SEnergyPrices& arg1,
                     std::string arg2 )
                     :  SubjRef (arg0),
                        ViewRef ( arg0.SayViewRef() ),
                        u_Cases_byKey(),
                        RankCasesOldestToNewest (
                           [&] ( NGuiKey sortedAheadOnTrue, NGuiKey sortedBehindOnTrue ) -> bool {

                           // "ahead"/"front" = lower index, "behind"/"back" = higher index
                           // Only arrays and vectors have an "index" concept (i.e., are contiguous)
                           // difftime(tLater - tEarlier) => diff seconds as positive dbl
                           // difftime(tEarlier - tLater) => diff seconds as negative dbl

                              return ( (  std::difftime( u_Cases_byKey[sortedAheadOnTrue]->SayWhenTrapped(), 
                                                         u_Cases_byKey[sortedBehindOnTrue]->SayWhenTrapped()
                                          ) < 0.0 ) ?
                                          true : false
                              );
                           }
                        ),
                        RankCasesMostCostToLeast (
                           [&] ( NGuiKey sortedAheadOnTrue, NGuiKey sortedBehindOnTrue ) -> bool {
                              return ( u_Cases_byKey[sortedAheadOnTrue]->SayDollarsPerDayCost() > 
                                       u_Cases_byKey[sortedBehindOnTrue]->SayDollarsPerDayCost()
                              );
                           }
                        ),
                        caseKeysByDecrRank(0),
                        energyPricesRef (arg1),
                        filenameDiskArchive (arg2 + "_casekit.xml"),
                        keyOfCaseJustDestroyed (0),
                        rankingCasesByCostNotAge (false) {

   /*
   if ( ArchivePreexists( kitArchFilename ) ) {     // TBD for full path to app-designated directory
      RestoreCaseKitFromFile();
   }
   */
   u_Cases_byKey.reserve(FIXED_CASEKIT_NUMCASESOUT_MAX); 

}


CCaseKit::~CCaseKit( void ) {

   // Empty d-tor
}


//======================================================================================================/
// private methods

EGuiReply CCaseKit::DestroyCase( NGuiKey keyOfCaseToDestroy ) {

   if ( u_Cases_byKey.count( keyOfCaseToDestroy ) == 0 ) {
       return EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
   }

   u_Cases_byKey.at( keyOfCaseToDestroy ).reset();
      // Reset of smart ptr calls d-tor of obj it manages
      // (CCase d-tor sets CRule assoc. w/ destroyed case back to "autoMode")

   u_Cases_byKey.erase( keyOfCaseToDestroy );          // unordered-map erases key-value pair from itself
   caseKeysByDecrRank.remove( keyOfCaseToDestroy );   // calls "==" operation on NGuiKey, which it has
   return EGuiReply::OKAY_done_caseDestroyedByUser_discardKey;
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

EGuiReply CCaseKit::RankCasesOnCostNotAge( bool knobInput ) {

   rankingCasesByCostNotAge = knobInput;
   return EGuiReply::OKAY_allDone;
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

void CCaseKit::RegenCaseRankings( void ) {

   caseKeysByDecrRank.clear();  // std::forward_list to provide faster sorts

   for ( const auto& pairRef_case : u_Cases_byKey ) {

      caseKeysByDecrRank.push_front( pairRef_case.first );
   }
   if ( u_Cases_byKey.size() > 1 ) {

      // Lambdas here are functor arguments to a sort, so no "()" operators at end of their names
      caseKeysByDecrRank.sort( ( rankingCasesByCostNotAge ?
                                    RankCasesMostCostToLeast : 
                                    RankCasesOldestToNewest )
      );
   }
   return;
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
/*
void CCaseKit::RestoreCaseKitFromFile( void ) {

   std::ifstream inFlow( filenameDiskArchive.c_str() );
   cereal::XMLInputArchive archive( inFlow );

   int test = 0;

   archive(test);

   std::cout << "Restored value of test is: " << std::to_string(test) << std::endl;

   inFlow.close();

   return;
}


void CCaseKit::SaveCaseKitToFile( void ) {

   std::ofstream outFlow( filenameDiskArchive.c_str() );
   cereal::XMLOutputArchive archive( outFlow );

   int test = 3;

   archive( CEREAL_NVP(test) );

   //outFlow.close();

   return;
}
*/

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// public methods

GuiPackCaseFull_t  CCaseKit::SayFullGuiPackFromCase( NGuiKey keyGiven ) const {

   return ( u_Cases_byKey.count( keyGiven ) == 0 ?
            SGuiPackCaseFull( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            u_Cases_byKey.at( keyGiven )->SayFullGuiPack() 
            // must use at() since method is const, [] is non-const 
      );
}


GuiPackCaseDyna_t  CCaseKit::SayDynamicGuiPackFromCase( NGuiKey keyGiven) const {

   return ( u_Cases_byKey.count( keyGiven ) == 0 ?
            SGuiPackCaseDyna( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            u_Cases_byKey.at( keyGiven )->SayDynamicGuiPack()
            // must use at() since method is const, [] is non-const 
      );
}


std::vector<NGuiKey> CCaseKit::SayCaseKeysByDecrRank( void ) const {

   return std::vector<NGuiKey>( caseKeysByDecrRank.begin(), caseKeysByDecrRank.end() );
}


std::vector<std::string> CCaseKit::SayCaseNamesByDecrRank( void ) const {

   std::vector<std::string> reply(0);
   reply.reserve( u_Cases_byKey.size() );
   for ( const auto& keyRef : caseKeysByDecrRank ) {
      reply.push_back( u_Cases_byKey.at(keyRef)->SayCaseName() );
   } 
   return reply;
}


//======================================================================================================/

EApiReply CCaseKit::CreateAndOwnCase(  CRuleKit& ruleKitRef,
                                       ASubject& subjRef,
                                       time_t trapTime,
                                       int trapTrigger,
                                       int secsPerRuleCycle,
                                       CRule& ruleRef,
                                       const CTraceRealtime& ruleRtTraceRef ) { 

   EApiReply reply = EApiReply::Okay_tallyZero; // default reply should following 'if' test as 'false'

   if ( u_Cases_byKey.size() < FIXED_CASEKIT_NUMCASESOUT_MAX ) {

      std::unique_ptr<CCase> u_Case_locallyScopedOnHeap = 
         std::make_unique<CCase>(   ruleKitRef,
                                    subjRef,
                                    trapTime,
                                    trapTrigger,
                                    secsPerRuleCycle,
                                    ruleRef,
                                    ruleRtTraceRef,
                                    ruleRef.HasDiagnostics()
         );

      NGuiKey keyOfNewCase( u_Case_locallyScopedOnHeap->SayGuiKey() );

      std::pair<CaseOwnershipTable_t::iterator, bool> caseTableVerbReply =
         u_Cases_byKey.emplace(
            std::pair< NGuiKey, std::unique_ptr<CCase> >(
               keyOfNewCase,
               u_Case_locallyScopedOnHeap.release()
         )
      );
      RegenCaseRankings();

      ViewRef.AddCaseToCaseKitLookup( keyOfNewCase, this ); // Removal from LUT is by CCase d-tor

      //SaveCaseKitToFile();

      reply = EApiReply::Okay_tallyOne;
   }
   return reply;
}

//======================================================================================================/

EGuiReply CCaseKit::AnswerCasePromptUsingOptionIndex( NGuiKey caseKey, size_t iOption ) {

   EGuiReply reply = EGuiReply::OKAY_allDone;

   if ( u_Cases_byKey.count(caseKey) > 0 ) { 

      AoaReply_t caseReply = u_Cases_byKey[caseKey]->CheckAnswerValidThenSet( iOption );

      switch ( caseReply ) { 
 
         case ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI : 
            reply = DestroyCase( caseKey );
            break;

         case ACTION_KIT_CASEVERIFIEDANDLEARNED_FAULTFIXED_DESTROYANDTELLGUI :
            reply = DestroyCase( caseKey );
            break;

         case ACTION_KIT_ANSWERNOTALLOWED_TELLGUI :
            reply = EGuiReply::FAIL_set_givenDialogueAnswerNotAllowed;
            break;

         default:
            break;
      }
   }
   return reply;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
