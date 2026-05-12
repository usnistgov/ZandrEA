//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* Source code file to an "EA" part of the ZandrEA (tm) project at: https://github.com/usnistgov/ZandrEA
This file last edited in base repo by: DAV, U.S. National Institute of Standards and Technology (NIST).
As a Work of the United States Government, this file is not subject to copyright within the United
States. For other countries, Copyright 2025-2026 National Institute of Standards and Technology.
For countries other than the United States, this file is licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy
of the License at: https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License. */
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* File summary:
   Implements CPortOmni 
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

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
