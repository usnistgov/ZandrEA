// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements CPortOmni 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX


#include "portOmni.hpp"
#include "mvc_ctrlr.hpp"
#include "mvc_view.hpp"


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implement class methods

CPortOmni::CPortOmni(   CController& arg0,
                        CView& arg1 )
                        :  IExportOmni(),
                           CtrlrRef (arg0),
                           ViewRef (arg1) {

   SExportedHandles::SetPortPointer( this );
}


CPortOmni::~CPortOmni( void ) { }

//=====================================================================================================/
// Private methods


//=====================================================================================================/
// Public methods

EGuiReply CPortOmni::PrepareApplicationForShutdown( void ) {

   return CtrlrRef.PrepareApplicationForShutdown();
}


EGuiReply CPortOmni::SetTimeStampInDomain( std::tm tmStruct ) {

   return CtrlrRef.SetTimeStampInDomain( tmStruct );
}


EGuiReply CPortOmni::SingleStepDomainOnTimeAndInputs( void ) {

   return CtrlrRef.SingleStepModelOnTimeAndInputs();
}


std::vector<EPointName>
CPortOmni::SayInputPointNameOrderExpectedBySubject( NGuiKey subjectGuiKey ) const {

   return CtrlrRef.SayInputPointNameOrderExpectedBySubject( subjectGuiKey );
}


EGuiReply  CPortOmni::SetCoincidentInputsForSubject( const std::vector<GuiFpn_t>& dataVectorRef,
                                                     NGuiKey subjectGuiKey ) {

   return CtrlrRef.ReadInDataForSubject( dataVectorRef, subjectGuiKey );
}


GuiPackKnob_t CPortOmni::GetInfoFromKnob( NGuiKey knobKey ) const {

   return CtrlrRef.GetGuiPackFromKnob( knobKey );
}


EGuiReply CPortOmni::SetKnobToValue( NGuiKey knobKey, GuiFpn_t value ) {

   return CtrlrRef.SetKnobToValue( knobKey, value );
}


std::string CPortOmni::SayTextIdentifyingKnob( NGuiKey knobKey ) const {

   return CtrlrRef.SayTextIdentifyingKnob( knobKey );
}


//===============================


GuiPackDomain_t CPortOmni::SayInfoFromDomain( void ) const {

   return ViewRef.SayGuiPackFromDomain();
}


std::queue<std::string>  CPortOmni::SayNewAlertsFifoFromDomainThenClear( void ) {

   return ViewRef.SayNewAlertsFifoFromDomainThenClear();
}


GuiPackSubjectBasic_t CPortOmni::SayInfoFromSubject( NGuiKey subjectGuiKey ) const {

   return ViewRef.SayGuiPackFromSubject( subjectGuiKey );
}


GuiPackSubjectCases_t CPortOmni::SayCurrentCasesFromSubject( NGuiKey subjectGuiKey ) const {

   return ViewRef.SayCurrentCasesFromSubject( subjectGuiKey );
}


std::string CPortOmni::SayTextIdentifyingSubject( NGuiKey subjectKey ) const {

   return ViewRef.SayTextIdentifyingSubject( subjectKey );
}


GuiPackRuleKitFull_t CPortOmni::SayFullInfoFromRuleKit( NGuiKey kitDisplayGuiKey ) const {

   return ViewRef.SayFullGuiPackFromRuleKitDisplay( kitDisplayGuiKey );
}


GuiPackRuleKitDyna_t CPortOmni::SayDynamicInfoFromRuleKit( NGuiKey kitDisplayGuiKey ) const {

   return ViewRef.SayDynamicGuiPackFromRuleKitDisplay( kitDisplayGuiKey );
}


std::string CPortOmni::SayTextIdentifyingRuleKit( NGuiKey kitKey ) const {

   return ViewRef.SayTextIdentifyingRuleKit( kitKey );
}


GuiPackFeatureFull_t CPortOmni::SayFullInfoFromFeature( NGuiKey featureGuiKey ) const {

   return ViewRef.SayFullGuiPackFromFeature( featureGuiKey );
}


GuiPackFeatureDyna_t CPortOmni::SayDynamicInfoFromFeature( NGuiKey featureGuiKey ) const {

   return ViewRef.SayDynamicGuiPackFromFeature( featureGuiKey );
}


GuiPackCaseFull_t  CPortOmni::SayFullInfoFromCase( NGuiKey caseGuiKey ) const {

   // Expecting RVO to elide all copying between CCase and external GUI/Client

   return ViewRef.SayFullGuiPackFromCase( caseGuiKey );
}


GuiPackCaseDyna_t  CPortOmni::SayDynamicInfoFromCase( NGuiKey caseGuiKey ) const {

   // Expecting RVO to elide all copying between CCase and external GUI/Client

   return ViewRef.SayDynamicGuiPackFromCase( caseGuiKey );
}


EGuiReply CPortOmni::AnswerCaseWithZeroBasedOptionIndex( NGuiKey caseGuiKey,
                                                         size_t iOption ) {

   return ViewRef.AnswerCaseWithOptionIndex( caseGuiKey, iOption );
}


GuiPackKronoFull_t CPortOmni::SayFullInfoFromKrono( NGuiKey kronoGuiKey ) const {

   return ViewRef.SayFullGuiPackFromKrono( kronoGuiKey );
}


GuiPackKronoDyna_t CPortOmni::SayDynamicInfoFromKrono( NGuiKey kronoGuiKey ) const { 

   return ViewRef.SayDynamicGuiPackFromKrono( kronoGuiKey );
}


GuiPackPane_t CPortOmni::SayInfoFromPane( NGuiKey paneGuiKey ) const {

   return ViewRef.SayGuiPackFromPane( paneGuiKey );
}


GuiPackTraceFull_t CPortOmni::SayFullInfoFromTraceInKrono(  NGuiKey traceGuiKey,
                                                            NGuiKey kronoGuiKey ) const {

   return ViewRef.SayFullGuiPackFromTrace( traceGuiKey );
}


GuiPackTraceDyna_t CPortOmni::SayDynamicInfoFromTraceInKrono(  NGuiKey traceGuiKey,
                                                               NGuiKey kronoGuiKey ) const {

   return ViewRef.SayDynamicGuiPackFromTrace( traceGuiKey );
}


GuiPackHistogram_t CPortOmni::SayInfoFromHistogram( NGuiKey histogramGuiKey ) const {

   return ViewRef.SayGuiPackFromHistogram( histogramGuiKey );
}


std::string CPortOmni::SayTextIdentifyingHistogram( NGuiKey histoKey ) const {

   return ViewRef.SayTextIdentifyingHistogram( histoKey );
}


EGuiReply CPortOmni::SetModeOfHistogramToZeroBasedOptionIndex( NGuiKey histogramGuiKey,
                                                               size_t iOption ) {

   return ViewRef.SetModeOfHistogramToOptionIndex( histogramGuiKey, iOption );
}


EGuiReply CPortOmni::SetSpanOfHistogramToZeroBasedOptionIndex(   NGuiKey histogramGuiKey,
                                                               size_t iOption ) {

   return ViewRef.SetSpanOfHistogramToOptionIndex( histogramGuiKey, iOption );
}


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
