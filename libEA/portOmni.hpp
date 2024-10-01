// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares the concrete class CPortOmni for an EA Application's  "Port object". 
   A CController object ultimately implements the "Controller" methods of the EA runtime API.
   A CView object ultimately implements the "View" methods of the EA runtime API.
   Since the API is all pure virtual methods per parent class IExportOmni in file exportCalls.hpp,
   both implementations first go through a CPortOmni object. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef OMNIPORT_HPP
#define OMNIPORT_HPP


#include "customTypes.hpp"
#include "exportCalls.hpp"

#include <functional>

// fwd declares

class CController;
class CView;

//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX/

class CPortOmni : public IExportOmni {  

   public:

      CPortOmni(  CController&,
                  CView& );

      ~CPortOmni( void );

//======================================================================================================/
// Runtime API methods implemented through CController

      virtual EGuiReply                PrepareApplicationForShutdown( void );

      virtual EGuiReply                SetTimeStampInDomain( std::tm ) override;

      virtual EGuiReply                SingleStepDomainOnTimeAndInputs( void ) override;

      virtual std::vector<EPointName> SayInputPointNameOrderExpectedBySubject(NGuiKey) const override;

      virtual EGuiReply                SetCoincidentInputsForSubject(   const std::vector<GuiFpn_t>&,
                                                                        NGuiKey ) override;

      virtual GuiPackKnob_t            GetInfoFromKnob( NGuiKey ) const override;
      virtual EGuiReply                SetKnobToValue( NGuiKey, GuiFpn_t ) override;
      virtual std::string              SayTextIdentifyingKnob( NGuiKey ) const override;

//======================================================================================================/
// Runtime API methods implemented through CView

      virtual GuiPackDomain_t          SayInfoFromDomain( void ) const  override;
      virtual std::queue<std::string>  SayNewAlertsFifoFromDomainThenClear( void ) override;

      virtual GuiPackSubjectBasic_t    SayInfoFromSubject( NGuiKey ) const override;
      virtual GuiPackSubjectCases_t    SayCurrentCasesFromSubject( NGuiKey ) const override;
      virtual std::string              SayTextIdentifyingSubject( NGuiKey ) const override;

      virtual GuiPackRuleKitFull_t     SayFullInfoFromRuleKit( NGuiKey ) const override;
      virtual GuiPackRuleKitDyna_t     SayDynamicInfoFromRuleKit( NGuiKey ) const override;
      virtual std::string              SayTextIdentifyingRuleKit( NGuiKey ) const override;

      virtual GuiPackFeatureFull_t     SayFullInfoFromFeature( NGuiKey ) const override;
      virtual GuiPackFeatureDyna_t     SayDynamicInfoFromFeature( NGuiKey ) const override;

      virtual GuiPackCaseFull_t        SayFullInfoFromCase( NGuiKey ) const override;
      virtual GuiPackCaseDyna_t        SayDynamicInfoFromCase(NGuiKey ) const override;
      virtual EGuiReply                AnswerCaseWithZeroBasedOptionIndex( NGuiKey, size_t ) override;

      virtual GuiPackKronoFull_t       SayFullInfoFromKrono( NGuiKey ) const override;
      virtual GuiPackKronoDyna_t       SayDynamicInfoFromKrono( NGuiKey ) const override;
 
      virtual GuiPackPane_t            SayInfoFromPane( NGuiKey ) const override;

      virtual GuiPackTraceFull_t       SayFullInfoFromTraceInKrono( NGuiKey, NGuiKey ) const override;
      virtual GuiPackTraceDyna_t       SayDynamicInfoFromTraceInKrono(
                                          NGuiKey, NGuiKey ) const override;

      virtual GuiPackHistogram_t       SayInfoFromHistogram( NGuiKey ) const override;
      virtual std::string              SayTextIdentifyingHistogram( NGuiKey ) const override;
      virtual EGuiReply                SetModeOfHistogramToZeroBasedOptionIndex( NGuiKey,
                                                                                 size_t ) override;
      virtual EGuiReply                SetSpanOfHistogramToZeroBasedOptionIndex( NGuiKey,
                                                                                 size_t ) override;
  
       
//uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu/

   private:

      CController&                                    CtrlrRef;

      CView&                                          ViewRef;

};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
