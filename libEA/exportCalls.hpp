// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare "Runtime API" between EA front end (host clock, data acquisition, GUI) and EA back end
   (AFDD engine). The EA front end calls these API methods, the EA back end implements them.    
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef EXPORTCALLS_HPP
#define EXPORTCALLS_HPP

#include "exportTypes.hpp"


//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX/
/*
   Getter starting with "Get***()" is complemented by a setter "Set***()" for that same API field,
   getter starting with "Say***()" has no matching setter (i.e., API field is read-only at GUI)
*/


class IExportOmni {  

   public:

      virtual                         ~IExportOmni( void ) { /* empty */ };

      virtual EGuiReply                PrepareApplicationForShutdown( void ) = 0;

      virtual EGuiReply                SetTimeStampInDomain( std::tm ) = 0;

      virtual EGuiReply                SingleStepDomainOnTimeAndInputs( void ) = 0;

      virtual std::vector<EPointName>  SayInputPointNameOrderExpectedBySubject( NGuiKey ) const = 0;

      virtual EGuiReply                SetCoincidentInputsForSubject(   const std::vector<GuiFpn_t>&,
                                                                        NGuiKey ) = 0;

      virtual GuiPackKnob_t            GetInfoFromKnob( NGuiKey ) const = 0;
      virtual EGuiReply                SetKnobToValue( NGuiKey, GuiFpn_t ) = 0;
      virtual std::string              SayTextIdentifyingKnob( NGuiKey ) const = 0;

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

      virtual GuiPackDomain_t          SayInfoFromDomain( void ) const = 0;
      virtual std::queue<std::string>  SayNewAlertsFifoFromDomainThenClear( void ) = 0;

      virtual GuiPackSubjectBasic_t    SayInfoFromSubject( NGuiKey ) const = 0;
      virtual GuiPackSubjectCases_t    SayCurrentCasesFromSubject( NGuiKey ) const = 0;
      virtual std::string              SayTextIdentifyingSubject( NGuiKey ) const = 0;

      virtual GuiPackRuleKitFull_t     SayFullInfoFromRuleKit( NGuiKey ) const = 0;
      virtual GuiPackRuleKitDyna_t     SayDynamicInfoFromRuleKit( NGuiKey ) const = 0;
      virtual std::string              SayTextIdentifyingRuleKit( NGuiKey ) const = 0;
     
      virtual GuiPackFeatureFull_t     SayFullInfoFromFeature( NGuiKey ) const = 0;
      virtual GuiPackFeatureDyna_t     SayDynamicInfoFromFeature( NGuiKey ) const = 0;

      virtual GuiPackCaseFull_t        SayFullInfoFromCase( NGuiKey ) const = 0;
      virtual GuiPackCaseDyna_t        SayDynamicInfoFromCase( NGuiKey ) const = 0;
      virtual EGuiReply                AnswerCaseWithZeroBasedOptionIndex( NGuiKey, size_t ) = 0;

      virtual GuiPackKronoFull_t       SayFullInfoFromKrono( NGuiKey ) const = 0;
      virtual GuiPackKronoDyna_t       SayDynamicInfoFromKrono( NGuiKey) const = 0;
 
      virtual GuiPackPane_t            SayInfoFromPane( NGuiKey ) const = 0;
 
      virtual GuiPackTraceFull_t       SayFullInfoFromTraceInKrono(  NGuiKey, NGuiKey ) const = 0;
      virtual GuiPackTraceDyna_t       SayDynamicInfoFromTraceInKrono(  NGuiKey, NGuiKey ) const = 0;

      virtual GuiPackHistogram_t       SayInfoFromHistogram( NGuiKey ) const = 0;
      virtual std::string              SayTextIdentifyingHistogram( NGuiKey ) const = 0;
      virtual EGuiReply                SetModeOfHistogramToZeroBasedOptionIndex( NGuiKey, size_t ) = 0;
      virtual EGuiReply                SetSpanOfHistogramToZeroBasedOptionIndex( NGuiKey, size_t ) = 0;
};


struct SExportedHandles {

   public:

      static IExportOmni*  GetPortPointer( void );

      static void          SetPortPointer( IExportOmni* );

   private:

      static IExportOmni*  p_Port;

};



#endif


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   

--------------------------------------------------------------------------------
XXX END FILE NOTES */

/* BONEYARD        XXXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX



--------------------------------------------------------------------------------
XXX END BONEYARD */


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
