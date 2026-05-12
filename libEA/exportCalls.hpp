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
   Declare "EA Runtime API" between EA front end (host clock, data acquisition, GUI) and EA back end
   (AFDD engine). The EA front end calls these API methods, the EA back end implements them.    
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef EXPORTCALLS_HPP
#define EXPORTCALLS_HPP

#include "exportTypes.hpp"

/*
   A getter starting with "Get***()" is complemented by a setter "Set***()" for that same API field,
   A getter starting with "Say***()" has no matching setter (i.e., API field is read-only at GUI)
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
