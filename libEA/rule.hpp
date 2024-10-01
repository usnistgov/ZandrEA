// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares CRule concrete class for all Rule objects 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef RULE_HPP
#define RULE_HPP

#include "seqElement.hpp"      // Inheiritance requires type completion, also brings "customTypes.hpp"
#include <string>
#include <functional>
#include <memory>

 // Forward declares
class AFact;
class ASubject;

class CCase;
class CCaseKit;
class CController;
class CDisplayRuleKit;
class CHistogramRule;
class CHypo;
class CKnobBool;
class CKnobSelectNzint;
class CKnowBaseH5;
class CKronoRealtime;
class CPaneRealtime;
class CRainRuleKit;
class CRuleKit;
class CSeqTimeAxis;
class CTraceRealtime;
class CView;

struct SEnergyPrices;

typedef std::unordered_map<NGuiKey, std::unique_ptr<CPaneRealtime>>     RtPaneOwnershipTable_t;
typedef std::unordered_map<Nzint_t, std::unique_ptr<CTraceRealtime>>    RtRuleTraceOwnershipTable_t;


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare one class for any and all AFDD rules

class CRule { 

   public:

    // Methods
      CRule(   CController&,
               CRuleKit&,
               Nzint_t,
               EAlertMsg, 
               EAlertMsg,
               EAlertMsg,
               EAlertMsg,
               std::vector<AFact*>,
               std::vector<AFact*>,
               std::function<bool(void)>,
               std::function<bool(void)>,
               bool );
//             std::function<int( const CRule&, const SEnergyCosts& )> );

      ~CRule( void );

      const RtTraceAccessTable_t&   SayRealtimeAccessTable( void ) const;
      const std::vector<NGuiKey>&   SayCrefToKnobKeys( void ) const;
      std::string                   SayRuleUaiText( void ) const;
      EAlertMsg                     SayFocusMsg( void ) const;
      EAlertMsg                     SayTest_If( void ) const;
      EAlertMsg                     SayTest_Then( void ) const;
      EAlertMsg                     SayFailureMsg( void ) const;
      Nzint_t                       SaveAntecedentSnapshotsAndSaySetSgi( void );   // See Class Note [2]
      Nzint_t                       SayRuleUai( void ) const;
      NGuiKey                       SayKeyToOwnKnob( void ) const;
      Nzint_t                       SaySnapshotSetSgi( void ) const;
      Bindex_t                      CycleAndSayBindex( void );
      bool                          IsPinnedToUnitOutput( void ) const;
      bool                          HasDiagnostics( void ) const;
      bool                          HasSnapshotSet( void ) const;
      bool                          IsInAutoMode( void ) const;
      void                          SetCaseModeTo( bool );
      void                          SetIdleModeTo( bool );
      void                          LendHistogramKeysOfAntecedentsTo( std::vector<NGuiKey>& ) const;

      EApiReply                     DestroySnapshotSet( void );
      void                          AssociateHypo( CHypo* const );
      void                          BuildRuleIntoKbase( CKnowBaseH5& );    // See Class Note [3]  

      int      SayDollarPerDayFaultCost( const SEnergyPrices& );           // See Class Note [1]

   private:

   // Handles
      const CRuleKit&               RuleKitRef;
      std::unique_ptr<CKnobBool>    u_Knob;
      std::vector<CHypo*>           p_AssocHypo;

   // Vectors of handles to individual boolean input facts (operands)
      const std::vector<AFact*>    p_OperandsIf;
      const std::vector<AFact*>    p_OperandsThen;

   // Lambdas capturing operands by ref, wrapped in std::function 
      const std::function<bool( void )>                              TestIf; 
      const std::function<bool( void )>                              TestThen;
      const std::function<int( const CRule&, const SEnergyPrices& )> EstmCostOfFault;
  
   // Fields
      const RtTraceAccessTable_t    p_RtTracesOfAntecedents_byKey;
      std::vector<NGuiKey>          knobKeys_ownedAndAntecedent;
      std::vector<Nzint_t>          assocHypoUai;           // See Class Note [4]
      const EAlertMsg               msg_ruleFocus;
      const EAlertMsg               msg_IfTest;
      const EAlertMsg               msg_ThenTest;
      const EAlertMsg               msgUponFailure;

      const size_t                  numOperandsIf;
      const size_t                  numOperandsThen;
      unsigned int                  caseModeOffset;
      unsigned int                  idleModeOffset;
      static Nzint_t                sgiForNextSnapshotSet;  // See Class Note [5]
      const Nzint_t                 ruleUai;
      Nzint_t                       snapshotSetSgi;         // See Class Note [6]
      Bindex_t                      bindexNow;
      bool                          holdingSnapshots;
      bool                          resultIf;
      bool                          resultThen;
      bool                          isRuleAtIdle;           // needed for implementing a knob
      const bool                    isRulePinnedToUnitOutput;
      bool                          valid;

   // Methods
      static Nzint_t                GenerateSgiForNewSnapshotSet( void );
      RtTraceAccessTable_t          TabulateRealtimeAccessToAntecedents( void );
      bool                          PullValidity( void );

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Only a CRule object has ready access to all the process information that might be needed to
      calculate an estimate of the energy/day being wasted as long as Failed() tests = true.  So, 
      lambda was put here, instead of in CCase or CCaseKit to allow test of whether lambdas can
      capture or be passed 'this' of objects running them (woudl make lambda able to access fields
      in class of hosting object.

[2]   CRule calls for snapshots of its anecedents ( which subsequently, each call for
      snapshots of their own inputs (fact or analog)).  But snapshot of CRule itself must be done by
      CRainRuleKit, as CRuleKitRain holds bindexLogs 

[3]   Called by CRuleKit obj from BuildRuleKitIntoKbase( CKnowBase* ), passing ptr to kbase as actual arg.
      
[4]   Kbase needs to know hypoId of ALL hypos associated to a rule in order to properly initialize the
      nodes (i.e., prior occurence arrays) accessed from that rule.  So, iters to tally of all associated
      hypoId must be passed to CKnowBase obj as an arg of AddRuleToKbase() call. 

[5]   Since static, scope of any given snapshotSetSgi is across entire app instance

[6]   = 0 when Rule holds no snapshot.          

^^^^ END CLASS NOTES */

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Concrete class for "Rule Kit" objects.  Rule Kit, by being an ISeqElement subclass, is link between
   CSequence (source of triggering) and the cycling of individual CRule objects as a group at a specific
   intended frequency [via each CRule being "known to" (i.e., "observer" of) a particular "Rule Kit"]
*/

class CRuleKit  : public ISeqElement {

   public:

      CRuleKit(   CSequence&,
                  ASubject&,
                  EDataLabel, 
                  int,           // seconds between trap action on rules failed
                  CController&,
                  int = 1 );     // triggers per cycle (tpc) of kit (and all CRule objects held by it)

      ~CRuleKit( void );

   // Handles, public
      std::unique_ptr<CRainRuleKit> const       u_RainRuleKit;
      std::unique_ptr<CKnowBaseH5> const        u_Kbase;
      std::unique_ptr<CDisplayRuleKit> const    u_GuiDisplay;

   // Methods
      std::vector<std::string>         SayRuleLabels_GuiTopToBottom( void ) const;
      std::vector<std::string>         SayRuleTexts_If_GuiTopToBottom( void ) const;
      std::vector<std::string>         SayRuleTexts_Then_GuiTopToBottom( void ) const;
      std::vector<EGuiState>           SayNewestRuleStates_GuiTopToBottom( void ) const;
      std::vector<NGuiKey>             SayRuleKitKnobKeys( void ) const;      
      std::vector<NGuiKey>             SayKnobKeyOfEachRule_GuiTopToBottom( void ) const;
      std::vector<NGuiKey>             SayHistogramKeyOfEachRule_GuiTopToBottom( void ) const;
      std::string                      SayKitCaption( void ) const;
      std::string                      SayKitSgiOfNumberAsText( void) const;
      CCaseKit&                        SayCaseKitRef( void ) const;
      CController&                     SayCtrlrRef( void ) const;
      CSeqTimeAxis&                    SayTimeAxisRef( void ) const;
      Nzint_t                          CheckRuleUaiFreeThenKeep( Nzint_t );
      NGuiKey                          SayRealtimeKronoKey( void ) const;
      NGuiKey                          SayHistogramKey( void ) const;
      void                             AddRuleToKit( CRule* const );
      void                             ClearKitOfRealtimeKronoParts( void );

      // Called explicitly by tool.cpp only after all rules/hypos/evid registered to kit:
      // $$$ (Yes, smelly, but do not now see TBD alternative) $$$
      void                             FinalizeRuleKitAndBuildKbase( void );
 
   private:

      // friend CDisplayRuleKit;  $$$ TBD; repl display's public method calls on kit with friending $$$

    // Handles
      RuleUaiToPtrTable_t              p_Rules_byUai;
      RtRuleTraceOwnershipTable_t      u_RealtimeTracesForRulesInKit_byUai;
      CCaseKit&                        CaseKitRef;
      CController&                     CtrlrRef;      // for knob on r-t krono
      CView&                           ViewRef;
      AKnob*                           p_KnobSelectingRuleUai;

      RtTraceAccessTable_t             p_TracesInRealtimeKrono_byKey;
      RtPaneOwnershipTable_t           u_PanesInRealtimeKrono_byKey;
      std::unique_ptr<CKronoRealtime>  u_RealtimeKrono;
      
   // Fields
      std::vector<Nzint_t>             ruleUais_guiTopToBottom;
      std::vector<NGuiKey>             knobKeysOfRuleKitItself;
      std::vector<NGuiKey>             knobKeyOfEachRuleInKit_guiTopToBottom;
      std::vector<NGuiKey>             histogramKeyOfEachRuleInKit_guiTopToBottom;
      const Nzint_t                    kitSgiFromSubject;
      Nzint_t                          uaiOfRuleInRtKrono_zeroIfNoneOrAll;
      bool                             areAllRulesNotInCaseModePutToIdle;  // needed for implementing a knob
      bool                             isRealtimeKronoShowingAllRules;     // needed for implementing a knob
      bool                             kitFinalized;
                         

   // Methods
      std::string       GenerateKronoCaption_OneRule( Nzint_t ) const;
      std::string       GenerateKronoCaption_AllRulesInKit( void ) const;
      EGuiReply         IdleAllRulesNotInCaseMode( bool );
      EGuiReply         LoadRealtimeKronoWithLogicChainOfRule( Nzint_t );
      EGuiReply         LoadRealtimeKronoWithResultsFromAllRulesInKit( bool );
      void              AttachOwnKnobs( CController& );

      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      Cycle( time_t ) override;


/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   1:1 correspondence of a Kbase to a RuleKit, but only if a Kbase is to be built for that kit.  If
      so, the RuleKit uses its Kbase pointer to permanently build into the initially empty Kbase all the
      rule-hypo-evid (RHE) nodes it will have for life of runtime.

      Since tool may have a RuleKit without a corresponding Kbase (e.g, that kit has no diagnostic
      interactivity with a human User), a const ptr to non-const Kbase object is declared (vs. a ref)
      to allow for init with nullptr.
      
[2]   The RuleKit, rather than the CaseKit, builds nodes into its Kbase, because the RuleKit permanently
      holds pointers to all rules, which collectively hold permanent ptrs to all hypos, which
      collectively hold permanent ptrs to all evids.  However, once the Kbase is initally built, all
      interaction with it is through CCase objects.  Hypos and evids become associated to a particular
      CCase object by being "read in" from the Kbase when the CCase is created, using the affected
      rule's ruleId as a first index.

      This approach means that only INITIALLY are the tool expert diagnostics dependent upon the
      rule-hypo-evid node assignments defined at start of runtime [i.e., in the "tool_***.cpp"
      "CreateTool()" function ("script")].

[3]   Those CRule objects given a non-default EstmCostOfFail lambda field may require calls to
      field getters aboard an associated "subject" of a concrete subclass of ISubject.  Rather than
      pass a pointer to the subject through the CRule c-tor, instead the subject pointer is held
      by the CRuleKit class, and is "pulled" to the CRule object when it "joins" that particular kit.

[4]   depth of failedRuleRain, in cycles = (depth in triggers) / tpc.            

^^^^ END CLASS NOTES */

};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
