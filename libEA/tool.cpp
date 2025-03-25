// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implement abstract base class ATool, and concrete classes derived from it
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "tool.hpp"           // brings along state.hpp and stateParts.hpp

#include "agentTask.hpp"
#include "case.hpp"
#include "dataChannel.hpp"
#include "formula.hpp"
#include "knowBase.hpp"
#include "knowParts.hpp"

#include "mvc_ctrlr.hpp"
#include "mvc_view.hpp"
#include "portOmni.hpp"

#include "rule.hpp"
#include "seqElement.hpp"
#include "state.hpp"
#include "subject.hpp"
#include "taskClock.hpp"
#include "viewParts.hpp"
#include "mvc_model.hpp"

#include <utility>


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Tool Interface Implementation

ATool::ATool(  CDomain& domainRef,           // registration as observer requires ref passed non-const
               EDataLabel label,             // passes through direct to Subject c-tor; not CTool info
               ERealName name,               // passes through direct to Subject c-tor; not CTool info
               const CClockPerPort& clockRef,   // const = enforced as read-only
               CSequence& seq0Ref,           // pass-throughs from CApplication c-tor...
               CController& ctrlrRef,
               CView& viewRef,
               CPortOmni& portRef )
               :  u_RuleKit ( nullptr) {

   // Empty c-tor
}

ATool::~ATool( void ) { /* empty d-tor */ }


// End of Abstract Tool Definition
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Start of AHU (concrete) Tool Definition

CTool_ahu_ibal::CTool_ahu_ibal(  CDomain& domainRef,
                                 EDataLabel ahuLabel,
                                 ERealName ahuName,
                                 ERealName chwPlantName,
                                 ERealName hwPlantName,
                                 const CClockPerPort& clockRef,
                                 CSequence& seq0Ref,
                                 CController& ctrlrRef,
                                 CView& viewRef,
                                 CPortOmni& portRef )
                                 :  ATool(   domainRef,
                                             ahuLabel,
                                             ahuName, 
                                             clockRef,
                                             seq0Ref,
                                             ctrlrRef,
                                             viewRef,
                                             portRef
                                    ) {

//======================================================================================================/
// $$$ TBD to move these reassignments to ATool initialization list (i.e., learn smt ptr move-semantics)

u_Subject = std::make_unique<CSubj_ahu_sdvr>(
               domainRef,
               ahuLabel,
               ahuName,
               chwPlantName,
               hwPlantName,
               5000.0f,    // cfmAirRated
               22.0f,      // gpmChwRated
               25.0f,      // kwPreheatRated
               0.30f       // min fraction OA
);


u_RuleKit = std::make_unique<CRuleKit>(
               seq0Ref,
               *u_Subject,
               EDataLabel::RuleKit, 
               START_RULEKIT_SECSBETWEENTRAPS,
               ctrlrRef
);

//======================================================================================================/
// Point objects

   u_Psas =          std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_pressure_static_air_supply,
                                                      EDataUnit::PressureGage_iwg,
                                                      EDataRange::Analog_zeroTo3,
                                                      EPlotGroup::Free,
                                      // $$$ TBD to match enum below to actual BAS point list names $$$
                                                      EPointName::Pressure_static_air_supply,
                                                      ctrlrRef );

   u_Tao =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_outside,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::Free,
                                                      EPointName::Temperature_air_outside,
                                                      ctrlrRef );

   u_Udm =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_command_damper_mixing,
                                                      EDataUnit::Ratio_percent,
                                                      EDataRange::Analog_percent,
                                                      EPlotGroup::Free,
                                                      EPointName::Position_damper_mixingBox,
                                                      ctrlrRef );

   u_Tam =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_mixed,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::Free,
                                                      EPointName::Temperature_air_mixed,
                                                      ctrlrRef );
   
   u_Tar =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_return,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::Free,
                                                      EPointName::Temperature_air_return,
                                                      ctrlrRef );

   u_Uvc =            std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_command_valve_chw,
                                                      EDataUnit::Ratio_percent,
                                                      EDataRange::Analog_percent,
                                                      EPlotGroup::Free,
                                                      EPointName::Position_valve_chw,
                                                      ctrlrRef );

   u_Tas =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_supply,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::GroupA,
                                                      EPointName::Temperature_air_supply,
                                                      ctrlrRef );

   u_TasSetpt =      std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_supply_setpt,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::GroupA,
                                                      EPointName::Temperature_air_supply_setpt,
                                                      ctrlrRef );

   u_Bso =           std::make_unique<CPointBinary>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_binary_system_occupied,
                                                      EDataLabel::Fact_direct_Bso,
                                                      EPointName::Binary_systemOccupied,
                                                      ctrlrRef );

   u_Uvh =            std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_command_valve_hw,
                                                      EDataUnit::Ratio_percent,
                                                      EDataRange::Analog_percent,
                                                      EPlotGroup::Free,
                                                      EPointName::Position_valve_hw,
                                                      ctrlrRef );

   u_Qas =           std::make_unique<CPointAnalog>(   seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_flowVolume_air_supply,
                                                      EDataUnit::FlowVolume_cfm,
                                                      EDataRange::Analog_zeroTo3k,
                                                      EPlotGroup::Free,
                                                      EPointName::FlowRateVolume_air_ahu,
                                                      ctrlrRef );

//======================================================================================================/
// Formula objects

   u_absDifTaoTar =  std::make_unique<CFormula>(   seq0Ref,
                                                   *u_Subject,
                                                   EDataLabel::Formula_absDif_TarTao,
                                                   EDataUnit::TemperatureDiff_Fdeg,
                                                   EDataRange::Analog_zeroTo120,
                                                   EDataSuffix::None,
                                                   EPlotGroup::GroupA, // segregate; GUI can scale y-axis
                                                   std::vector<CPointAnalog*>(
                                                      { u_Tao.get(), u_Tar.get() }
                                                   ),
                                                   std::function<float(void)>( [&]() -> float {
                                                      return   std::abs(
                                                                  u_Tao->u_Rain->NowY() - u_Tar->u_Rain->NowY()
                                                               );
                                                      }
                                                   )
   );

   u_fracOA =  std::make_unique<CFormula>(   seq0Ref,
                                             *u_Subject,
                                             EDataLabel::Formula_fraction_OA_tempProxy,
                                             EDataUnit::None,
                                             EDataRange::Analog_zeroToOne,
                                             EDataSuffix::None,
                                             EPlotGroup::Free, // segregate; GUI can scale y-axis
                                             std::vector<CPointAnalog*>(
                                                { u_Tam.get(), u_Tao.get(), u_Tar.get() }
                                             ),
                                             std::function<float(void)>( [&]() -> float {
                                                return   (  ( u_Tam->u_Rain->NowY() - u_Tar->u_Rain->NowY() ) /
                                                            ( u_Tao->u_Rain->NowY() - u_Tar->u_Rain->NowY() )
                                                         );
                                                }
                                             )
   );

   u_maxTaoTar =  std::make_unique<CFormula>(   seq0Ref,
                                                *u_Subject,
                                                EDataLabel::Formula_maxTaoTar,
                                                EDataUnit::Temperature_degF,
                                                EDataRange::Analog_zeroTo120,
                                                EDataSuffix::None,
                                                EPlotGroup::Free,
                                                std::vector<CPointAnalog*>(
                                                   { u_Tao.get(), u_Tar.get() }
                                                ),
                                                std::function<float(void)>( [&]() -> float {
                                                   return   std::max( u_Tao->u_Rain->NowY(), u_Tar->u_Rain->NowY()
                                                            );
                                                   }
                                                )
   );

   u_minTaoTar =  std::make_unique<CFormula>(   seq0Ref,
                                                *u_Subject,
                                                EDataLabel::Formula_minTaoTar,
                                                EDataUnit::Temperature_degF,
                                                EDataRange::Analog_zeroTo120,
                                                EDataSuffix::None,
                                                EPlotGroup::Free,
                                                std::vector<CPointAnalog*>(
                                                   { u_Tao.get(), u_Tar.get() }
                                                ),
                                                std::function<float(void)>( [&]() -> float {
                                                   return   std::min( u_Tao->u_Rain->NowY(), u_Tar->u_Rain->NowY()
                                                            );
                                                   }
                                                )
   );

//======================================================================================================/
// Chart objects - Shewhart charts

   u_TasShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Tas.get(), ctrlrRef);

   u_TasSetptShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_TasSetpt.get(), ctrlrRef);

   u_UvcShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Uvc.get(), ctrlrRef);

   u_QasShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Qas.get(), ctrlrRef);

//======================================================================================================/
// Chart objects - tracking charts
// float halfband = (+/-),  float warn = (units-of-x)-minutes over a reset

   u_Qas_VS_auto = std::make_unique<CChartTracking>(  seq0Ref,
                                                      *u_Subject,
                                                      u_Qas.get(),
                                                      u_QasShew.get(),
                                                      INIT_MINDEFMAX_TRACKING_HALFBAND_CFM_0TO3K,
                                                      INIT_MINDEFMAX_TRACKING_WARN_CFM_0TO3K,
                                                      ctrlrRef );

   u_Tas_VS_auto =     std::make_unique<CChartTracking>( seq0Ref,
                                                         *u_Subject,
                                                         u_Tas.get(),
                                                         u_TasShew.get(),
                                                         INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120,
                                                         INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120,
                                                         ctrlrRef );


   u_Tas_VS_setpt =     std::make_unique<CChartTracking>( seq0Ref,
                                                         *u_Subject,
                                                         u_Tas.get(),
                                                         u_TasShew.get(),
                                                         u_TasSetpt.get(),
                                                         u_TasSetptShew.get(),
                                                         INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120,
                                                         INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120,
                                                         ctrlrRef );

   u_Uvc_VS_auto = std::make_unique<CChartTracking>(  seq0Ref,
                                                      *u_Subject,
                                                      u_Uvc.get(),
                                                      u_UvcShew.get(),
                                                      INIT_MINDEFMAX_TRACKING_HALFBAND_ANYPERCENT,
                                                      INIT_MINDEFMAX_TRACKING_WARN_ANYPERCENT,
                                                      ctrlrRef );

//======================================================================================================/
// Fact objects based on charts - Shewhart charts 

   u_TasSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasSteady,
                                                                           *u_TasShew,
                                                                           &CChartShewhart::IsSteady );

   u_TasSetptSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasSetptSteady,
                                                                           *u_TasSetptShew,
                                                                           &CChartShewhart::IsSteady );

   u_QasSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QasSteady,
                                                                           *u_QasShew,
                                                                           &CChartShewhart::IsSteady );

   u_UvcSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_UvcSteady,
                                                                           *u_UvcShew,
                                                                           &CChartShewhart::IsSteady );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Fact objects based on charts - tracking charts

   u_QasHunting =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QasHunting,
                                                                           *u_Qas_VS_auto,
                                                                           &CChartTracking::IsHunting );

   u_UvcHunting =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_UvcHunting,
                                                                           *u_Uvc_VS_auto,
                                                                           &CChartTracking::IsHunting );


   u_TasTrackingHigh =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasTrackingHigh,
                                                                           *u_Tas_VS_setpt,
                                                                           &CChartTracking::IsRising );

   u_TasTrackingLow =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasTrackingLow,
                                                                           *u_Tas_VS_setpt,
                                                                           &CChartTracking::IsFalling );

   u_TasRising =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasRising,
                                                                           *u_Tas_VS_auto,
                                                                           &CChartTracking::IsRising );

   u_TasFalling =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasFalling,
                                                                           *u_Tas_VS_auto,
                                                                           &CChartTracking::IsFalling );

//======================================================================================================/ 
// CFactRelatingLeftToParam

   u_PsasZero = std::make_unique<   CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_PsasZero,
                                    u_Psas.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_ZERO,
                                    INIT_MINDEFMAX_RELATE_HYSTER_IWG_0TO3,
                                    INIT_MINDEFMAX_RELATE_SLACK_IWG_0TO3,
                                    ctrlrRef );

   u_QasZero = std::make_unique< CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_QasZero,
                                    u_Qas.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_ZERO,
                                    INIT_MINDEFMAX_RELATE_HYSTER_CFM_0TO3K,
                                    INIT_MINDEFMAX_RELATE_SLACK_CFM_0TO3K,
                                    ctrlrRef );

   u_UdmFullOA = std::make_unique<  CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_UdmFullOA,
                                    u_Udm.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_PERCENT_FULL,   // Udm = 1.0 (100%) for full OA ***? chk OK***
                                    INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT,
                                    INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT,
                                    ctrlrRef );

   u_UvcShut = std::make_unique<  CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_UvcShut,
                                    u_Uvc.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_PERCENT_SHUT,
                                    INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT,
                                    INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT,
                                    ctrlrRef );

   u_fracOAatMin = std::make_unique<   CFactRelatingLeftToParam<
                                       CFormula,
                                       PtrRainGetr_t,
                                       Left_EQ_Right>
                                       >( seq0Ref,
                                          *u_Subject,
                                          EDataLabel::Fact_para_fracOA_EQ_min,
                                          u_fracOA.get(),
                                          &CRainAnalog::NowY,
                                          // $$$ TBD hvac params by r-t calls vs. hard code $$$
                                          INIT_HVACPARAM_AHU_OAFRAC_MIN,  // TBD that this be forumulated in r-t from current occupancy
                                          INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT,
                                          INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT,
                                          ctrlrRef );

   u_Tam_GT_frzStat = std::make_unique<   CFactRelatingLeftToParam<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_GT_Right>
                                          >( seq0Ref,
                                             *u_Subject,
                                             EDataLabel::Fact_para_Tam_GT_frzStat,
                                             u_Tam.get(),
                                             &CRainAnalog::NowY,
                                             INIT_HVACPARAM_AHU_FRZSTAT_DEGF,
                                             INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                             INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                             ctrlrRef );

   u_absDifTaoTar_GTE_10F = std::make_unique<   CFactRelatingLeftToParam<
                                                CFormula,
                                                PtrRainGetr_t,
                                                Left_GTE_Right>
                                                >( seq0Ref,
                                                   *u_Subject,
                                                   EDataLabel::Fact_para_absDifTarTao_GTE_10F,
                                                   u_absDifTaoTar.get(),
                                                   &CRainAnalog::NowY,
                                                   10.0f,
                                                   INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                                   INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_ZERO,
                                                   ctrlrRef );

//======================================================================================================/ 
// CFactRelatingLeftToRight
// EQ specializations

   u_Tas_EQ_TasSetpt = std::make_unique<  CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_EQ_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                          >( seq0Ref,
                                             *u_Subject,
                                             EDataLabel::Fact_data_Tas_EQ_setpt,
                                             u_Tas.get(),
                                             &CRainAnalog::NowY,
                                             u_TasSetpt.get(),
                                             &CRainAnalog::NowY,
                                             INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                             INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                             ctrlrRef );



//------------------------------ 
// GT specializations

   u_Tam_GT_TasSetpt = std::make_unique<  CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_GT_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                          >( seq0Ref,
                                             *u_Subject,
                                             EDataLabel::Fact_data_Tam_GT_TasSetpt,
                                             u_Tam.get(),
                                             &CRainAnalog::NowY,
                                             u_TasSetpt.get(),
                                             &CRainAnalog::NowY,
                                             INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                             INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                             ctrlrRef );

   u_Tam_GTE_minTaoTar = std::make_unique<   CFactRelatingLeftToRight<
                                             CPointAnalog,
                                             PtrRainGetr_t,
                                             Left_GTE_Right,
                                             CFormula,
                                             PtrRainGetr_t>
                                             >( seq0Ref,
                                                *u_Subject,
                                                EDataLabel::Fact_data_Tam_GTE_minTaoTar,
                                                u_Tam.get(),
                                                &CRainAnalog::NowY,
                                                u_minTaoTar.get(),
                                                &CRainAnalog::NowY,
                                                INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                                INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                                ctrlrRef );

//------------------------------ 
// LT specializations

   u_Tao_LT_Tar = std::make_unique< CFactRelatingLeftToRight<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_LT_Right,
                                    CPointAnalog,
                                    PtrRainGetr_t>
                                    >( seq0Ref,
                                       *u_Subject,
                                       EDataLabel::Fact_data_Tao_LT_Tar,
                                       u_Tao.get(),
                                       &CRainAnalog::NowY,
                                       u_Tar.get(),
                                       &CRainAnalog::NowY,
                                       INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                       INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                       ctrlrRef );


   u_Tao_LT_TasSetpt = std::make_unique<  CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_LT_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                          >( seq0Ref,
                                             *u_Subject,
                                             EDataLabel::Fact_data_Tao_LT_TasSetpt,
                                             u_Tao.get(),
                                             &CRainAnalog::NowY,
                                             u_TasSetpt.get(),
                                             &CRainAnalog::NowY,
                                             INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                             INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                             ctrlrRef );

   u_Tas_LT_Tam = std::make_unique< CFactRelatingLeftToRight<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_LT_Right,
                                    CPointAnalog,
                                    PtrRainGetr_t>
                                    >( seq0Ref,
                                       *u_Subject,
                                       EDataLabel::Fact_data_Tas_LT_Tam,
                                       u_Tas.get(),
                                       &CRainAnalog::NowY,
                                       u_Tam.get(),
                                       &CRainAnalog::NowY,
                                       INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                       INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                       ctrlrRef );

   u_Tam_LTE_maxTaoTar = std::make_unique<   CFactRelatingLeftToRight<
                                             CPointAnalog,
                                             PtrRainGetr_t,
                                             Left_LTE_Right,
                                             CFormula,
                                             PtrRainGetr_t>
                                             >( seq0Ref,
                                                *u_Subject,
                                                EDataLabel::Fact_data_Tam_LTE_maxTaoTar,
                                                u_Tam.get(),
                                                &CRainAnalog::NowY,
                                                u_maxTaoTar.get(),
                                                &CRainAnalog::NowY,
                                                INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                                INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                                ctrlrRef );

//======================================================================================================/
// Define direct Fact objects

   u_sysOcc = std::make_unique<CFactFromPoint>(
                  seq0Ref,
                  *u_Subject,
                  EDataLabel::Fact_direct_Bso,
                  *u_Bso);

//======================================================================================================/
// Define sustained Fact objects

   u_sysOccSus = std::make_unique<CFactSustained>(
                  seq0Ref,
                  *u_Subject,
                  EDataLabel::Fact_sustained_Bso,
                  *u_sysOcc,
                  true,
                  INIT_MINDEFMAX_FACTSUSTAINED_MINCYCLES,
                  ctrlrRef);

//======================================================================================================/
// Define CFactFromFacts objects

   std::vector<AFact*> operands(0);
   operands.reserve(6);
   std::function<bool(void)>LambdaToCopy( [&]() -> bool { return NaNBOOL; } );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// unitOn

   operands.clear();
   operands.push_back( u_PsasZero.get() );
   operands.push_back( u_QasZero.get() );

  LambdaToCopy =  (  [&]() -> bool {
                        return ( ( ! u_PsasZero->Now() ) &&
                                 ( ! u_QasZero->Now() ) );
                     } );

   u_unitOn =  std::make_unique<CFactFromFacts>(
                              seq0Ref,
                              *u_Subject,
                              EDataLabel::Fact_subj_ahu_unitOn,
                              operands,
                              LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// inputsSteady

   operands.clear();
   operands.push_back( u_TasSetptSteady.get() );

   LambdaToCopy = (  [&]() -> bool {
                        return ( u_TasSetptSteady->Now() );
                     } );

   u_inputsSteady =  std::make_unique<CFactFromFacts>(
                        seq0Ref,
                        *u_Subject,
                        EDataLabel::Fact_subj_ahu_inputsSteady,
                        operands,
                        LambdaToCopy );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// outputLevel

   operands.clear();
   operands.push_back( u_sysOccSus.get() );
//   operands.push_back( u_TasSteady.get() );
//   operands.push_back( u_TasRising.get() );
//   operands.push_back( u_TasFalling.get() );

   LambdaToCopy = (  [&]() -> bool {
                        return ( u_sysOccSus->Now()
//                                 ( u_TasSteady->Now() &&
//                                   ( ! u_TasRising->Now() ) &&
//                                   ( ! u_TasFalling->Now() )
//                                 )
                        );
                     } );

   u_outputLevel =  std::make_unique<CFactFromFacts>(
                        seq0Ref,
                        *u_Subject,
                        EDataLabel::Fact_subj_ahu_outputLevel,
                        operands,
                        LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// chwCoolingAir

   operands.clear();
   operands.push_back( u_unitOn.get() );
   operands.push_back( u_Tas_LT_Tam.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_unitOn->Now()  &&
                              u_Tas_LT_Tam->Now() );
                  } );

   u_chwCoolingAir = std::make_unique<CFactFromFacts>(
                        seq0Ref,
                        *u_Subject,
                        EDataLabel::Fact_subj_ahu_chwCoolingAir,
                        operands,
                        LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// chwNeeded

   operands.clear();
   operands.push_back( u_unitOn.get() );
   operands.push_back( u_Tam_GT_TasSetpt.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_unitOn->Now()  &&
                              u_Tam_GT_TasSetpt->Now() );
                  } );

   u_chwNeeded =  std::make_unique<CFactFromFacts>(
                     seq0Ref,
                     *u_Subject,
                     EDataLabel::Fact_subj_ahu_chwNeeded,
                     operands,
                     LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// econActive

   operands.clear();
   operands.push_back( u_unitOn.get() );
   operands.push_back( u_fracOAatMin.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_unitOn->Now()  &&
                              (! u_fracOAatMin->Now()) );
                  } );

   u_econActive = std::make_unique<CFactFromFacts>(
                     seq0Ref,
                     *u_Subject,
                     EDataLabel::Fact_subj_ahu_econActive,
                     operands,
                     LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// econExpected

   operands.clear();
   operands.push_back( u_unitOn.get() );
   operands.push_back( u_Tao_LT_TasSetpt.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_unitOn->Now()  &&
                              u_Tao_LT_TasSetpt->Now() );
                  } );

   u_econExpected =  std::make_unique<CFactFromFacts>(
                              seq0Ref,
                              *u_Subject,
                              EDataLabel::Fact_subj_ahu_econExpected,
                              operands,
                              LambdaToCopy );
 
//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Instantiate Knowledge Base
//======================================================================================================/
// first - instantiate Rule objects

   std::vector<AFact*> operands_if(0);
   std::vector<AFact*> operands_then(0);
   std::function<bool(void)> LambdaToCopy_if( [&]() -> bool { return NaNBOOL; } );
   std::function<bool(void)> LambdaToCopy_then( [&]() -> bool { return NaNBOOL; } );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 1

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_outputLevel.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_Tas_EQ_TasSetpt.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 (! u_econExpected->Now()) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_Tas_EQ_TasSetpt->Now() );
                        } );

   u_Rule1 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(1u),
                                       EAlertMsg::Rule_ahuSdvr_1_focus,
                                       EAlertMsg::Rule_ahuSdvr_1_if,
                                       EAlertMsg::Rule_ahuSdvr_1_then,
                                       EAlertMsg::Rule_ahuSdvr_1_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );
 
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 2

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_outputLevel.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_TasTrackingHigh.get() );
   operands_then.push_back( u_TasTrackingLow.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 (! u_econExpected->Now()) );
                        }
   );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( (! u_TasTrackingHigh->Now()) && 
                                    (! u_TasTrackingLow->Now()) );
                           }
   );

   u_Rule2 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(2u),
                                       EAlertMsg::Rule_ahuSdvr_2_focus,
                                       EAlertMsg::Rule_ahuSdvr_2_if,
                                       EAlertMsg::Rule_ahuSdvr_2_then,
                                       EAlertMsg::Rule_ahuSdvr_2_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

 
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 3

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_outputLevel.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_Tas_EQ_TasSetpt.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 u_econExpected->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_Tas_EQ_TasSetpt->Now() );
                        } );

   u_Rule3 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(3u),
                                       EAlertMsg::Rule_ahuSdvr_3_focus,
                                       EAlertMsg::Rule_ahuSdvr_3_if,
                                       EAlertMsg::Rule_ahuSdvr_3_then,
                                       EAlertMsg::Rule_ahuSdvr_3_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

 
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 4

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_outputLevel.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_TasTrackingHigh.get() );
   operands_then.push_back( u_TasTrackingLow.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 u_econExpected->Now() );
                        } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( ! u_TasTrackingHigh->Now() && 
                                    ! u_TasTrackingLow->Now() );
                        } );

   u_Rule4 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(4u),
                                       EAlertMsg::Rule_ahuSdvr_4_focus,
                                       EAlertMsg::Rule_ahuSdvr_4_if,
                                       EAlertMsg::Rule_ahuSdvr_4_then,
                                       EAlertMsg::Rule_ahuSdvr_4_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 5

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_UvcSteady.get() );
   operands_if.push_back( u_UvcShut.get() );
   operands_then.push_back( u_chwCoolingAir.get() );


   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_UvcSteady->Now() &&
                                 u_UvcShut->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( ! u_chwCoolingAir->Now() );
                        } );

   u_Rule5 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(5u),
                                       EAlertMsg::Rule_ahuSdvr_5_focus,
                                       EAlertMsg::Rule_ahuSdvr_5_if,
                                       EAlertMsg::Rule_ahuSdvr_5_then,
                                       EAlertMsg::Rule_ahuSdvr_5_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 6

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_outputLevel.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_econActive.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 u_econExpected->Now() );
                        } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_econActive->Now() );
                        } );

   u_Rule6 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(6u),
                                       EAlertMsg::Rule_ahuSdvr_6_focus,
                                       EAlertMsg::Rule_ahuSdvr_6_if,
                                       EAlertMsg::Rule_ahuSdvr_6_then,
                                       EAlertMsg::Rule_ahuSdvr_6_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 7

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_chwNeeded.get() );
   operands_then.push_back( u_chwCoolingAir.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_outputLevel->Now() &&
                                 u_chwNeeded->Now() );
                     } );



   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_chwCoolingAir->Now() );
                        } );

   u_Rule7 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(7u),
                                       EAlertMsg::Rule_ahuSdvr_7_focus,
                                       EAlertMsg::Rule_ahuSdvr_7_if,
                                       EAlertMsg::Rule_ahuSdvr_7_then,
                                       EAlertMsg::Rule_ahuSdvr_7_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 8
// *** TBD: replace Tam here with a preheat temp sensor Tap located between HW coil and CHW coil ***

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_then.push_back( u_Tam_GT_frzStat.get() );
 
   LambdaToCopy_if = ( [&]() -> bool { return u_unitOn->Now(); } );

   LambdaToCopy_then =  ( [&]() -> bool { return u_Tam_GT_frzStat->Now(); } );

   u_Rule8 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(8u),
                                       EAlertMsg::Rule_ahuSdvr_8_focus,
                                       EAlertMsg::Rule_ahuSdvr_8_if,
                                       EAlertMsg::Rule_ahuSdvr_8_then,
                                       EAlertMsg::Rule_ahuSdvr_8_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 9

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_then.push_back( u_Tam_GTE_minTaoTar.get() );
   operands_then.push_back( u_Tam_LTE_maxTaoTar.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_Tam_GTE_minTaoTar->Now() &&
                                    u_Tam_LTE_maxTaoTar->Now() );
                        } );

   u_Rule9 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(9u),
                                       EAlertMsg::Rule_ahuSdvr_9_focus,
                                       EAlertMsg::Rule_ahuSdvr_9_if,
                                       EAlertMsg::Rule_ahuSdvr_9_then,
                                       EAlertMsg::Rule_ahuSdvr_9_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 10

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_unitOn.get() );
   operands_if.push_back( u_absDifTaoTar_GTE_10F.get() );
   operands_if.push_back( u_econExpected.get() );
   operands_then.push_back( u_fracOAatMin.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_unitOn->Now() &&
                                 u_absDifTaoTar_GTE_10F->Now() &&
                                 ( ! u_econExpected->Now() ) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( u_fracOAatMin->Now() );
                        } );

   u_Rule10 = std::make_unique<CRule>( ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(10u),
                                       EAlertMsg::Rule_ahuSdvr_10_focus,
                                       EAlertMsg::Rule_ahuSdvr_10_if,
                                       EAlertMsg::Rule_ahuSdvr_10_then,
                                       EAlertMsg::Rule_ahuSdvr_10_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Instantiate Hypotheses objects

   u_Hypo1 = std::make_unique<CHypo>(1u, "Spurious failure on rule. Adjust parameters in rule operands.");
   u_Hypo2 = std::make_unique<CHypo>(2u, "Outside air temperature sensor bad.");
   u_Hypo3 = std::make_unique<CHypo>(3u, "Return air temperature sensor bad.");
   u_Hypo4 = std::make_unique<CHypo>(4u, "Mixed air temperature sensor bad.");
   u_Hypo5 = std::make_unique<CHypo>(5u, "Supply air temperature sensor bad.");
   u_Hypo6 = std::make_unique<CHypo>(6u, "CHW valve leaking.");
   u_Hypo7 = std::make_unique<CHypo>(7u, "CHW valve stuck.");
   u_Hypo8 = std::make_unique<CHypo>(8u, "CHW valve control loop trouble.");
   u_Hypo9 = std::make_unique<CHypo>(9u, "CHWS temperature setpt. too high.");
   u_Hypo10 = std::make_unique<CHypo>(10u, "CHWS pressure setpt. too low.");
   u_Hypo11 = std::make_unique<CHypo>(11u, "Cooling coil fouled airside or waterside.");
   u_Hypo12 = std::make_unique<CHypo>(12u, "Preheat valve leaking.");
   u_Hypo13 = std::make_unique<CHypo>(13u, "Preheat valve stuck.");
   u_Hypo14 = std::make_unique<CHypo>(14u, "Preheat control trouble.");
   u_Hypo15 = std::make_unique<CHypo>(15u, "HWS temperature setpt. too low.");
   u_Hypo16 = std::make_unique<CHypo>(16u, "HWS pressure setpt. too low.");
   u_Hypo17 = std::make_unique<CHypo>(17u, "Preheat coil fouled airside or waterside.");
   u_Hypo18 = std::make_unique<CHypo>(18u, "Mix damper stuck.");
   u_Hypo19 = std::make_unique<CHypo>(19u, "Mix damper control loop trouble.");


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Instantiate Evidence objects

   u_Evid1 = std::make_unique<CEvid>(1u, "Does case appear spurious, a 'false alarm'? (e.g., Are zone conditions okay?)");
   u_Evid2 = std::make_unique<CEvid>(2u, "Does outside air temp. signal agree adequately with a ref. measurement?");
   u_Evid3 = std::make_unique<CEvid>(3u, "Does return air temp. signal agree adequately with a ref. measurement?");
   u_Evid4 = std::make_unique<CEvid>(4u, "Does mixed air temp. signal agree adequately with a ref. measurement?");
   u_Evid5 = std::make_unique<CEvid>(5u, "Does supply temp. signal agree adequately with a ref. measurement?");
   u_Evid6 = std::make_unique<CEvid>(6u, "Is CHW coil flow negligible if valve driven shut (e.g., coil warms up)?");
   u_Evid7 = std::make_unique<CEvid>(8u, "Does CHW valve cycle freely, and position as commanded?");
   u_Evid8 = std::make_unique<CEvid>(7u, "Are CHW valve ctrl loop settings and signals correct?");
   u_Evid9 = std::make_unique<CEvid>(9u, "Is CHWS temperature setpt. low enough for zone loads?");
   u_Evid10 = std::make_unique<CEvid>(10u, "Is CHWS pressure setpt. high enough for zone loads?");
   u_Evid11 = std::make_unique<CEvid>(11u, "Are cooling coil fins acceptably clean and water d/p in spec?");
   u_Evid12 = std::make_unique<CEvid>(12u, "Is HW coil flow negligible if valve driven shut (e.g., coil cools off)?");
   u_Evid13 = std::make_unique<CEvid>(13u, "Does HW valve cycle freely, and position as commanded?");
   u_Evid14 = std::make_unique<CEvid>(14u, "Are preheat ctrl settings and signals correct?");
   u_Evid15 = std::make_unique<CEvid>(15u, "Is HWS temperature setpt. high enough for zone loads?");
   u_Evid16 = std::make_unique<CEvid>(16u, "Is HWS pressure setpt. high enough for zone loads?");
   u_Evid17 = std::make_unique<CEvid>(17u, "Are HW coil fins acceptably clean and water d/p in spec?");
   u_Evid18 = std::make_unique<CEvid>(18u, "Does mix damper cycle freely, and position as commanded?");
   u_Evid19 = std::make_unique<CEvid>(19u, "Are mix damper ctrl loop settings and signals correct?");

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Associate Hypos to Rules (and, thus, to the implied CRuleKit object)

   u_Rule1->AssociateHypo( u_Hypo1.get() );
   u_Rule1->AssociateHypo( u_Hypo5.get() );

   u_Rule2->AssociateHypo( u_Hypo1.get() );
   u_Rule2->AssociateHypo( u_Hypo5.get() );
   u_Rule2->AssociateHypo( u_Hypo7.get() );
   u_Rule2->AssociateHypo( u_Hypo9.get() );
   u_Rule2->AssociateHypo( u_Hypo10.get() );
   u_Rule2->AssociateHypo( u_Hypo11.get() );


   u_Rule3->AssociateHypo( u_Hypo1.get() );
   u_Rule3->AssociateHypo( u_Hypo5.get() );

   u_Rule4->AssociateHypo( u_Hypo1.get() );
   u_Rule4->AssociateHypo( u_Hypo18.get() );
   u_Rule4->AssociateHypo( u_Hypo19.get() );

   u_Rule5->AssociateHypo( u_Hypo1.get() );
   u_Rule5->AssociateHypo( u_Hypo6.get() );

   u_Rule6->AssociateHypo( u_Hypo1.get() );
   u_Rule6->AssociateHypo( u_Hypo18.get() );
   u_Rule6->AssociateHypo( u_Hypo19.get() );

   u_Rule7->AssociateHypo( u_Hypo1.get() );
   u_Rule7->AssociateHypo( u_Hypo7.get() );
   u_Rule7->AssociateHypo( u_Hypo8.get() );

   u_Rule8->AssociateHypo( u_Hypo1.get() );
   u_Rule8->AssociateHypo( u_Hypo13.get() );
   u_Rule8->AssociateHypo( u_Hypo14.get() );
   u_Rule8->AssociateHypo( u_Hypo15.get() );
   u_Rule8->AssociateHypo( u_Hypo16.get() );
   u_Rule8->AssociateHypo( u_Hypo17.get() );

   u_Rule9->AssociateHypo( u_Hypo1.get() );
   u_Rule9->AssociateHypo( u_Hypo2.get() );
   u_Rule9->AssociateHypo( u_Hypo3.get() );
   u_Rule9->AssociateHypo( u_Hypo4.get() );

   u_Rule10->AssociateHypo( u_Hypo1.get() );
   u_Rule10->AssociateHypo( u_Hypo18.get() );
   u_Rule10->AssociateHypo( u_Hypo19.get() );


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Associate Evids to Hypos : u_Hypo->AssociateEvid( p_Evid, direct?, invertible? );

   u_Hypo1->AssociateEvid( u_Evid1.get(), true, true );
   u_Hypo2->AssociateEvid( u_Evid2.get(), false, true );
   u_Hypo3->AssociateEvid( u_Evid3.get(), false, true );
   u_Hypo4->AssociateEvid( u_Evid4.get(), false, true );
   u_Hypo5->AssociateEvid( u_Evid5.get(), false, true );
   u_Hypo6->AssociateEvid( u_Evid6.get(), false, true );
   u_Hypo7->AssociateEvid( u_Evid7.get(), false, true );
   u_Hypo8->AssociateEvid( u_Evid8.get(), false, true );
   u_Hypo9->AssociateEvid( u_Evid9.get(), false, true );
   u_Hypo10->AssociateEvid( u_Evid10.get(), false, true );
   u_Hypo11->AssociateEvid( u_Evid11.get(), false, true );
   u_Hypo12->AssociateEvid( u_Evid12.get(), false, true );
   u_Hypo13->AssociateEvid( u_Evid13.get(), false, true );
   u_Hypo14->AssociateEvid( u_Evid14.get(), false, true );
   u_Hypo15->AssociateEvid( u_Evid15.get(), false, true );
   u_Hypo16->AssociateEvid( u_Evid16.get(), false, true );
   u_Hypo17->AssociateEvid( u_Evid17.get(), false, true );
   u_Hypo18->AssociateEvid( u_Evid18.get(), false, true );
   u_Hypo19->AssociateEvid( u_Evid19.get(), false, true );


// *** !!! DON'T FORGET ME: !!! ***

   u_RuleKit->FinalizeRuleKitAndBuildKbase();

//======================================================================================================/
// Instantiate GUI Features


   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat01 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   1,
                                                   *(u_Bso->u_Fact),
                                                   ([&]()->bool { return u_Bso->u_Fact->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat02 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   2,
                                                   *u_inputsSteady,
                                                   ([&]()->bool { return u_inputsSteady->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat03 =  std::make_unique<CFeatureAnalog>(
                     *u_Subject,
                     viewRef,
                     3,
                     *u_Tas,
                     ([&]()->float { return u_Tas->u_Rain->NowY(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat04 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   4,
                                                   *u_Tas_EQ_TasSetpt,
                                                   ([&]()->bool { return u_Tas_EQ_TasSetpt->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat05 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   5,
                                                   *u_TasTrackingHigh,
                                                   ([&]()->bool { return u_TasTrackingHigh->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat06 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   6,
                                                   *u_econExpected,
                                                   ([&]()->bool { return u_econExpected->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat07 = std::make_unique<CFeatureFact>(   *u_Subject,
                                                    viewRef,
                                                    7,
                                                    *u_econActive,
                                                    ([&]()->bool { return u_econActive->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat08 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   8,
                                                   *u_chwNeeded,
                                                   ([&]()->bool { return u_chwNeeded->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat09 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   9,
                                                   *u_chwCoolingAir,
                                                   ([&]()->bool { return u_chwCoolingAir->Now(); }) );

}  // End of Constructor

//=======================================================================================================/
// Destructor

CTool_ahu_ibal::~CTool_ahu_ibal( void ) { /* Emtpy d-tor*/ }


// End of AHU Tool Definition
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Start of VAV Box Tool Definition


CTool_vav_ibal::CTool_vav_ibal(  CDomain& domainRef,
                                 EDataLabel vavLabel,
                                 ERealName vavName,
                                 ERealName ahuName,
                                 ERealName hwPlantName,
                                 const CClockPerPort& clockRef,
                                 CSequence& seq0Ref,
                                 CController& ctrlrRef,
                                 CView& viewRef,
                                 CPortOmni& portRef )
                                 :  ATool(   domainRef,
                                             vavLabel,
                                             vavName, 
                                             clockRef,
                                             seq0Ref,
                                             ctrlrRef,
                                             viewRef,
                                             portRef
                                    ) {

//======================================================================================================/
// $$$ TBD to move these reassignments to ATool initialization list (i.e., learn smt ptr move-semantics)

u_Subject = std::make_unique<CSubj_vav_pihr>(
               domainRef,
               vavLabel,
               vavName,
               ahuName,
               hwPlantName,
               12.0f,      // inch diam duct
               2500.0f,    // cfm rated
               5.0f,       // gpm rated
               2.0f        // reheat kW
);

u_RuleKit = std::make_unique<CRuleKit>(
               seq0Ref,
               *u_Subject,
               EDataLabel::RuleKit, 
               START_RULEKIT_SECSBETWEENTRAPS,
               ctrlrRef
);


//======================================================================================================/
// Point objects

   u_Psai =          std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_pressure_static_air_inlet,
                                                      EDataUnit::PressureGage_iwg,
                                                      EDataRange::Analog_zeroTo3,
                                                      EPlotGroup::Free,
                                         // $$$ TBD to match enum below to actual BAS point list names $$$
                                                      EPointName::Pressure_static_air_supply,
                                                      ctrlrRef );

   u_Tai =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_inlet,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::Free,
                                                      EPointName::Temperature_air_supply,
                                                      ctrlrRef );

   
   u_Tad =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_discharge,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::Free,
                                                      EPointName::Temperature_air_discharge,
                                                      ctrlrRef );

   u_Taz =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_zone,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::GroupA,
                                                      EPointName::Temperature_air_zone,
                                                      ctrlrRef );

   u_TazSetptHtg =   std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_zone_setpt_htg,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::GroupA,
                                                      EPointName::Temperature_air_zone_setpt_htg,
                                                      ctrlrRef );

   u_TazSetptClg =   std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_temperature_air_zone_setpt_clg,
                                                      EDataUnit::Temperature_degF,
                                                      EDataRange::Analog_zeroTo120,
                                                      EPlotGroup::GroupA,
                                                      EPointName::Temperature_air_zone_setpt_clg,
                                                      ctrlrRef );

   u_Uvh =           std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_command_valve_hw,
                                                      EDataUnit::Ratio_percent,
                                                      EDataRange::Analog_percent,
                                                      EPlotGroup::Free,
                                                      EPointName::Position_valve_hw,
                                                      ctrlrRef );

   u_Udd =            std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_command_damper_disch,
                                                      EDataUnit::Ratio_percent,
                                                      EDataRange::Analog_percent,
                                                      EPlotGroup::Free,
                                                      EPointName::Position_damper_vav,
                                                      ctrlrRef );

   u_Qad =           std::make_unique<CPointAnalog>(   seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_flowVolume_air_disch,
                                                      EDataUnit::FlowVolume_cfm,
                                                      EDataRange::Analog_zeroTo3k,
                                                      EPlotGroup::GroupA,
                                                      EPointName::FlowRateVolume_air_vav,
                                                      ctrlrRef );

   u_QadSetpt =      std::make_unique<CPointAnalog>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_flowVolume_air_disch_setpt,
                                                      EDataUnit::FlowVolume_cfm,
                                                      EDataRange::Analog_zeroTo3k,
                                                      EPlotGroup::GroupA,
                                                      EPointName::FlowRateVolume_air_vav_setpt,
                                                      ctrlrRef );

   u_Bzo =           std::make_unique<CPointBinary>(  seq0Ref,
                                                      *u_Subject,
                                                      EDataLabel::Point_binary_zone_occupied,
                                                      EDataLabel::Fact_direct_Bzo,
                                                      EPointName::Binary_zoneOccupied,
                                                      ctrlrRef );

//======================================================================================================/
// Formula objects


//======================================================================================================/
// Chart objects - Shewhart charts

   u_PsaiShew =
      std::make_unique<CChartShewhart>( seq0Ref, *u_Subject, u_Psai.get(), ctrlrRef );

   u_QadShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Qad.get(), ctrlrRef);

   u_QadSetptShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_QadSetpt.get(), ctrlrRef);

   u_TadShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Tad.get(), ctrlrRef);

   u_TaiShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Tai.get(), ctrlrRef);

   u_TazShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Taz.get(), ctrlrRef);

   u_TazSetptClgShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_TazSetptClg.get(), ctrlrRef);

   u_TazSetptHtgShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_TazSetptHtg.get(), ctrlrRef);

   u_UddShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Udd.get(), ctrlrRef);

   u_UvhShew =
      std::make_unique<CChartShewhart>(seq0Ref, *u_Subject, u_Uvh.get(), ctrlrRef);

//======================================================================================================/
// Chart objects - tracking charts
// float halfband = (+/-),  float warn = (units-of-x)-minutes over a reset

   u_Qad_VS_setpt = std::make_unique<CChartTracking>( seq0Ref,
                                                      *u_Subject,
                                                      u_Qad.get(),
                                                      u_QadShew.get(),
                                                      u_QadSetpt.get(),
                                                      u_QadSetptShew.get(),
                                                      INIT_MINDEFMAX_TRACKING_HALFBAND_CFM_0TO3K,
                                                      INIT_MINDEFMAX_TRACKING_WARN_CFM_0TO3K,
                                                      ctrlrRef );

   u_Tad_VS_auto = std::make_unique<CChartTracking>(  seq0Ref,
                                                      *u_Subject,
                                                      u_Tad.get(),
                                                      u_TadShew.get(),
                                                      INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120,
                                                      INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120,
                                                      ctrlrRef );

   u_Taz_VS_setptClg = std::make_unique<CChartTracking>( seq0Ref,
                                                         *u_Subject,
                                                         u_Taz.get(),
                                                         u_TazShew.get(),
                                                         u_TazSetptClg.get(),
                                                         u_TazSetptClgShew.get(),
                                                         INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120,
                                                         INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120,
                                                         ctrlrRef );

   u_Taz_VS_setptHtg = std::make_unique<CChartTracking>( seq0Ref,
                                                         *u_Subject,
                                                         u_Taz.get(),
                                                         u_TazShew.get(),
                                                         u_TazSetptHtg.get(),
                                                         u_TazSetptHtgShew.get(),
                                                         INIT_MINDEFMAX_TRACKING_HALFBAND_DEGF_0TO120,
                                                         INIT_MINDEFMAX_TRACKING_WARN_DEGF_0TO120,
                                                         ctrlrRef );

//======================================================================================================/
// Fact objects based on charts - Shewhart charts 

   u_PsaiSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_PsaiSteady,
                                                                           *u_PsaiShew,
                                                                           &CChartShewhart::IsSteady );

   u_QadSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QadSteady,
                                                                           *u_QadShew,
                                                                           &CChartShewhart::IsSteady );

   u_QadSetptSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QadSetptSteady,
                                                                           *u_QadSetptShew,
                                                                           &CChartShewhart::IsSteady );

   u_TadSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TadSteady,
                                                                           *u_TadShew,
                                                                           &CChartShewhart::IsSteady );

   u_TaiSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TasSteady,
                                                                           *u_TaiShew,
                                                                           &CChartShewhart::IsSteady );

   u_TazSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TazSteady,
                                                                           *u_TazShew,
                                                                           &CChartShewhart::IsSteady );

   u_TazSetptClgSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TazSetptClgSteady,
                                                                           *u_TazSetptClgShew,
                                                                           &CChartShewhart::IsSteady );

   u_TazSetptHtgSteady =
      std::make_unique< CFactFromChart< CChartShewhart, PtrShewGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TazSetptHtgSteady,
                                                                           *u_TazSetptHtgShew,
                                                                           &CChartShewhart::IsSteady );

   u_UddSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_UddSteady,
                                                                           *u_UddShew,
                                                                           &CChartShewhart::IsSteady );

   u_UvhSteady =
      std::make_unique< CFactFromChart<CChartShewhart, PtrShewGetr_t> >(  seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_UvhSteady,
                                                                           *u_UvhShew,
                                                                           &CChartShewhart::IsSteady );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Fact objects based on charts - tracking charts

   u_QadHunting =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QadHunting,
                                                                           *u_Qad_VS_setpt,
                                                                           &CChartTracking::IsHunting );

   u_QadTrackingHigh =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QadTrackingHigh,
                                                                           *u_Qad_VS_setpt,
                                                                           &CChartTracking::IsRising );

   u_QadTrackingLow =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_QadTrackingLow,
                                                                           *u_Qad_VS_setpt,
                                                                           &CChartTracking::IsFalling );

   u_TadHunting =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TadHunting,
                                                                           *u_Tad_VS_auto,
                                                                           &CChartTracking::IsHunting );

   u_TazTrackingHigh =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TazTrackingHigh,
                                                                           *u_Taz_VS_setptClg,
                                                                           &CChartTracking::IsRising );

   u_TazTrackingLow =
      std::make_unique< CFactFromChart<CChartTracking, PtrTrakGetr_t> >( seq0Ref,
                                                                           *u_Subject,
                                                                           EDataLabel::Fact_data_TazTrackingLow,
                                                                           *u_Taz_VS_setptHtg,
                                                                           &CChartTracking::IsFalling );

//======================================================================================================/ 
// CFactRelatingLeftToParam
// EQ specializations

   u_QadZero = std::make_unique<  CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_QadZero,
                                    u_Qad.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_ZERO,
                                    INIT_MINDEFMAX_RELATE_HYSTER_CFM_0TO3K,
                                    INIT_MINDEFMAX_RELATE_SLACK_CFM_0TO3K,
                                    ctrlrRef );

   u_UddFull = std::make_unique< CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_UddFull,
                                    u_Udd.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_PERCENT_FULL,
                                    INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT,
                                    INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT,
                                    ctrlrRef );

   u_UvhShut = std::make_unique<  CFactRelatingLeftToParam<
                                    CPointAnalog,
                                    PtrRainGetr_t,
                                    Left_EQ_Right>
                                 >( seq0Ref,
                                    *u_Subject,
                                    EDataLabel::Fact_para_UvhShut,
                                    u_Uvh.get(),
                                    &CRainAnalog::NowY,
                                    FIXED_PARAM_PERCENT_SHUT,
                                    INIT_MINDEFMAX_RELATE_HYSTER_ANYPERCENT,
                                    INIT_MINDEFMAX_RELATE_SLACK_ANYPERCENT,
                                    ctrlrRef );

//======================================================================================================/ 
// CFactRelatingLeftToRight
// GT specializations

   u_Tad_GT_Tai = std::make_unique< CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_GT_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                       >( seq0Ref,
                                          *u_Subject,
                                          EDataLabel::Fact_data_Tad_GT_Tai,
                                          u_Tad.get(),
                                          &CRainAnalog::NowY,
                                          u_Tai.get(),
                                          &CRainAnalog::NowY,
                                          INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                          INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_HARD,
                                          ctrlrRef );

   u_Tad_GT_Taz = std::make_unique< CFactRelatingLeftToRight<
                                       CPointAnalog,
                                       PtrRainGetr_t,
                                       Left_GT_Right,
                                       CPointAnalog,
                                       PtrRainGetr_t>
                                    >( seq0Ref,
                                      *u_Subject,
                                      EDataLabel::Fact_data_Tad_GT_Taz,
                                      u_Tad.get(),
                                      &CRainAnalog::NowY,
                                      u_Taz.get(),
                                      &CRainAnalog::NowY,
                                      INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                      INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                      ctrlrRef );

   u_Taz_GT_setptClg = std::make_unique<   CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_GT_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                       >( seq0Ref,
                                          *u_Subject,
                                          EDataLabel::Fact_data_Taz_GT_setptClg,
                                          u_Taz.get(),
                                          &CRainAnalog::NowY,
                                          u_TazSetptClg.get(),
                                          &CRainAnalog::NowY,
                                          INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                          INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_HARD,
                                          ctrlrRef );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/ 
// LT specializations

   u_Tad_LT_Taz = std::make_unique< CFactRelatingLeftToRight<
                                       CPointAnalog,
                                       PtrRainGetr_t,
                                       Left_LT_Right,
                                       CPointAnalog,
                                       PtrRainGetr_t>
                                    >( seq0Ref,
                                       *u_Subject,
                                       EDataLabel::Fact_data_Tad_LT_Taz,
                                       u_Tad.get(),
                                       &CRainAnalog::NowY,
                                       u_Taz.get(),
                                       &CRainAnalog::NowY,
                                       INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                       INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_EASY,
                                       ctrlrRef );

   u_Taz_LT_setptHtg = std::make_unique< CFactRelatingLeftToRight<
                                          CPointAnalog,
                                          PtrRainGetr_t,
                                          Left_LT_Right,
                                          CPointAnalog,
                                          PtrRainGetr_t>
                                       >( seq0Ref,
                                          *u_Subject,
                                          EDataLabel::Fact_data_Taz_LT_setptHtg,
                                          u_Taz.get(),
                                          &CRainAnalog::NowY,
                                          u_TazSetptHtg.get(),
                                          &CRainAnalog::NowY,
                                          INIT_MINDEFMAX_RELATE_HYSTER_DEGF_0TO120,
                                          INIT_MINDEFMAX_RELATE_SLACK_DEGF_0TO120_HARD,
                                          ctrlrRef );

//======================================================================================================/
// Define direct Fact objects

   u_zoneOccupied = std::make_unique<CFactFromPoint>(
                     seq0Ref,
                     *u_Subject,
                     EDataLabel::Fact_direct_Bzo,
                     *u_Bzo);

//======================================================================================================/
// Define CFactFromAntecedentSubject object

   u_ahuOutputOkay =   std::make_unique<CFactFromAntecedentSubject>(
                        seq0Ref,
                        *u_Subject,
                        EDataLabel::Fact_antecedent_vav_ahuOkay,
                        domainRef,
                        ahuName );

//======================================================================================================/
// Define CFactFromFacts objects

   std::vector<AFact*> operands(0);
   operands.reserve(4);
   std::function<bool(void)>LambdaToCopy( [&]() -> bool { return NaNBOOL; } );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// unitCoolingZone

   operands.clear();
   operands.push_back( u_Tad_LT_Taz.get() );
   operands.push_back( u_QadZero.get() );

  LambdaToCopy =  (  [&]() -> bool {
                        return ( ( u_Tad_LT_Taz->Now() ) &&
                                 ( ! u_QadZero->Now() ) );
                     } );

   u_unitCoolingZone =  std::make_unique<CFactFromFacts>(
                              seq0Ref,
                              *u_Subject,
                              EDataLabel::Fact_subj_vav_unitCoolingZone,
                              operands,
                              LambdaToCopy );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// unitHeatingZone

   operands.clear();
   operands.push_back( u_Tad_GT_Taz.get() );
   operands.push_back( u_QadZero.get() );
 
   LambdaToCopy = (  [&]() -> bool {
                        return ( u_Tad_GT_Tai->Now() &&
                                 ( ! u_QadZero->Now() ) );
                     } );

   u_unitReheating = std::make_unique<CFactFromFacts>( seq0Ref,
                                                         *u_Subject,
                                                         EDataLabel::Fact_subj_vav_unitReheating,
                                                         operands,
                                                         LambdaToCopy );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// inputsSteady

   operands.clear();
   operands.push_back( u_PsaiSteady.get() );
   operands.push_back( u_TaiSteady.get() );
   operands.push_back( u_TazSetptHtgSteady.get() );
   operands.push_back( u_TazSetptClgSteady.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_PsaiSteady->Now() &&
                              u_TaiSteady->Now() &&
                              u_TazSetptHtgSteady->Now() &&
                              u_TazSetptClgSteady->Now() );
                  } );

   u_inputsSteady =  std::make_unique<CFactFromFacts>(
                              seq0Ref,
                              *u_Subject,
                              EDataLabel::Fact_subj_vav_inputsSteady,
                              operands,
                              LambdaToCopy );
 
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// TazInBand

   operands.clear();
   operands.push_back( u_inputsSteady.get() );
   operands.push_back( u_Taz_GT_setptClg.get() );
   operands.push_back( u_Taz_LT_setptHtg.get() );

   LambdaToCopy = ( [&]() -> bool {
                     return ( u_inputsSteady->Now() &&
                              ( ! (u_Taz_GT_setptClg->Now() || u_Taz_LT_setptHtg->Now()) ) );
                  } );

      u_TazInBand = std::make_unique<CFactFromFacts>( seq0Ref,
                                                   *u_Subject,
                                                   EDataLabel::Fact_subj_vav_TazInBand,
                                                   operands,
                                                   LambdaToCopy );

 
//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Instantiate Knowledge Base

//======================================================================================================/
// first - instantiate Rule objects

   std::vector<AFact*> operands_if(0);
   std::vector<AFact*> operands_then(0);
   std::function<bool(void)> LambdaToCopy_if( [&]() -> bool { return NaNBOOL; } );
   std::function<bool(void)> LambdaToCopy_then( [&]() -> bool { return NaNBOOL; } );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 1

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_ahuOutputOkay.get() );
   operands_if.push_back( u_inputsSteady.get() );

   operands_then.push_back( u_QadTrackingLow.get() );
   operands_then.push_back( u_QadTrackingHigh.get() );

   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_ahuOutputOkay->Now() &&
                                 u_inputsSteady->Now()
                        );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool {
                           return ( ! (u_QadTrackingLow->Now() || u_QadTrackingHigh->Now()) );
                        } );

   u_Rule1 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(1u),
                                       EAlertMsg::Rule_vav_1_focus,
                                       EAlertMsg::Rule_vav_1_if,
                                       EAlertMsg::Rule_vav_1_then,
                                       EAlertMsg::Rule_vav_1_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );
 
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 2

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_ahuOutputOkay.get() );
   operands_if.push_back( u_inputsSteady.get() );

   operands_then.push_back( u_QadHunting.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_ahuOutputOkay->Now() &&
                                 u_inputsSteady->Now()
                        );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return !( u_QadHunting->Now() ); } );

   u_Rule2 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(2u),
                                       EAlertMsg::Rule_vav_2_focus,
                                       EAlertMsg::Rule_vav_2_if,
                                       EAlertMsg::Rule_vav_2_then,
                                       EAlertMsg::Rule_vav_2_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 3

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_ahuOutputOkay.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_Taz_GT_setptClg.get() );

   operands_then.push_back( u_unitCoolingZone.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_ahuOutputOkay->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_Taz_GT_setptClg->Now());
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return u_unitCoolingZone->Now(); } );

   u_Rule3 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(3u),
                                       EAlertMsg::Rule_vav_3_focus,
                                       EAlertMsg::Rule_vav_3_if,
                                       EAlertMsg::Rule_vav_3_then,
                                       EAlertMsg::Rule_vav_3_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 4

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_unitCoolingZone.get() );
   operands_if.push_back( u_UddFull.get() );

   operands_then.push_back( u_TazTrackingHigh.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_unitCoolingZone->Now() &&
                                 u_UddFull->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return ! u_TazTrackingHigh->Now(); } );

   u_Rule4 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(4u),
                                       EAlertMsg::Rule_vav_4_focus,
                                       EAlertMsg::Rule_vav_4_if,
                                       EAlertMsg::Rule_vav_4_then,
                                       EAlertMsg::Rule_vav_4_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 5

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_unitCoolingZone.get() );
   operands_if.push_back( u_UddFull.get() );

   operands_then.push_back( u_TazTrackingHigh.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_unitCoolingZone->Now() &&
                                 ( ! u_UddFull->Now() ) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return ! u_TazTrackingHigh->Now(); } );

   u_Rule5 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(5u),
                                       EAlertMsg::Rule_vav_5_focus,
                                       EAlertMsg::Rule_vav_5_if,
                                       EAlertMsg::Rule_vav_5_then,
                                       EAlertMsg::Rule_vav_5_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 6

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_Taz_LT_setptHtg.get() );
 
   operands_then.push_back( u_unitReheating.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_Taz_LT_setptHtg->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return u_unitReheating->Now(); } );

   u_Rule6 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(6u),
                                       EAlertMsg::Rule_vav_6_focus,
                                       EAlertMsg::Rule_vav_6_if,
                                       EAlertMsg::Rule_vav_6_then,
                                       EAlertMsg::Rule_vav_6_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_PINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 7

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_unitReheating.get() );
   operands_if.push_back( u_UddFull.get() );

   operands_then.push_back( u_TazTrackingLow.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_unitReheating->Now() &&
                                 u_UddFull->Now() );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return ! u_TazTrackingLow->Now(); } );

   u_Rule7 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(7u),
                                       EAlertMsg::Rule_vav_7_focus,
                                       EAlertMsg::Rule_vav_7_if,
                                       EAlertMsg::Rule_vav_7_then,
                                       EAlertMsg::Rule_vav_7_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 8

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_unitReheating.get() );
   operands_if.push_back( u_UddFull.get() );

   operands_then.push_back( u_TazTrackingLow.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_inputsSteady->Now() &&
                                 u_unitReheating->Now() &&
                                 ( ! u_UddFull->Now() ) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return ! u_TazTrackingLow->Now(); } );

   u_Rule8 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(8u),
                                       EAlertMsg::Rule_vav_8_focus,
                                       EAlertMsg::Rule_vav_8_if,
                                       EAlertMsg::Rule_vav_8_then,
                                       EAlertMsg::Rule_vav_8_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 9

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_UvhShut.get() );
   operands_if.push_back( u_QadZero.get() );

   operands_then.push_back( u_Tad_GT_Tai.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_inputsSteady->Now() &&
                                 u_UvhShut->Now() &&
                                 ( ! u_QadZero->Now() ) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return ! u_Tad_GT_Tai->Now(); } );

   u_Rule9 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(9u),
                                       EAlertMsg::Rule_vav_9_focus,
                                       EAlertMsg::Rule_vav_9_if,
                                       EAlertMsg::Rule_vav_9_then,
                                       EAlertMsg::Rule_vav_9_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Rule 10

   operands_if.clear();
   operands_then.clear();

   operands_if.push_back( u_zoneOccupied.get() );
   operands_if.push_back( u_ahuOutputOkay.get() );
   operands_if.push_back( u_inputsSteady.get() );
   operands_if.push_back( u_QadZero.get() );

   operands_then.push_back( u_TazInBand.get() );
 
   LambdaToCopy_if = ( [&]() -> bool {
                        return ( u_zoneOccupied->Now() &&
                                 u_ahuOutputOkay->Now() &&
                                 u_inputsSteady->Now() &&
                                 ( ! u_QadZero->Now() ) );
                     } );

   LambdaToCopy_then =  ( [&]() -> bool { return u_TazInBand->Now(); } );

   u_Rule10 = std::make_unique<CRule>(  ctrlrRef,
                                       *u_RuleKit,
                                       static_cast<Nzint_t>(10u),
                                       EAlertMsg::Rule_vav_10_focus,
                                       EAlertMsg::Rule_vav_10_if,
                                       EAlertMsg::Rule_vav_10_then,
                                       EAlertMsg::Rule_vav_10_onFail,
                                       operands_if,
                                       operands_then,
                                       LambdaToCopy_if,
                                       LambdaToCopy_then,
                                       FIXED_RULE_UNITOUTPUT_NOTPINNED );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Instantiate Hypotheses objects

   u_Hypo1 = std::make_unique<CHypo>(1u, "Spurious failure on rule. Adjust parameters in rule operands.");
   u_Hypo2 = std::make_unique<CHypo>(2u, "Air velocity sensor bad.");
   u_Hypo3 = std::make_unique<CHypo>(3u, "Inlet air temperature sensor bad.");
   u_Hypo4 = std::make_unique<CHypo>(4u, "Discharge air temperature sensor bad.");
   u_Hypo5 = std::make_unique<CHypo>(5u, "Zone air temperature sensor bad.");
   u_Hypo6 = std::make_unique<CHypo>(6u, "HW valve leaking.");
   u_Hypo7 = std::make_unique<CHypo>(7u, "HW valve stuck.");
   u_Hypo8 = std::make_unique<CHypo>(8u, "HW valve control loop trouble.");
   u_Hypo9 = std::make_unique<CHypo>(9u, "HW supply temperature setpt. too low.");
   u_Hypo10 = std::make_unique<CHypo>(10u, "HW supply flow trouble.");
   u_Hypo11 = std::make_unique<CHypo>(11u, "HW coil fouled.");
   u_Hypo12 = std::make_unique<CHypo>(12u, "Damper stuck.");
   u_Hypo13 = std::make_unique<CHypo>(13u, "Damper control loop trouble.");
   u_Hypo14 = std::make_unique<CHypo>(14u, "AHU supply air temperature setpt. too high.");
   u_Hypo15 = std::make_unique<CHypo>(15u, "AHU supply air pressure setpt. too low.");


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Instantiate Evidence objects

   u_Evid1 = std::make_unique<CEvid>(1u, "Does case appear spurious, a 'false alarm'? (e.g., Are zone conditions okay?)");
   u_Evid2 = std::make_unique<CEvid>(2u, "Does air velocity signal agree adequately with a ref. measurement?");
   u_Evid3 = std::make_unique<CEvid>(3u, "Does inlet air temp. signal agree adequately with a ref. measurement?");
   u_Evid4 = std::make_unique<CEvid>(4u, "Does disch. air temp. signal agree adequately with a ref. measurement?");
   u_Evid5 = std::make_unique<CEvid>(5u, "Does zone temp. signal agree adequately with a ref. measurement?");
   u_Evid6 = std::make_unique<CEvid>(6u, "Is coil HW flow negligible if HW valve driven shut (e.g., coil cools off)?");
   u_Evid7 = std::make_unique<CEvid>(7u, "Does HW valve cycle freely, and position as commanded?");
   u_Evid8 = std::make_unique<CEvid>(8u, "Are HW valve ctrl loop settings and signals correct?");
   u_Evid9 = std::make_unique<CEvid>(9u, "Is HW supply temp. setpt. high enough for zone loads?");
   u_Evid10 = std::make_unique<CEvid>(10u, "Are HW pump head, flow, and amps normal for speed commanded?");
   u_Evid11 = std::make_unique<CEvid>(11u, "Are HW coil fins acceptably clean?");
   u_Evid12 = std::make_unique<CEvid>(12u, "Does damper cycle freely, and position as commanded?");
   u_Evid13 = std::make_unique<CEvid>(13u, "Are damper ctrl loop settings and signals correct?");
   u_Evid14 = std::make_unique<CEvid>(14u, "Is AHU supply air temp. setpt. low enough for zone loads?");
   u_Evid15 = std::make_unique<CEvid>(15u, "Is AHU supply air press. setpt. high enough for zone loads?");


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Associate Hypos to Rules (and, thus, to the implied CRuleKit object)

   u_Rule1->AssociateHypo( u_Hypo1.get() );
   u_Rule1->AssociateHypo( u_Hypo2.get() );
   u_Rule1->AssociateHypo( u_Hypo12.get() );
   u_Rule1->AssociateHypo( u_Hypo13.get() );

   u_Rule2->AssociateHypo( u_Hypo1.get() );
   u_Rule2->AssociateHypo( u_Hypo13.get() );

   u_Rule3->AssociateHypo( u_Hypo1.get() );
   u_Rule3->AssociateHypo( u_Hypo6.get() );
   u_Rule3->AssociateHypo( u_Hypo7.get() );
   u_Rule3->AssociateHypo( u_Hypo12.get() );
   u_Rule3->AssociateHypo( u_Hypo14.get() );
   u_Rule3->AssociateHypo( u_Hypo15.get() );

   u_Rule4->AssociateHypo( u_Hypo1.get() );
   u_Rule4->AssociateHypo( u_Hypo5.get() );
   u_Rule4->AssociateHypo( u_Hypo14.get() );
   u_Rule4->AssociateHypo( u_Hypo15.get() );

   u_Rule5->AssociateHypo( u_Hypo1.get() );
   u_Rule5->AssociateHypo( u_Hypo12.get() );
   u_Rule5->AssociateHypo( u_Hypo14.get() );

   u_Rule6->AssociateHypo( u_Hypo1.get() );
   u_Rule6->AssociateHypo( u_Hypo7.get() );
   u_Rule6->AssociateHypo( u_Hypo8.get() );

   u_Rule7->AssociateHypo( u_Hypo1.get() );
   u_Rule7->AssociateHypo( u_Hypo7.get() );
   u_Rule7->AssociateHypo( u_Hypo8.get() );
   u_Rule7->AssociateHypo( u_Hypo9.get() );
   u_Rule7->AssociateHypo( u_Hypo10.get() );
   u_Rule7->AssociateHypo( u_Hypo11.get() );

   u_Rule8->AssociateHypo( u_Hypo1.get() );
   u_Rule8->AssociateHypo( u_Hypo7.get() );
   u_Rule8->AssociateHypo( u_Hypo8.get() );

   u_Rule9->AssociateHypo( u_Hypo1.get() );
   u_Rule9->AssociateHypo( u_Hypo3.get() );
   u_Rule9->AssociateHypo( u_Hypo4.get() );
   u_Rule9->AssociateHypo( u_Hypo6.get() );

   u_Rule10->AssociateHypo( u_Hypo1.get() );
   u_Rule10->AssociateHypo( u_Hypo6.get() );
   u_Rule10->AssociateHypo( u_Hypo7.get() );
   u_Rule10->AssociateHypo( u_Hypo12.get() );
   u_Rule10->AssociateHypo( u_Hypo13.get() );
   u_Rule10->AssociateHypo( u_Hypo14.get() );

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
// Associate Evids to Hypos : u_Hypo->AssociateEvid( p_Evid, direct?, invertible? );

   u_Hypo1->AssociateEvid( u_Evid1.get(), true, true );
   u_Hypo2->AssociateEvid( u_Evid2.get(), false, true );
   u_Hypo3->AssociateEvid( u_Evid3.get(), false, true );
   u_Hypo4->AssociateEvid( u_Evid4.get(), false, true );
   u_Hypo5->AssociateEvid( u_Evid5.get(), false, true );
   u_Hypo6->AssociateEvid( u_Evid6.get(), false, true );
   u_Hypo7->AssociateEvid( u_Evid7.get(), false, true );
   u_Hypo8->AssociateEvid( u_Evid8.get(), false, true );
   u_Hypo9->AssociateEvid( u_Evid9.get(), false, true );
   u_Hypo10->AssociateEvid( u_Evid10.get(), false, true );
   u_Hypo11->AssociateEvid( u_Evid11.get(), false, true );
   u_Hypo12->AssociateEvid( u_Evid12.get(), false, true );
   u_Hypo13->AssociateEvid( u_Evid13.get(), false, true );
   u_Hypo14->AssociateEvid( u_Evid14.get(), false, true );
   u_Hypo15->AssociateEvid( u_Evid15.get(), false, true );

// !!! DON'T FORGET ME: !!!

   u_RuleKit->FinalizeRuleKitAndBuildKbase();

//======================================================================================================/
// Instantiate GUI Features


   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat01 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   1,
                                                   *(u_Bzo->u_Fact),
                                                   ([&]()->bool { return u_Bzo->u_Fact->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat02 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   2,
                                                   *u_inputsSteady,
                                                   ([&]()->bool { return u_inputsSteady->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat03 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   3,
                                                   *u_TazInBand,
                                                   ([&]()->bool { return u_TazInBand->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat04 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   4,
                                                   *u_Taz_GT_setptClg,
                                                   ([&]()->bool { return u_Taz_GT_setptClg->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat05 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   5,
                                                   *u_Taz_LT_setptHtg,
                                                   ([&]()->bool { return u_Taz_LT_setptHtg->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat06 = std::make_unique<CFeatureFact>(   *u_Subject,
                                                    viewRef,
                                                    6,
                                                    *u_unitReheating,
                                                    ([&]()->bool { return u_unitReheating->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat07 = std::make_unique<CFeatureFact>(  *u_Subject,
                                                   viewRef,
                                                   7,
                                                   *u_unitCoolingZone,
                                                   ([&]()->bool { return u_unitCoolingZone->Now(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat08 =  std::make_unique<CFeatureAnalog>(
                     *u_Subject,
                     viewRef,
                     8,
                     *u_TazSetptClg,
                     ([&]()->float { return u_TazSetptClg->u_Rain->NowY(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat09 = std::make_unique<CFeatureAnalog>(  *u_Subject,
                                                   viewRef,
                                                   9,
                                                   *u_Taz,
                                                   ([&]()->float { return u_Taz->u_Rain->NowY(); }) );

   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
   u_GuiFeat10 =  std::make_unique<CFeatureAnalog>(
                     *u_Subject,
                     viewRef,
                     10,
                     *u_TazSetptHtg,
                     ([&]()->float { return u_TazSetptHtg->u_Rain->NowY(); }) );

}  // End of Constructor

//=======================================================================================================/
// Destructor


CTool_vav_ibal::~CTool_vav_ibal( void ) { /* Emtpy d-tor*/ }


// End VAV Box implementataion
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Begin Application implementation

CApplication::CApplication( void )
                  :  u_Domain( std::make_unique<CDomain>( ERealName::Domain_ibal ) ),
                     u_Clock( std::make_unique<CClockPerPort>( FIXED_CLOCK_SECSPERBELL )
                     ),
                     u_Agent( std::make_unique<CAgent>( *u_Clock ) ),
                     u_Ctrlr( std::make_unique<CController>( *u_Clock )
                     ),
                     u_Seq0( std::make_unique<CSequence>(   *u_Agent,
                                                            *u_Clock )
                     ),
                     u_View( std::make_unique<CView>( *u_Domain,
                                                      *u_Ctrlr )
                     ),
                     u_OmniPort( std::make_unique<CPortOmni>(  *u_Ctrlr,
                                                               *u_View )
                     ),
                     u_EachToolInApp(0) {


   u_EachToolInApp.push_back(

      std::make_unique<CTool_ahu_ibal>(  *u_Domain,  
                                       EDataLabel::Subject_ahu_singleDuct_vavReheat,
                                       ERealName::Subject_ahu1,
                                       ERealName::Subject_chwPlant,
                                       ERealName::Subject_hwPlant,
                                       *u_Clock,
                                       *u_Seq0,
                                       *u_Ctrlr,
                                       *u_View,
                                       *u_OmniPort
      )
   );  // end pushback call

   u_EachToolInApp.push_back(

      std::make_unique<CTool_ahu_ibal>(  *u_Domain,  
                                       EDataLabel::Subject_ahu_singleDuct_vavReheat,
                                       ERealName::Subject_ahu2,
                                       ERealName::Subject_chwPlant,
                                       ERealName::Subject_hwPlant,
                                       *u_Clock,
                                       *u_Seq0,
                                       *u_Ctrlr,
                                       *u_View,
                                       *u_OmniPort
      )
   );  // end pushback call

   u_EachToolInApp.push_back(

      std::make_unique<CTool_vav_ibal>( *u_Domain,  
                                          EDataLabel::Subject_vav_pressIndep_hwReheat,
                                          ERealName::Subject_vav1,
                                          ERealName::Subject_ahu2,
                                          ERealName::Subject_hwPlant,
                                          *u_Clock,
                                          *u_Seq0,
                                          *u_Ctrlr,
                                          *u_View,
                                          *u_OmniPort
      )
   );

   u_EachToolInApp.push_back(

      std::make_unique<CTool_vav_ibal>( *u_Domain,  
                                          EDataLabel::Subject_vav_pressIndep_hwReheat,
                                          ERealName::Subject_vav2,
                                          ERealName::Subject_ahu2,
                                          ERealName::Subject_hwPlant,
                                          *u_Clock,
                                          *u_Seq0,
                                          *u_Ctrlr,
                                          *u_View,
                                          *u_OmniPort
      )
   );
   
   u_EachToolInApp.push_back(

      std::make_unique<CTool_vav_ibal>( *u_Domain,  
                                          EDataLabel::Subject_vav_pressIndep_hwReheat,
                                          ERealName::Subject_vav3,
                                          ERealName::Subject_ahu1,
                                          ERealName::Subject_hwPlant,
                                          *u_Clock,
                                          *u_Seq0,
                                          *u_Ctrlr,
                                          *u_View,
                                          *u_OmniPort
      )
   );

   u_EachToolInApp.push_back(

      std::make_unique<CTool_vav_ibal>( *u_Domain,  
                                          EDataLabel::Subject_vav_pressIndep_hwReheat,
                                          ERealName::Subject_vav4,
                                          ERealName::Subject_ahu1,
                                          ERealName::Subject_hwPlant,
                                          *u_Clock,
                                          *u_Seq0,
                                          *u_Ctrlr,
                                          *u_View,
                                          *u_OmniPort
      )
   );

}   // End CApplication constructor

CApplication::~CApplication( void ) {

   u_EachToolInApp.clear();
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ