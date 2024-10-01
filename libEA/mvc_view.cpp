// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements the concrete class CView for an EA Application's  MVC View object. 
   A CView object ultimately implements the "View" methods of the EA runtime API.
   Since the API are all pure virtual methods, the implementation first goes through a CPortOmni object. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "mvc_view.hpp"

#include "mvc_ctrlr.hpp"      // Needed to register CView
#include "subject.hpp"
#include "case.hpp"
#include "viewParts.hpp"

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CView (interface) implementations


CView::CView(  CDomain& arg1,
               CController& arg2 )
               :  DomainRef (arg1),
                  p_CaseKits_byCaseKey(),
                  p_Features_byKey(),
                  p_Histograms_byKey(),
                  p_Kronos_byKey(),
                  p_Panes_byKey(),
                  p_RuleKitDisplays_byKey(),
                  p_Subjects_byKey(),
                  p_Traces_byKey(),
                  postedAlerts() {

   /* CController needs to "know" CView as Agent ("Model") tells View (via ctrlr) to update after Model
      is updated
   */
   arg1.Register( this );
   arg2.Register( this );

}


CView::~CView( void ) {

   // empty d-tor
}


//======================================================================================================/
// Private methods

EGuiReply CView::Regen( void ) {

   // Only Features and r-t Traces have dynamic fields needing runtime updates

   for ( const auto& iter : p_Features_byKey ) {

      (iter.second)->Update();
   }

/*
   for ( const auto& iter : p_RealtimeTracesByKey_subsetNeedingTrigger ) {

      (iter.second)->Trigger();
   }
*/
   return EGuiReply::OKAY_allDone;
}


//======================================================================================================/
// Public methods

EGuiReply CView::Update( void ) {     // Called by Ctrlr once notified by Model that it has updated

   return Regen();
}


GuiPackDomain_t CView::SayGuiPackFromDomain( void ) const { return DomainRef.SayGuiPack(); }


std::queue<std::string> CView::SayNewAlertsFifoFromDomainThenClear( void ) {

   return DomainRef.SayNewAlertsFifoThenClear();
}

/*
   Throughout the following getter methods, if called object is immortal, any bad Key given is taken
   as an error in GUI programming, not an error in User action, so letting method throw uncaught off
   of ".at()" call.  If called object is mortal (e.g., cases, krono content), Key is tested and
   method here returns FAIL_error reply if Key is bad. 
   Methods must use at() since method is const, and [] is non-const (i.e., it can write to a container)
*/

GuiPackSubjectBasic_t  CView::SayGuiPackFromSubject( NGuiKey subjectKey ) const {

   return p_Subjects_byKey.at( subjectKey )->SayBasicGuiPack();
}


GuiPackSubjectCases_t  CView::SayCurrentCasesFromSubject( NGuiKey subjectKey ) const {

   return p_Subjects_byKey.at( subjectKey )->SayCurrentCases(); 
}


std::string  CView::SayTextIdentifyingSubject( NGuiKey subjectKey ) const {

   return p_Subjects_byKey.at( subjectKey )->SayNameAsText(); 
}


GuiPackRuleKitFull_t  CView::SayFullGuiPackFromRuleKitDisplay( NGuiKey displayKey ) const {

   return p_RuleKitDisplays_byKey.at( displayKey )->SayFullGuiPack(); 
}


GuiPackRuleKitDyna_t  CView::SayDynamicGuiPackFromRuleKitDisplay( NGuiKey displayKey ) const {

   return p_RuleKitDisplays_byKey.at( displayKey )->SayDynamicGuiPack(); 
}


std::string  CView::SayTextIdentifyingRuleKit( NGuiKey displayKey ) const {

   return p_RuleKitDisplays_byKey.at( displayKey )->SayRuleKitCaption(); 
}


GuiPackFeatureFull_t  CView::SayFullGuiPackFromFeature( NGuiKey idGiven ) const {

   return p_Features_byKey.at( idGiven )->SayFullGuiPack();
}


GuiPackFeatureDyna_t  CView::SayDynamicGuiPackFromFeature( NGuiKey idGiven ) const {

   return p_Features_byKey.at( idGiven )->SayDynamicGuiPack();
}


GuiPackCaseFull_t  CView::SayFullGuiPackFromCase(  NGuiKey caseKey ) const {

   // Expecting RVO to elide all copying between CCase and external GUI/Client

   return ( p_CaseKits_byCaseKey.count(caseKey) == 0 ?
            SGuiPackCaseFull( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_CaseKits_byCaseKey.at(caseKey)->SayFullGuiPackFromCase( caseKey ) );
}


GuiPackCaseDyna_t  CView::SayDynamicGuiPackFromCase(  NGuiKey caseKey ) const {

   // Expecting RVO to elide all copying between CCase and external GUI/Client

   return ( p_CaseKits_byCaseKey.count(caseKey) == 0 ?
            SGuiPackCaseDyna( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_CaseKits_byCaseKey.at(caseKey)->SayDynamicGuiPackFromCase( caseKey ) );
}


EGuiReply CView::AnswerCaseWithOptionIndex(  NGuiKey caseKey, size_t iOption ) {

   return ( p_CaseKits_byCaseKey.count(caseKey) == 0 ?
            EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled :
            p_CaseKits_byCaseKey.at(caseKey)->AnswerCasePromptUsingOptionIndex( caseKey, iOption ) );
}


GuiPackKronoFull_t  CView::SayFullGuiPackFromKrono( NGuiKey kronoKey ) const {

   return ( p_Kronos_byKey.count(kronoKey) == 0 ?
            SGuiPackKronoFull( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_Kronos_byKey.at(kronoKey)->SayFullGuiPack() );
}


GuiPackKronoDyna_t  CView::SayDynamicGuiPackFromKrono( NGuiKey kronoKey ) const {

   return ( p_Kronos_byKey.count(kronoKey) == 0 ?
            SGuiPackKronoDyna( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_Kronos_byKey.at(kronoKey)->SayDynamicGuiPack() );
}

GuiPackPane_t  CView::SayGuiPackFromPane( NGuiKey paneKey ) const {

   return ( p_Panes_byKey.count(paneKey) == 0 ?
            SGuiPackPane( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_Panes_byKey.at(paneKey)->SayGuiPack() );
}


GuiPackTraceFull_t  CView::SayFullGuiPackFromTrace( NGuiKey traceKey ) const {

   return ( p_Traces_byKey.count(traceKey) == 0 ?
            SGuiPackTraceFull( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_Traces_byKey.at(traceKey)->SayFullGuiPack() );
}


GuiPackTraceDyna_t  CView::SayDynamicGuiPackFromTrace( NGuiKey traceKey ) const {

   return ( p_Traces_byKey.count(traceKey) == 0 ?
            SGuiPackTraceDyna( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
            p_Traces_byKey.at(traceKey)->SayDynamicGuiPack() );
}


GuiPackHistogram_t  CView::SayGuiPackFromHistogram( NGuiKey histoKey ) const {

   if ( p_Histograms_byKey.count(histoKey) == 0 ){
      return SGuiPackHistogram( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled );
   }

   return p_Histograms_byKey.at(histoKey)->SayGuiPack();
}


std::string  CView::SayTextIdentifyingHistogram( NGuiKey histoKey ) const {

   return p_Histograms_byKey.at( histoKey )->SayIdentifyingText(); 
}


EGuiReply CView::SetModeOfHistogramToOptionIndex(  NGuiKey histoKey, size_t iOption ) {

   if ( p_Histograms_byKey.count(histoKey) == 0 ){
      return EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled;
   }

   return p_Histograms_byKey.at(histoKey)->SetModeActiveToOptionIndex( iOption );
}


EGuiReply CView::SetSpanOfHistogramToOptionIndex(  NGuiKey histoKey, size_t iOption ) {

   return ( p_Histograms_byKey.count(histoKey) == 0 ?
            EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled :
            p_Histograms_byKey.at( histoKey )->SetSpanActiveToOptionIndex( iOption ) );
}



void CView::AddCaseToCaseKitLookup( NGuiKey caseKey, CCaseKit* p_CaseKit) {

   // map insert() nops upon finding key already in container.  Emplace() not advantageous enough here.

   p_CaseKits_byCaseKey.insert( std::pair<NGuiKey, CCaseKit*>(caseKey, p_CaseKit) );
   return;
}

void CView::RemoveCaseFromCaseKitLookup( NGuiKey caseKey ) {

   p_CaseKits_byCaseKey.erase( caseKey );
   return;
}

void CView::AddRuleKitDisplay( CDisplayRuleKit& arg ) {

   // map insert() nops upon finding key already in container.  Emplace() not advantageous enough here.

   p_RuleKitDisplays_byKey.insert(
      std::pair<NGuiKey, CDisplayRuleKit*>( arg.SayGuiKey(), &arg)
   );
   return;
}


void CView::AddFeature( AFeature& arg ) {

   p_Features_byKey.insert(
      std::pair<NGuiKey, AFeature*>( arg.SayGuiKey(), &arg)
   );
   return;
}


void CView::AddHistogram( AHistogram& arg ) {

   p_Histograms_byKey.insert(
      std::pair<NGuiKey, AHistogram*>( arg.SayGuiKey(), &arg )
   );
   return;
}


void CView::GainAccessTo( CKronoRealtime* const arg ) {

   p_Kronos_byKey.emplace(
      std::pair<NGuiKey, CKronoRealtime*>( arg->SayGuiKey(), arg )
   );
   return;
}


void CView::GainAccessTo( CKronoSnapshot* const arg ) {

   p_Kronos_byKey.emplace(
      std::pair<NGuiKey, CKronoSnapshot*>( arg->SayGuiKey(), arg )
   );
   return;
}

void CView::GainAccessTo( CPaneRealtime* const arg ) {

   p_Panes_byKey.emplace(
      std::pair<NGuiKey, CPaneRealtime*>( arg->SayGuiKey(), arg )
   );
   return;
}

void CView::GainAccessTo( CPaneSnapshot* const arg ) {

   p_Panes_byKey.emplace(
      std::pair<NGuiKey, CPaneSnapshot*>( arg->SayGuiKey(), arg )
   );
   return;
}

void CView::GainAccessTo( CTraceRealtime* const arg ) {

   p_Traces_byKey.emplace(
      std::pair<NGuiKey, CTraceRealtime*>( arg->SayGuiKey(), arg )
   );
   return;
}

void CView::GainAccessTo( CTraceSnapshot* const arg ) {

   p_Traces_byKey.emplace(
      std::pair<NGuiKey, CTraceSnapshot*>( arg->SayGuiKey(), arg )
   );
   return;
}

void CView::LoseAccessTo( CKronoRealtime* const ptr ) {

   p_Kronos_byKey.erase( ptr->SayGuiKey() );
   return;
}

void CView::LoseAccessTo( CKronoSnapshot* const ptr ) {

   p_Kronos_byKey.erase( ptr->SayGuiKey() );
   return;
}

void CView::LoseAccessTo( CPaneRealtime* const ptr ) {

   p_Panes_byKey.erase( ptr->SayGuiKey() );
   return;
}

void CView::LoseAccessTo( CPaneSnapshot* const ptr ) {

   p_Panes_byKey.erase( ptr->SayGuiKey() );
   return;
}

void CView::LoseAccessTo( CTraceRealtime* const ptr ) {

   p_Traces_byKey.erase( ptr->SayGuiKey() );
   return;
}

void CView::LoseAccessTo( CTraceSnapshot* const ptr ) {

   p_Traces_byKey.erase( ptr->SayGuiKey() );
   return;
}


void CView::AddSubject( ASubject& arg ) {

   p_Subjects_byKey.emplace(
      std::pair<NGuiKey, ASubject*>( arg.SayGuiKey(), &arg )
   );
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
