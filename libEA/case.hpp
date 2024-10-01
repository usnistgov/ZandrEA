// This file is in EA library of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares CCase and CCaseKit classes for AFDD diagnostic "cases"
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef CASE_HPP
#define CASE_HPP

#include "diagnosticTypes.hpp"

//#include <cereal/archives/binary.hpp>
//#include <cereal/archives/xml.hpp>

#include <functional>
#include <memory>
#include <queue>
#include <tuple>
#include <forward_list>
#include "customTypes.hpp"
#include "guiShadow.hpp"


// Forward declares (to avoid unnecessary #includes)
class ASubject;

class CController;
class CKnowBaseH5;
class CKronoSnapshot;
class CPaneSnapshot;
class CRule;
class CRuleKit;
class CTraceSnapshot;
class CView;

// typedefs
typedef std::unordered_map<NGuiKey, std::unique_ptr<CTraceSnapshot>> SsTraceOwnershipTable_t;
typedef std::unordered_map<NGuiKey, std::unique_ptr<CPaneSnapshot>>  SsPaneOwnershipTable_t;
typedef std::string                                                  GuiMsgInsert_t;
typedef std::vector<std::string>                                     GuiMsgPacket_t;
typedef std::vector<std::string>                                     GuiOptionSet_t;

// File-specific definitions
const Nzint_t MAXCASESOUT = 3;

// for readability inside CCaseRankerFunc defn
const size_t CASEID = 0;
const size_t FAILCOST = 1;
const size_t TRAPTIME = 2;

/*
Complete lexicon for actions on answer (AOA) to a prompt sent to GUI
e.g., the action taken based on answer User gave to an evidence query.  Action is either
performed by Case object or performed by Case Kit object.
*/
enum AoaReply_t { 


   ACTION_NONE,
   ACTION_CASE_ALLEVIDSSPENT_NONE,        // TBD on what case does in this situation
   ACTION_CASE_EVIDANSWERED_CONTINUE,
   ACTION_CASE_EVIDSKIPPED_ROLLQUEUE,
   ACTION_CASE_IDLEDBYUSER_REGENGUI,
   ACTION_CASE_SOLNVERIFIED_UNMASKRULE,   // unmasked even if not fixed, so recurrence compels fixing

   ACTION_KIT_CASEDROPPED_DESTROYANDTELLGUI,
   ACTION_KIT_CASEVERIFIEDANDLEARNED_FAULTFIXED_DESTROYANDTELLGUI,
   ACTION_KIT_ANSWERNOTALLOWED_TELLGUI,
   ACTION_KIT_CASEUSERACCESSCLASH
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////

class CCase : public IGuiShadow { 

   typedef AoaReply_t (CCase::*AOA_t)(void);    // pre-C++11 way of doing dynamic action by PMF

   public:
   // Methods

      CCase(   CRuleKit&,
               ASubject&,                    // to send user alert to Domain
               time_t,                       // timestamp of latest FAIL on Rule ("trap time")
               int,                          // triggerCount at time trapped
               int,                          // secsPerCycle of Rule
               CRule&,                       // See Class Note [1]
               const CTraceRealtime&,
               bool );                       // Rule has diagnostics


      ~CCase( void );

      GuiPackCaseFull_t                SayFullGuiPack( void ) const;
      GuiPackCaseDyna_t                SayDynamicGuiPack( void ) const;
      std::string                      SayCaseName( void ) const;
      Nzint_t                          SaySnapshotSetSgi( void ) const;
      time_t                           SayWhenTrapped( void ) const;
      int                              SayDollarsPerDayCost( void ) const;
      AoaReply_t                       CheckAnswerValidThenSet( size_t );


   private:

   // Private static fields
      static const GuiMsgInsert_t   addToReport_trailer_NeedUserToFactuallyVerifySolution_lineOne;
      static const GuiMsgInsert_t   addToReport_trailer_NeedUserToFactuallyVerifySolution_lineTwo;
      static const GuiMsgInsert_t   addToPrompt_trailer_HowDoYouWantToProceed; // = HDYWTP "trailer"

      // Report typically not static, but generated lazy by a method.  One report is static.
      static const GuiMsgPacket_t   report_CaseBustedKnowledgeBase;  // a bust makes full report moot
      
      static const GuiMsgPacket_t   prompt_ThatAnswerInvalidChooseOnlyAsAllowed;
      static const GuiMsgPacket_t   prompt_TopMenu_CaseInProcess_HDYWTP;
      static const GuiMsgPacket_t   prompt_ShouldThisCaseBeDeleted;  // used after reporting a bust
      static const GuiMsgPacket_t   prompt_CaseSpentOut_HDYWTP;
      static const GuiMsgPacket_t   prompt_MappAt100Percent_HDYWTP;
      static const GuiMsgPacket_t   prompt_IsMappHypoCorrect;
      static const GuiMsgPacket_t   prompt_IsAnyCaseHypoCorrect;
      static const GuiMsgPacket_t   prompt_IsFaultFixed;
      static const GuiMsgPacket_t   prompt_UserVerifyCorrectHypoByChoosing;

      static const GuiOptionSet_t   choice_NoYes;      // typ for case management questions
      static const GuiOptionSet_t   choice_NoYesDnk;   // typ for evaluating evidence questions
      // Multiple choice options per enumerating letters in report are generated by a method
      static const GuiOptionSet_t   choice_TopMenu_CaseInProcess;
      static const GuiOptionSet_t   choice_ForMappAt100Percent;
      static const GuiOptionSet_t   choice_ForCaseVerifiedAndLearned;
      static const GuiOptionSet_t   choice_ForCaseSpentOut;

   // Handles [must be "knows-a" vs. "has-a", since numbers/type of each only known at runtime]

      SsTraceOwnershipTable_t          u_SnapshotTracesOfObjectsAntecedentToCaseRule_byKey;
      SsPaneOwnershipTable_t           u_PanesInSnapshotKrono_byKey;
      std::unique_ptr<CTraceSnapshot>  u_SnapshotTraceOfCaseRule;
      std::unique_ptr<CKronoSnapshot>  u_SnapshotKrono;

      CRule&               RuleRef; // nonconst obj allows CCase to edit CRule (TBD, not yet used)
      CView&               ViewRef; // Trace/Pane/Krono d-tors call on View to release ptrs
      CKnowBaseH5* const   p_Kbase;

      //std::function<int(CCase*)> p_ActOnAnswer;  // not working, not calling '=' overload in .cpp
      AOA_t                p_ActOnAnswer;

   // Private member fields

      HypoTallyMap_t       hypoTally;        // Maps hypoId to a SHypoTally struct
      EvidTallyMap_t       evidTally;        // Maps evidId to char giving status 'F', 'T', or 'U'
      std::queue<Nzint_t>  evidsUnevaluated;
      GuiMsgPacket_t       userReport;    // "report" must be purely declarative, leader is userAlert
      GuiMsgPacket_t       userPrompt;    // "prompt" may begin declarative, must end interrogative
      GuiOptionSet_t       userOptions;   // currently only multichoice index 0 thru N into vector
      std::array<size_t,2> answerMinMax;  // per above, currently only multichoice w/ min = 0
      const time_t         timeTrapped;
      Tally_t              sumPriorCountsTrueOfAllCaseHypos;
      Tally_t              sumJointPostCountsAccumByHyposAlive;
      const Nzint_t        caseSgi;
      const std::string    caseName;
      const std::string    reportPreamble;  // const message describing what case is about
      const Nzint_t        snapshotSetSgi;
      const Nzint_t        caseRuleUai;
      Nzint_t              uaiMappHypo;    // uai of hypo having maximum a-posterori probability
      Nzint_t              uaiTrueHypo;    // uai of hypo provided/verified by User as true case soln
      Nzint_t              uaiEvidUp;
      size_t               answer;
      size_t               caseRank;
      size_t               numHyposAlive;
      size_t               numHyposPrior;
      bool                 noEvidenceEvaluated;
      bool                 caseBustsKbase; //"Bust" = every case hypo (incl. false alarm) found false
      bool                 mappAt100pct;   // See Class Note [4]
      bool                 caseSpentKbase; //"Spent" = every case evid evaluated either True or False
      const bool           ruleHasDiagnostics;
      bool                 caseVerifiedAndLearned;
      static bool          kBasePtrSet;
      bool                 waitingOnUserToAnswer;

   // Private methods

      static std::string      InitReportPreamble( ERealName, time_t, int, int, const CRule& );
      static GuiOptionSet_t   GenerateMultipleChoiceOfLength( size_t );

      void                 LoadSnapshotKronoWithLogicChainOfCase( CRuleKit& );

      void                 SetAnswerMinMax( size_t );
      void                 ApplyBayesGivenEvidUpIs( size_t );
      void                 UpdateBayesProbabilities( void );
      void                 WriteBayesProbsToReport( void );
      void                 LearnFromUserVerifyingSoln( void );
      void                 LearnFaultHasBeenFixed(void);
      void                 RegenGuiFieldsPerCaseAsIs( void );

      AoaReply_t           ActOnAnswerTo_ShouldThisCaseBeDeleted( void );
      AoaReply_t           ActOnAnswerTo_EvaluatePromptedEvidenceQuery( void );
      AoaReply_t           ActOnAnswerTo_CaseSpentOut( void );
      AoaReply_t           ActOnAnswerTo_TopMenu_CaseInProcess( void );
      AoaReply_t           ActOnAnswerTo_MappAt100Percent( void );
      AoaReply_t           ActOnAnswerTo_CaseVerifiedAndLearned( void );
      AoaReply_t           ActOnAnswerTo_IsMappHypoCorrect( void );
      AoaReply_t           ActOnAnswerTo_IsAnyCaseHypoCorrect( void );
      AoaReply_t           ActOnAnswerTo_UserVerifyCorrectHypoByChoosing( void );
      AoaReply_t           ActOnAnswerTo_IsFaultFixed( void );


/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[2]   CCase object's dialogue with human user allows FOUR modes through which user is aked (via GUI) to
      provide information to EA, by answering "prompt messages" from the CCase object:
         1) Mode 'B' = binary choice, with no opt out if user unsure, i.e., :
                  { 0, 1 }  to be interpreted as "no/false" (0) or "yes/true" (1)
         2) Mode 'M' = multiple choice:
                  { 0, 1, 2, ... N }, read as "A", "B", "C" up to no. of choices allowed via mode setter
         3) Mode 'R' = signed integer within a range given by mode setter:
                  { bottom, ... top } , in unit increments
         4) Mode 'T' = ternary choice, with an opt out if user unsure, i.e., :
                  { 0, 1, 2 }  to be interpreted only as "no", "yes", or "do not know" (DNK)
 

[3]   "MAPP" = 'maximum a priori probability', or 'maximum a posteriori probability', depending on
      the context where it's used (i.e., 'priori' applies prior to first evidence answered,
      'posteriori' applies after first evidence annswered.)

^^^^ END CLASS NOTES */
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Purpose of "CaseKit" object is to create and manage the temporary lifetimes of the individual "case"
   objects resulting when rules in the "RulesKit" are trapped as repeatedly "failed".  Thus, all fields
   and methods (attributes and behaviors) of rule-based alarming and diagnosis having a permanent "life"
   go in a "RulesKit", while fields and methods related to the (temporary) life of individual "cases"
   go in the corresponding "CaseKit".
*/

typedef std::unordered_map<NGuiKey, std::unique_ptr<CCase>>    CaseOwnershipTable_t; 

class CCaseKit { 

   public:

      CCaseKit(   ASubject&,              // non-const to register at 1:1 for ISubject:CCaseKit
                  const SEnergyPrices&,   // passed as const ref to allow R-T variations
                  std::string );          // name preamble for filenameDiskArchive

      ~CCaseKit( void );

      // Public Methods called externally from GUI
      GuiPackCaseFull_t                SayFullGuiPackFromCase( NGuiKey ) const;
      GuiPackCaseDyna_t                SayDynamicGuiPackFromCase( NGuiKey ) const;
      std::vector<NGuiKey>             SayCaseKeysByDecrRank( void ) const;
      std::vector<std::string>         SayCaseNamesByDecrRank( void ) const;
      EGuiReply                        AnswerCasePromptUsingOptionIndex( NGuiKey, size_t );

      // Public Methods called internally within API
      EApiReply                        CreateAndOwnCase( CRuleKit&,
                                                         ASubject&,
                                                         time_t,
                                                         int,
                                                         int,
                                                         CRule&,
                                                         const CTraceRealtime& );

   private:

   // Handles, private
      ASubject&                                             SubjRef;
      CView&                                                ViewRef;
      CaseOwnershipTable_t                                  u_Cases_byKey;      // See Class Note [1]

   // Objects (Ranking requires std:sort to access to object members, so can't be statics)
      const std::function<bool(NGuiKey,NGuiKey)>            RankCasesOldestToNewest; 
      const std::function<bool(NGuiKey,NGuiKey)>            RankCasesMostCostToLeast;

   // Fields
      std::forward_list<NGuiKey>       caseKeysByDecrRank;
      const SEnergyPrices&             energyPricesRef;      // units defined in CDomain (e.g., $/day)
      const std::string                filenameDiskArchive;  // fstream bin target, ext = .eak 
      NGuiKey                          keyOfCaseJustDestroyed;
      bool                             rankingCasesByCostNotAge;

   // Friends
      //friend class cereal::access;        // Required per cereal:: documentation

   // Methods
      EGuiReply                        DestroyCase( NGuiKey );
      EGuiReply                        RankCasesOnCostNotAge( bool );
      void                             RegenCaseRankings( void );
      //void                             SaveCaseKitToFile( void );
      //void                             RestoreCaseKitFromFile( void );

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Since CCase objects have temporal (vs. eternal) lifespan, smart ptrs are used.  Further, ALL access
      to CCase pointers (and thus, CCase objects themselves) is limited to one place, the "p_Case" map
      of the CCaseKit object.

[2]   Approach is that dollar costs of faults make no difference in EA logic until reaching the point
      of a CCaseKit object assigning "rank" to the CCase objects belonging to it.  Thus, SEnergyCost
      struct is NOT assigned to ASubject (which would make it not directly accessible to CCaseKit) but
      only to CCaseKit itself.

      A CCaseKit object has a 1:1 correspondence to a particular CRuleKit object, and thus to one
      particular ASubject object (i.e., one specific item of equipment... e.g., "VAV Box #127" )        

^^^^ END CLASS NOTES */

};


#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
