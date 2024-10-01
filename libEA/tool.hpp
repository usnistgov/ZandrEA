// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Header declaring abstract base class ATool, and concrete classes derived from it
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

/*
   Including headers here to avert nasty amount of fwd-declares having templated classes that involve
   function-pointer type parameters.  See no advantage to fwd-declare classes in the header to tool.cpp
*/

#include "state.hpp"
#include "chart.hpp"
#include "rainfall.hpp"

class CAgent;
class CCaseKit;
class CChartShewhart;
class CChartTracking;
class CClockPerPort;
class CController;
class CDomain;
class CEvid;
class CFeatureAnalog;
class CFeatureFact;
class CFormula;
class CHypo;
class CKnowBaseH5;
class CPointAnalog;
class CPointBinary;
class CPortOmni;
class CRule;
class CRuleKit;
class CSequence;
class CSeqTimeAxis;
class CSubj_ahu_sdvr;
class CSubj_vav_pihr;
class CView;


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CApplication needs an abstract interface to any Tool object

class ATool { 

   public:
    // Methods

     ~ATool( void );

   protected:

   // Declare handles to owned objects

      std::unique_ptr<CRuleKit>        u_RuleKit;

      ATool(   CDomain&,   // registration as observer requires ref passed non-const
               EDataLabel,
               ERealName,
               const CClockPerPort&,       // const = enforced as read-only
               CSequence&,
               CController&,
               CView&,
               CPortOmni& );
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare a concrete subclass for single-duct AHU tool

class CTool_ahu_ibal : public ATool { 

   public:
    // Methods

      CTool_ahu_ibal(   CDomain&,
                        EDataLabel,
                        ERealName,              // ahu own name
                        ERealName,              // antecedent chw plant name
                        ERealName,              // antecedent hw plant name
                        const CClockPerPort&,
                        CSequence&,
                        CController&,
                        CView&,
                        CPortOmni& );

      ~CTool_ahu_ibal( void );

   private:

      std::unique_ptr<CSubj_ahu_sdvr> u_Subject;

      std::unique_ptr<CPointAnalog> u_Psas;  // Input Channel 0, zero-based to match vector indices
      std::unique_ptr<CPointAnalog> u_Tao;   // Decl order must agree with order of data in push-in vec
      std::unique_ptr<CPointAnalog> u_Udm;
      std::unique_ptr<CPointAnalog> u_Tam;
      std::unique_ptr<CPointAnalog> u_Tar;
      std::unique_ptr<CPointAnalog> u_Uvc;
      std::unique_ptr<CPointAnalog> u_Tas;
      std::unique_ptr<CPointAnalog> u_TasSetpt;
      std::unique_ptr<CPointBinary> u_Bso;
      std::unique_ptr<CPointAnalog> u_Uvh;
      std::unique_ptr<CPointAnalog> u_Qas;    // Input Channel 10

      std::unique_ptr<CFormula>     u_absDifTaoTar;
      std::unique_ptr<CFormula>     u_fracOA;
      std::unique_ptr<CFormula>     u_maxTaoTar;
      std::unique_ptr<CFormula>     u_minTaoTar;

      std::unique_ptr<CChartShewhart> u_TasShew;
      std::unique_ptr<CChartShewhart> u_TasSetptShew;
      std::unique_ptr<CChartShewhart> u_UvcShew;
      std::unique_ptr<CChartShewhart> u_QasShew;

      std::unique_ptr<CChartTracking> u_Qas_VS_auto;
      std::unique_ptr<CChartTracking> u_Tas_VS_auto;
      std::unique_ptr<CChartTracking> u_Tas_VS_setpt;
      std::unique_ptr<CChartTracking> u_Uvc_VS_auto;

      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TasSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TasSetptSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_UvcSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_QasSteady;

      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_QasHunting;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_UvcHunting;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TasTrackingHigh;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TasTrackingLow;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TasRising;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TasFalling;
//======================================================================================================/ 
// CFactRelatingLeftToParam objects relate a "left" analog value to a parameter on "right", w/ bool result
// Specializations based upon left value greater than (GT), less than (LT), or equal to (EQ) the param
//------------------------------ 
// EQ specializations

      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_PsasZero;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_QasZero;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_UdmFullOA;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_UvcShut;
      std::unique_ptr< CFactRelatingLeftToParam<CFormula, PtrRainGetr_t, Left_EQ_Right> >  u_fracOAatMin;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_GT_Right> >  u_Tam_GT_frzStat;
      std::unique_ptr< CFactRelatingLeftToParam<CFormula, PtrRainGetr_t, Left_GTE_Right> > u_absDifTaoTar_GTE_10F;


//======================================================================================================/ 
// CFactRelatingLeftToRight objects relate two ("left" and "right") analog values, with bool result
// Specializations based upon left value greater than (GT), less than (LT), or equal to (EQ) the right value
//------------------------------ 
// EQ specializations

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_EQ_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tas_EQ_TasSetpt;


//------------------------------ 
// GT specializations

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_GT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tam_GT_TasSetpt;

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_GTE_Right, CFormula, PtrRainGetr_t>
         >  u_Tam_GTE_minTaoTar;

//------------------------------ 
// LT specializations

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_LT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tao_LT_Tar;

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_LT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tao_LT_TasSetpt;

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_LT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tas_LT_Tam;

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_LTE_Right, CFormula, PtrRainGetr_t>
         >  u_Tam_LTE_maxTaoTar;

//======================================================================================================/ 
// Declare direct Facts

      std::unique_ptr<CFactFromPoint> u_sysOcc;

//======================================================================================================/ 
// Declare sustained Facts

      std::unique_ptr<CFactSustained> u_sysOccSus;

//======================================================================================================/
// Declare facts from facts

      std::unique_ptr<CFactFromFacts>  u_unitOn;
      std::unique_ptr<CFactFromFacts>  u_inputsSteady;
      std::unique_ptr<CFactFromFacts>  u_outputLevel;
      std::unique_ptr<CFactFromFacts>  u_chwCoolingAir;
      std::unique_ptr<CFactFromFacts>  u_chwNeeded;
      std::unique_ptr<CFactFromFacts>  u_econActive;
      std::unique_ptr<CFactFromFacts>  u_econExpected;

//======================================================================================================/
// Declare "rule" objects

      std::unique_ptr<CRule>  u_Rule1;
      std::unique_ptr<CRule>  u_Rule2;
      std::unique_ptr<CRule>  u_Rule3;
      std::unique_ptr<CRule>  u_Rule4;
      std::unique_ptr<CRule>  u_Rule5;
      std::unique_ptr<CRule>  u_Rule6;
      std::unique_ptr<CRule>  u_Rule7;
      std::unique_ptr<CRule>  u_Rule8;
      std::unique_ptr<CRule>  u_Rule9;
      std::unique_ptr<CRule>  u_Rule10;

//======================================================================================================/
// Declare objects implementing knowledge-base for VAV box
//----------------------------------------------------------------------------------------
// Declare "Hypo 1" through "Hypo 15"

      std::unique_ptr<CHypo>   u_Hypo1;
      std::unique_ptr<CHypo>   u_Hypo2;
      std::unique_ptr<CHypo>   u_Hypo3;
      std::unique_ptr<CHypo>   u_Hypo4;
      std::unique_ptr<CHypo>   u_Hypo5;
      std::unique_ptr<CHypo>   u_Hypo6;
      std::unique_ptr<CHypo>   u_Hypo7;
      std::unique_ptr<CHypo>   u_Hypo8;
      std::unique_ptr<CHypo>   u_Hypo9;
      std::unique_ptr<CHypo>   u_Hypo10;
      std::unique_ptr<CHypo>   u_Hypo11;
      std::unique_ptr<CHypo>   u_Hypo12;
      std::unique_ptr<CHypo>   u_Hypo13;
      std::unique_ptr<CHypo>   u_Hypo14;
      std::unique_ptr<CHypo>   u_Hypo15;
      std::unique_ptr<CHypo>   u_Hypo16;
      std::unique_ptr<CHypo>   u_Hypo17;
      std::unique_ptr<CHypo>   u_Hypo18;
      std::unique_ptr<CHypo>   u_Hypo19;

//----------------------------------------------------------------------------------------
// Declare "Evid 1" through "Evid 14"

      std::unique_ptr<CEvid>   u_Evid1;
      std::unique_ptr<CEvid>   u_Evid2;
      std::unique_ptr<CEvid>   u_Evid3;
      std::unique_ptr<CEvid>   u_Evid4;
      std::unique_ptr<CEvid>   u_Evid5;
      std::unique_ptr<CEvid>   u_Evid6;
      std::unique_ptr<CEvid>   u_Evid7;
      std::unique_ptr<CEvid>   u_Evid8;
      std::unique_ptr<CEvid>   u_Evid9;
      std::unique_ptr<CEvid>   u_Evid10;
      std::unique_ptr<CEvid>   u_Evid11;
      std::unique_ptr<CEvid>   u_Evid12;
      std::unique_ptr<CEvid>   u_Evid13;
      std::unique_ptr<CEvid>   u_Evid14;
      std::unique_ptr<CEvid>   u_Evid15;
      std::unique_ptr<CEvid>   u_Evid16;
      std::unique_ptr<CEvid>   u_Evid17;
      std::unique_ptr<CEvid>   u_Evid18;
      std::unique_ptr<CEvid>   u_Evid19;

//======================================================================================================/
// Declare GUI Features

      std::unique_ptr<CFeatureFact>    u_GuiFeat01;
      std::unique_ptr<CFeatureFact>    u_GuiFeat02;
      std::unique_ptr<CFeatureAnalog>  u_GuiFeat03;
      std::unique_ptr<CFeatureFact>    u_GuiFeat04;
      std::unique_ptr<CFeatureFact>    u_GuiFeat05;
      std::unique_ptr<CFeatureFact>    u_GuiFeat06;
      std::unique_ptr<CFeatureFact>    u_GuiFeat07;
      std::unique_ptr<CFeatureFact>    u_GuiFeat08;
      std::unique_ptr<CFeatureFact>    u_GuiFeat09;
 
};

// End of CTool_Ahu_sdvr declaration
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare a concrete subclass for VAV box tool

class CTool_vav_ibal : public ATool { 

   public:
    // Methods

      CTool_vav_ibal( CDomain&,
                      EDataLabel,
                      ERealName,     // vav own name (real-world)
                      ERealName,     // antecedent ahu name
                      ERealName,     // antecedent hw plant name
                      const CClockPerPort&,
                      CSequence&,
                      CController&,
                      CView&,
                      CPortOmni& );

      ~CTool_vav_ibal( void );

   private:

      std::unique_ptr<CSubj_vav_pihr> u_Subject;

      std::unique_ptr<CPointAnalog> u_Psai;     // Input Channel 0, zero-based to match vector indices
      std::unique_ptr<CPointAnalog> u_Tai;      // *** TBD that AHU points passed from an AHU object ***
      std::unique_ptr<CPointAnalog> u_Tad;
      std::unique_ptr<CPointAnalog> u_Taz;
      std::unique_ptr<CPointAnalog> u_TazSetptHtg;
      std::unique_ptr<CPointAnalog> u_TazSetptClg;
      std::unique_ptr<CPointAnalog> u_Uvh;
      std::unique_ptr<CPointAnalog> u_Udd;
      std::unique_ptr<CPointAnalog> u_Qad;
      std::unique_ptr<CPointAnalog> u_QadSetpt;
      std::unique_ptr<CPointBinary> u_Bzo;     // Input Channel 10

      std::unique_ptr<CChartShewhart> u_PsaiShew;
      std::unique_ptr<CChartShewhart> u_QadShew;
      std::unique_ptr<CChartShewhart> u_QadSetptShew;
      std::unique_ptr<CChartShewhart> u_TadShew;
      std::unique_ptr<CChartShewhart> u_TaiShew;
      std::unique_ptr<CChartShewhart> u_TazShew;
      std::unique_ptr<CChartShewhart> u_TazSetptClgShew;
      std::unique_ptr<CChartShewhart> u_TazSetptHtgShew;
      std::unique_ptr<CChartShewhart> u_UddShew; 
      std::unique_ptr<CChartShewhart> u_UvhShew;

      std::unique_ptr<CChartTracking> u_Qad_VS_setpt;
      std::unique_ptr<CChartTracking> u_Tad_VS_auto;
      std::unique_ptr<CChartTracking> u_Taz_VS_setptClg;
      std::unique_ptr<CChartTracking> u_Taz_VS_setptHtg;

      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_PsaiSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_QadSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_QadSetptSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TadSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TaiSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TazSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TazSetptClgSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_TazSetptHtgSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_UddSteady;
      std::unique_ptr< CFactFromChart<CChartShewhart, PtrShewGetr_t> > u_UvhSteady;

      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_QadHunting;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_QadTrackingHigh;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_QadTrackingLow;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TadHunting;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TazTrackingHigh;
      std::unique_ptr< CFactFromChart<CChartTracking, PtrTrakGetr_t> > u_TazTrackingLow;

//======================================================================================================/ 
// CFactRelatingLeftToParam objects relate a "left" analog value to a parameter on "right", w/ bool result
// Specializations based upon left value greater than (GT), less than (LT), or equal to (EQ) the param
//------------------------------ 
// EQ specializations

      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_QadZero;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_UddFull;
      std::unique_ptr< CFactRelatingLeftToParam<CPointAnalog, PtrRainGetr_t, Left_EQ_Right> >  u_UvhShut;

//======================================================================================================/ 
// CFactRelatingLeftToRight objects relate two ("left" and "right") analog values, with bool result
// Specializations based upon left value greater than (GT), less than (LT), or equal to (EQ) the right value
//------------------------------ 
// GT specializations

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_GT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tad_GT_Tai;

      std::unique_ptr<
         CFactRelatingLeftToRight< CPointAnalog, PtrRainGetr_t, Left_GT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tad_GT_Taz;

      std::unique_ptr<
         CFactRelatingLeftToRight< CPointAnalog, PtrRainGetr_t, Left_GT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Taz_GT_setptClg;

//------------------------------ 
// LT specializations

      std::unique_ptr<
         CFactRelatingLeftToRight<CPointAnalog, PtrRainGetr_t, Left_LT_Right, CPointAnalog, PtrRainGetr_t>
         >  u_Tad_LT_Taz;

      std::unique_ptr<
         CFactRelatingLeftToRight< CPointAnalog, PtrRainGetr_t, Left_LT_Right, CPointAnalog, PtrRainGetr_t>
      >  u_Taz_LT_setptHtg;

//======================================================================================================/ 
// Declare direct Facts

      std::unique_ptr<CFactFromPoint> u_zoneOccupied;

//======================================================================================================/ 
// Declare facts from antecedent subjects

      std::unique_ptr<CFactFromAntecedentSubject>  u_ahuOutputOkay;      

//======================================================================================================/
// Declare facts from facts

      std::unique_ptr<CFactFromFacts>  u_unitCoolingZone;
      std::unique_ptr<CFactFromFacts>  u_unitReheating;
      std::unique_ptr<CFactFromFacts>  u_inputsSteady;
      std::unique_ptr<CFactFromFacts>  u_TazInBand;

//======================================================================================================/
// Declare "rule" objects

      std::unique_ptr<CRule>  u_Rule1;
      std::unique_ptr<CRule>  u_Rule2;
      std::unique_ptr<CRule>  u_Rule3;
      std::unique_ptr<CRule>  u_Rule4;
      std::unique_ptr<CRule>  u_Rule5;
      std::unique_ptr<CRule>  u_Rule6;
      std::unique_ptr<CRule>  u_Rule7;
      std::unique_ptr<CRule>  u_Rule8;
      std::unique_ptr<CRule>  u_Rule9;
      std::unique_ptr<CRule>  u_Rule10;

//======================================================================================================/
// Declare objects implementing knowledge-base for VAV box
//----------------------------------------------------------------------------------------
// Declare "Hypo 1" through "Hypo 15"

      std::unique_ptr<CHypo>   u_Hypo1;
      std::unique_ptr<CHypo>   u_Hypo2;
      std::unique_ptr<CHypo>   u_Hypo3;
      std::unique_ptr<CHypo>   u_Hypo4;
      std::unique_ptr<CHypo>   u_Hypo5;
      std::unique_ptr<CHypo>   u_Hypo6;
      std::unique_ptr<CHypo>   u_Hypo7;
      std::unique_ptr<CHypo>   u_Hypo8;
      std::unique_ptr<CHypo>   u_Hypo9;
      std::unique_ptr<CHypo>   u_Hypo10;
      std::unique_ptr<CHypo>   u_Hypo11;
      std::unique_ptr<CHypo>   u_Hypo12;
      std::unique_ptr<CHypo>   u_Hypo13;
      std::unique_ptr<CHypo>   u_Hypo14;
      std::unique_ptr<CHypo>   u_Hypo15;

//----------------------------------------------------------------------------------------
// Declare "Evid 1" through "Evid 14"

      std::unique_ptr<CEvid>   u_Evid1;
      std::unique_ptr<CEvid>   u_Evid2;
      std::unique_ptr<CEvid>   u_Evid3;
      std::unique_ptr<CEvid>   u_Evid4;
      std::unique_ptr<CEvid>   u_Evid5;
      std::unique_ptr<CEvid>   u_Evid6;
      std::unique_ptr<CEvid>   u_Evid7;
      std::unique_ptr<CEvid>   u_Evid8;
      std::unique_ptr<CEvid>   u_Evid9;
      std::unique_ptr<CEvid>   u_Evid10;
      std::unique_ptr<CEvid>   u_Evid11;
      std::unique_ptr<CEvid>   u_Evid12;
      std::unique_ptr<CEvid>   u_Evid13;
      std::unique_ptr<CEvid>   u_Evid14;
      std::unique_ptr<CEvid>   u_Evid15;

//======================================================================================================/
// Declare GUI Features

      std::unique_ptr<CFeatureFact>   u_GuiFeat01;
      std::unique_ptr<CFeatureFact>   u_GuiFeat02;
      std::unique_ptr<CFeatureFact>   u_GuiFeat03;
      std::unique_ptr<CFeatureFact>   u_GuiFeat04;
      std::unique_ptr<CFeatureFact>   u_GuiFeat05;
      std::unique_ptr<CFeatureFact>   u_GuiFeat06;
      std::unique_ptr<CFeatureFact>   u_GuiFeat07;
      std::unique_ptr<CFeatureAnalog>  u_GuiFeat08;
      std::unique_ptr<CFeatureAnalog>  u_GuiFeat09;
      std::unique_ptr<CFeatureAnalog>  u_GuiFeat10;

//======================================================================================================/
// End of CTool_VavBox declaration
 
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete class for an EA application

class CApplication {

   public:

      CApplication( void );

      ~CApplication( void );

   private:

      std::unique_ptr<CDomain>                        u_Domain;
      std::unique_ptr<CClockPerPort>                  u_Clock;
      std::unique_ptr<CAgent>                         u_Agent;
      std::unique_ptr<CController>                    u_Ctrlr;
      std::unique_ptr<CSequence>                      u_Seq0;
      std::unique_ptr<CView>                          u_View;
      std::unique_ptr<CPortOmni>                      u_OmniPort;
      std::vector< std::unique_ptr<ATool> >           u_EachToolInApp;

};


#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ