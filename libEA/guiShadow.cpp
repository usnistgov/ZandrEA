// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements an abstract "integrating" class that "shadows" front-end GUI elements in the back end (the
   GUI's information source).  Concrete classes specific to each type of GUI element are derived from it. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "guiShadow.hpp"
#include <iostream>
#include <iomanip>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Static fields for IGuiShadow

unsigned long long   IGuiShadow::nextFreshKeySeedValue = 1;  // First issued = 1, 0 = null

#ifdef SWBSWBSWB
// Clang/G++ don't like this for some non-obvious reason.
// I'm not sure but it doesn't appear that this is even needed, so
// I'm trying without it for now.
std::ostringstream   IGuiShadow::intStreamer;
bool                 IGuiShadow::streamerConfigured = false;
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of IGuiShadow

IGuiShadow::IGuiShadow( EApiType arg0 )      // c-tor called when owner is not a LUT
                        :  ownGuiKey ( GenerateNewGuiKey() ),
                           ownApiType (arg0) {

#ifdef SWBSWBSWB
// Clang/G++ don't like this for some non-obvious reason.
// I'm not sure but it doesn't appear that this is even needed, so
// I'm trying without it for now.
   if ( ! streamerConfigured ) {
      IGuiShadow::intStreamer.fill('0');     // all std::iomanip actors are 'sticky' except setw(n)
      IGuiShadow::streamerConfigured = true;
   }
#endif

}

IGuiShadow::~IGuiShadow( void ) { };

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_DATARANGE \
   { \
      { EDataRange::Undefined, {{ NaNFLOAT, NaNFLOAT }} }, \
      { EDataRange::Analog_percent, {{ 0.0f, 100.0f }} }, \
      { EDataRange::Analog_zeroToOne, {{ 0.0f, 1.0f }} }, \
      { EDataRange::Analog_zeroTo3, {{ 0.0f, 3.0f }} }, \
      { EDataRange::Analog_zeroTo120, {{ 0.0f, 120.0f }} }, \
      { EDataRange::Analog_zeroTo3k, {{ 0.0f, 3000.0f }} }, \
      { EDataRange::Bindex_fact, {{ NaNFLOAT, NaNFLOAT }} }, \
      { EDataRange::Bindex_rule, {{ NaNFLOAT, NaNFLOAT }} }, \
      { EDataRange::Boolean, {{ 0.0f, 1.0f }} }, \
      { EDataRange::None, {{ NaNFLOAT, NaNFLOAT }} } \
   }

std::array<float,2> IGuiShadow::LookUpMinMax( EDataRange key ) {

   static DataRangeTable_t      pairLookup_DataRange( INIT_DATARANGE );

   if ( pairLookup_DataRange.count( key ) == 0 ) {
      throw std::logic_error( "Attempted untabulated range code" ); // deliberately no catch
   }
   return pairLookup_DataRange[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

/* EDataLabel might need to be seen on GUI in either of two forms, "tag" or "text":
   "LookUpTag( EDataLabel )" = compact form, fits better to features and plots, but uses abbreviations.
   "LookUpText( EDataLabel)" = verbose form, fully describes labeled object w/o using abbreviations.
   Only the "tags" appear needed currently.
*/

#define INIT_DATATAG \
   { \
      { EDataLabel::Undefined,                                       "?????" }, \
      { EDataLabel::Case_alertToGui,                                 "FAULT PROBABLE: "}, \
      { EDataLabel::Fact_antecedent_vav_ahuOkay,                     "ahuOutputOK" }, \
      { EDataLabel::Fact_antecedent_vav_hwPlantOkay,                 "hwPlantOK" }, \
      { EDataLabel::Fact_direct_Bso,                                 "sysOcc" }, \
      { EDataLabel::Fact_direct_Bzo,                                 "zoneOcc" }, \
      { EDataLabel::Fact_data_PsaiSteady,                            "PsaiSteady" }, \
      { EDataLabel::Fact_data_PsasSteady,                            "PsasSteady" }, \
      { EDataLabel::Fact_data_QadHunting,                            "QasHunts" }, \
      { EDataLabel::Fact_data_QadSetptSteady,                        "QadSetSteady" }, \
      { EDataLabel::Fact_data_QadSteady,                             "QadSteady" }, \
      { EDataLabel::Fact_data_QadTrackingHigh,                       "QadTrackHigh" }, \
      { EDataLabel::Fact_data_QadTrackingLow,                        "QadTrackLow" }, \
      { EDataLabel::Fact_data_QasHunting,                            "QasHunts" }, \
      { EDataLabel::Fact_data_QasSetptSteady,                        "QasSetSteady" }, \
      { EDataLabel::Fact_data_QasSteady,                             "QasSteady" }, \
      { EDataLabel::Fact_data_QasTrackingHigh,                       "QasTrackHigh" }, \
      { EDataLabel::Fact_data_QasTrackingLow,                        "QasTrackLow" }, \
      { EDataLabel::Fact_data_Tad_GT_Tai,                            "Tad>Tai" }, \
      { EDataLabel::Fact_data_Tad_GT_Taz,                            "Tad>Taz" }, \
      { EDataLabel::Fact_data_Tad_LT_Taz,                            "Tad<Taz" }, \
      { EDataLabel::Fact_data_TadHunting,                            "TadHunts" }, \
      { EDataLabel::Fact_data_TadSteady,                             "TadSteady" }, \
      { EDataLabel::Fact_data_Tam_EQ_Tao,                            "Tam=Tao" }, \
      { EDataLabel::Fact_data_Tam_GTE_minTaoTar,                     "Tam>=minTaoTar" }, \
      { EDataLabel::Fact_data_Tam_GT_TasSetpt,                       "Tam>TasSetpt" }, \
      { EDataLabel::Fact_data_Tam_LTE_maxTaoTar,                     "Tam<maxTaoTar" }, \
      { EDataLabel::Fact_data_Tao_LT_Tar,                            "Tao<Tar" }, \
      { EDataLabel::Fact_data_Tao_LT_TasSetpt,                       "Tao<TasSetpt" }, \
      { EDataLabel::Fact_data_Tas_EQ_setpt,                          "Tas=Setpt" }, \
      { EDataLabel::Fact_data_Tas_GT_Tam,                            "Tas>Tam" }, \
      { EDataLabel::Fact_data_Tas_LT_Tam,                            "Tas<Tam" }, \
      { EDataLabel::Fact_data_Tas_LT_Tar,                            "Tas<Tar" }, \
      { EDataLabel::Fact_data_TasFalling,                            "TasFall" }, \
      { EDataLabel::Fact_data_TasHunting,                            "TasHunts" }, \
      { EDataLabel::Fact_data_TasRising,                             "TasRise" }, \
      { EDataLabel::Fact_data_TasSetptSteady,                        "TasSetSteady" }, \
      { EDataLabel::Fact_data_TasSteady,                             "TasSteady" }, \
      { EDataLabel::Fact_data_TasTrackingHigh,                       "TasTrackHigh" }, \
      { EDataLabel::Fact_data_TasTrackingLow,                        "TasTrackLow" }, \
      { EDataLabel::Fact_data_Taz_GT_setptClg,                       "Taz>band" }, \
      { EDataLabel::Fact_data_Taz_LT_setptHtg,                       "Taz<band" }, \
      { EDataLabel::Fact_data_TazSetptClgSteady,                     "TazSetClgSteady" }, \
      { EDataLabel::Fact_data_TazSetptHtgSteady,                     "TazSetHtgSteady" }, \
      { EDataLabel::Fact_data_TazSteady,                             "TazSteady" }, \
      { EDataLabel::Fact_data_TazTrackingHigh,                       "TazTrackHigh" }, \
      { EDataLabel::Fact_data_TazTrackingLow,                        "TazTrackLow" }, \
      { EDataLabel::Fact_data_UddSteady,                             "UddSteady" }, \
      { EDataLabel::Fact_data_UdmSteady,                             "UdmSteady" }, \
      { EDataLabel::Fact_data_UvcHunting,                            "UvcHunt" }, \
      { EDataLabel::Fact_data_UvcSteady,                             "UvcSteady" }, \
      { EDataLabel::Fact_data_UvhSteady,                             "UvhSteady" }, \
      { EDataLabel::Fact_data_ZddSteady,                             "ZddSteady" }, \
      { EDataLabel::Fact_data_ZdmSteady,                             "ZdmSteady" }, \
      { EDataLabel::Fact_data_ZvcHunting,                            "ZvcHunt" }, \
      { EDataLabel::Fact_data_ZvcSteady,                             "ZvcSteady" }, \
      { EDataLabel::Fact_data_ZvhSteady,                             "ZvhSteady" }, \
      { EDataLabel::Fact_para_absDifTarTao_GTE_10F,                  "|Tao-Tar|>10F" }, \
      { EDataLabel::Fact_para_fracOA_EQ_min,                         "atMinOA" }, \
      { EDataLabel::Fact_para_fracOA_GTE_min,                        ">=MinOA" }, \
      { EDataLabel::Fact_para_PsasZero,                              "PsasZero" }, \
      { EDataLabel::Fact_para_Tam_GT_frzStat,                        "Tam>frzStat" }, \
      { EDataLabel::Fact_para_QasZero,                               "QasZero" }, \
      { EDataLabel::Fact_para_QadZero,                               "QadZero" }, \
      { EDataLabel::Fact_para_UddFull,                               "UddFull" }, \
      { EDataLabel::Fact_para_UdmFullOA,                             "UdmFullOA" }, \
      { EDataLabel::Fact_para_UdmFullRA,                             "UdmFullRA" }, \
      { EDataLabel::Fact_para_UvcShut,                               "UvcShut" }, \
      { EDataLabel::Fact_para_UvhShut,                               "UvhShut" }, \
      { EDataLabel::Fact_para_ZddFull,                               "ZddFull" }, \
      { EDataLabel::Fact_para_ZdmFullOA,                             "ZdmFullOA" }, \
      { EDataLabel::Fact_para_ZdmFullRA,                             "ZdmFullRA" }, \
      { EDataLabel::Fact_para_ZvcShut,                               "ZvcShut" }, \
      { EDataLabel::Fact_para_ZvhShut,                               "ZvhShut" }, \
      { EDataLabel::Fact_subj_ahu_chwCoolingAir,                     "chwClgAir" }, \
      { EDataLabel::Fact_subj_ahu_chwHelpingEcon,                    "chwHelpingEcon" }, \
      { EDataLabel::Fact_subj_ahu_chwNeeded,                         "chwNeed" }, \
      { EDataLabel::Fact_subj_ahu_econActive,                        "econActive" }, \
      { EDataLabel::Fact_subj_ahu_econAtMax,                         "econAtMax" }, \
      { EDataLabel::Fact_subj_ahu_econExpected,                      "econExpected" }, \
      { EDataLabel::Fact_subj_ahu_inputsSteady,                      "inputsSteady" }, \
      { EDataLabel::Fact_subj_ahu_okayOnEconAlone,                   "okayOnEconAlone" }, \
      { EDataLabel::Fact_subj_ahu_okayOnEconPlusChw,                 "okayOnEconPlusChw" }, \
      { EDataLabel::Fact_subj_ahu_outputLevel,                       "outputLevel" }, \
      { EDataLabel::Fact_subj_ahu_preheatNeeded,                     "preheatNeeded" }, \
      { EDataLabel::Fact_subj_ahu_unitOn,                            "unitOn" }, \
      { EDataLabel::Fact_subj_ahu_unitPreheating,                    "preheat" }, \
      { EDataLabel::Fact_subj_vav_inputsSteady,                      "inputsSteady" }, \
      { EDataLabel::Fact_subj_vav_TazInBand,                         "TazInBand" }, \
      { EDataLabel::Fact_subj_vav_unitCoolingZone,                   "clgZone" }, \
      { EDataLabel::Fact_subj_vav_unitHeatingZone,                   "htgZone" }, \
      { EDataLabel::Fact_subj_vav_unitReheating,                     "reheating" }, \
      { EDataLabel::Fact_sustained_Bso,                              "BsoSus" }, \
      { EDataLabel::Fact_sustained_Bzo,                              "BzoSus" }, \
      { EDataLabel::Formula_absDif_TarTao,                           "absDifTaoTar"}, \
      { EDataLabel::Formula_fraction_OA_tempProxy,                   "fracOA_T"}, \
      { EDataLabel::Formula_maxTaoTar,                               "maxTaoTar"}, \
      { EDataLabel::Formula_minTaoTar,                               "minTaoTar"}, \
      { EDataLabel::Knob_chartShew_secsDataUsing,                    "Shewhart: secsUsing"}, \
      { EDataLabel::Knob_chartShew_tripFreeMargin,                   "Shewhart: tfm"}, \
      { EDataLabel::Knob_chartShew_zPass,                            "Shewhart: zPass"}, \
      { EDataLabel::Knob_chartTrax_appsBtwnResets,                   "Tracking: appsBtwnResets"}, \
      { EDataLabel::Knob_chartTrax_halfBand,                         "Tracking: halfBand"}, \
      { EDataLabel::Knob_chartTrax_lagFraction,                      "Tracking: lagFrac"}, \
      { EDataLabel::Knob_chartTrax_actionSum,                        "Tracking: actionSum"}, \
      { EDataLabel::Knob_chartTrax_staleFraction,                    "Tracking: staleFrac"}, \
      { EDataLabel::Knob_factRelate_hyster,                          "hyster"}, \
      { EDataLabel::Knob_factRelate_slack,                           "slack ( + eases reln)"}, \
      { EDataLabel::Knob_factSustained_minCycles,                    "minCycles"}, \
      { EDataLabel::Knob_krono_setLookback,                          "time span"}, \
      { EDataLabel::Knob_rule_idleMode,                              "Idle this rule?"}, \
      { EDataLabel::Knob_ruleKit_idleModeAll,                        "Idle all rules in kit?"}, \
      { EDataLabel::Knob_ruleKit_loadRtKrono_logicOfOneRule,         "See r-t logic of Rule #_ ?"}, \
      { EDataLabel::Knob_ruleKit_loadRtKrono_resultsOfAllRules,      "See r-t results of all Rules?"}, \
      { EDataLabel::Point_binary_system_occupied,                    "Bso" }, \
      { EDataLabel::Point_binary_zone_occupied,                      "Bzo" }, \
      { EDataLabel::Point_command_damper_disch,                      "Udd" }, \
      { EDataLabel::Point_command_damper_mixing,                     "Udm" }, \
      { EDataLabel::Point_command_damper_outside,                    "Udo" }, \
      { EDataLabel::Point_command_valve_chw,                         "Uvc" }, \
      { EDataLabel::Point_command_valve_hw,                          "Uvh" }, \
      { EDataLabel::Point_command_fan_speed,                         "Ufs" }, \
      { EDataLabel::Point_flowVolume_air_supply,                     "Qas" }, \
      { EDataLabel::Point_flowVolume_air_supply_setpt,               "QasSet" }, \
      { EDataLabel::Point_flowVolume_air_disch,                      "Qad" }, \
      { EDataLabel::Point_flowVolume_air_disch_setpt,                "QadSet" }, \
      { EDataLabel::Point_position_damper_disch,                     "Zdd" }, \
      { EDataLabel::Point_position_damper_mixing,                    "Zdm" }, \
      { EDataLabel::Point_position_damper_outside,                   "Zdo" }, \
      { EDataLabel::Point_position_valve_chw,                        "Zvc" }, \
      { EDataLabel::Point_position_valve_hw,                         "Zvh" }, \
      { EDataLabel::Point_power_electric_fan,                        "Wef" }, \
      { EDataLabel::Point_power_electric_preheat,                    "Wep" }, \
      { EDataLabel::Point_power_electric_reheat,                     "Wer" }, \
      { EDataLabel::Point_pressure_static_air_inlet,                 "Psai" }, \
      { EDataLabel::Point_pressure_static_air_supply,                "Psas" }, \
      { EDataLabel::Point_temperature_air_coldDeck,                  "Tac" }, \
      { EDataLabel::Point_temperature_air_discharge,                 "Tad" }, \
      { EDataLabel::Point_temperature_air_hotDeck,                   "Tah" }, \
      { EDataLabel::Point_temperature_air_inlet,                     "Tai" }, \
      { EDataLabel::Point_temperature_air_lvgPreheat,                "Tap" }, \
      { EDataLabel::Point_temperature_air_mixed,                     "Tam" }, \
      { EDataLabel::Point_temperature_air_outside,                   "Tao" }, \
      { EDataLabel::Point_temperature_air_return,                    "Tar" }, \
      { EDataLabel::Point_temperature_air_supply,                    "Tas" }, \
      { EDataLabel::Point_temperature_air_supply_setpt,              "TasSet" }, \
      { EDataLabel::Point_temperature_air_zone,                      "Taz" }, \
      { EDataLabel::Point_temperature_air_zone_setpt_clg,            "TazSetClg" }, \
      { EDataLabel::Point_temperature_air_zone_setpt_htg,            "TazSetHtg" }, \
      { EDataLabel::Point_timeDate_local,                            "Date and time (local)" }, \
      { EDataLabel::RuleKit,                                         "Rule kit" }, \
      { EDataLabel::Subject_vav_pressIndep_hwReheat,  "VAV unit, press. indep., HW reheat" }, \
      { EDataLabel::Subject_ahu_singleDuct_vavReheat, "AHU, single duct, VAV reheat" } \
   }

std::string IGuiShadow::LookUpTag( EDataLabel key ) { 

   static DataTagTable_t        tagLookup_DataLabel( INIT_DATATAG );

   if (tagLookup_DataLabel.count( key ) == 0) {
      throw std::logic_error("Attempted untabulated data tag code"); // deliberately no catch
   }
   return tagLookup_DataLabel[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_ALERTMSG \
   { \
      { EAlertMsg::Undefined,                "" }, \
      { EAlertMsg::Common_givenBadKey,       "Given bad/expired key" }, \
      { EAlertMsg::Krono_givenSecsToShowOor, "Krono given time span out of range allowed" }, \
      { EAlertMsg::PointAnalog_inputSameDbl, "Input is unchanged double" }, \
      { EAlertMsg::RainAnalog_binOverUnder,  "Input outside binned range" }, \
      { EAlertMsg::Rule_ahuSdvr_1_focus,     "Tas regulation" }, \
      { EAlertMsg::Rule_ahuSdvr_1_if,        "IF: unitOn & inputsSteady & TasSteady & NOT(econExpected)" }, \
      { EAlertMsg::Rule_ahuSdvr_1_then,      "THEN: TasOnSetpt" }, \
      { EAlertMsg::Rule_ahuSdvr_1_onFail,    "Tas not on setpt during mech clg" }, \
      { EAlertMsg::Rule_ahuSdvr_2_focus,     "Tas regulation" }, \
      { EAlertMsg::Rule_ahuSdvr_2_if,        "IF: unit on & inputsSteady & NOT(econExpected)" }, \
      { EAlertMsg::Rule_ahuSdvr_2_then,      "THEN: NOT( TasTrackingHigh | Low)" }, \
      { EAlertMsg::Rule_ahuSdvr_2_onFail,    "Tas drifting off setpt during mech clg" }, \
      { EAlertMsg::Rule_ahuSdvr_3_focus,     "Tas regulation" }, \
      { EAlertMsg::Rule_ahuSdvr_3_if,        "IF: unitOn & inputsSteady & TasSteady & econExpected" }, \
      { EAlertMsg::Rule_ahuSdvr_3_then,      "THEN: TasOnSetpt" }, \
      { EAlertMsg::Rule_ahuSdvr_3_onFail,    "Tas not on setpt during economizer" }, \
      { EAlertMsg::Rule_ahuSdvr_4_focus,     "Tas regulation" }, \
      { EAlertMsg::Rule_ahuSdvr_4_if,        "IF: unit on & inputsSteady & econExpected" }, \
      { EAlertMsg::Rule_ahuSdvr_4_then,      "THEN: NOT( TasTrackingHigh | Low)" }, \
      { EAlertMsg::Rule_ahuSdvr_4_onFail,    "Tas drifting off setpt during economizer" }, \
      { EAlertMsg::Rule_ahuSdvr_5_focus,     "CHW" }, \
      { EAlertMsg::Rule_ahuSdvr_5_if,        "IF: unitOn & UvcSteady & UvcShut" }, \
      { EAlertMsg::Rule_ahuSdvr_5_then,      "THEN: NOT(chwCoolingAir)" }, \
      { EAlertMsg::Rule_ahuSdvr_5_onFail,    "Cooling across CHW coil despite valve shut" }, \
      { EAlertMsg::Rule_ahuSdvr_6_focus,     "economizer" }, \
      { EAlertMsg::Rule_ahuSdvr_6_if,        "IF: econExpected" }, \
      { EAlertMsg::Rule_ahuSdvr_6_then,      "THEN: econActive" }, \
      { EAlertMsg::Rule_ahuSdvr_6_onFail,    "Economizer not working" }, \
      { EAlertMsg::Rule_ahuSdvr_7_focus,     "CHW" }, \
      { EAlertMsg::Rule_ahuSdvr_7_if,        "IF: chwNeeded" }, \
      { EAlertMsg::Rule_ahuSdvr_7_then,      "THEN: chwCoolingAir" }, \
      { EAlertMsg::Rule_ahuSdvr_7_onFail,    "Mechanical cooling needed but not working" }, \
      { EAlertMsg::Rule_ahuSdvr_8_focus,     "preheat" }, \
      { EAlertMsg::Rule_ahuSdvr_8_if,        "IF: unitOn" }, \
      { EAlertMsg::Rule_ahuSdvr_8_then,      "THEN: Tam > freezeStat" }, \
      { EAlertMsg::Rule_ahuSdvr_8_onFail,    "Preheat not working" }, \
      { EAlertMsg::Rule_ahuSdvr_9_focus,     "sensors" }, \
      { EAlertMsg::Rule_ahuSdvr_9_if,        "IF: unitOn" }, \
      { EAlertMsg::Rule_ahuSdvr_9_then,      "THEN: Tam bounded by Toa and Tar" }, \
      { EAlertMsg::Rule_ahuSdvr_9_onFail,    "Temp readings at mix box not logical" }, \
      { EAlertMsg::Rule_ahuSdvr_10_focus,     "economizer" }, \
      { EAlertMsg::Rule_ahuSdvr_10_if,       "IF: |Tar - Tao| > 10 F & NOT(econExpected)" }, \
      { EAlertMsg::Rule_ahuSdvr_10_then,     "THEN: fracOA at minOA" }, \
      { EAlertMsg::Rule_ahuSdvr_10_onFail,   "Mixed air has improper OA fraction" }, \
      { EAlertMsg::Rule_vav_1_focus,         "airflow regulation" }, \
      { EAlertMsg::Rule_vav_1_if,            "IF: ahuOutputOkay & inputsSteady" }, \
      { EAlertMsg::Rule_vav_1_then,          "THEN: NOT(QadTrackingHigh OR QaTrackingLow)" }, \
      { EAlertMsg::Rule_vav_1_onFail,        "Airflow not following value commanded" }, \
      { EAlertMsg::Rule_vav_2_focus,         "airflow regulation" }, \
      { EAlertMsg::Rule_vav_2_if,            "IF: ahuOutputOkay & inputsSteady" }, \
      { EAlertMsg::Rule_vav_2_then,          "THEN: NOT(QadHunting)" }, \
      { EAlertMsg::Rule_vav_2_onFail,        "Airflow hunting" }, \
      { EAlertMsg::Rule_vav_3_focus,         "cooling action" }, \
      { EAlertMsg::Rule_vav_3_if,            "IF: ahuOutputOkay & inputsSteady & (Taz > TazSetClg)" }, \
      { EAlertMsg::Rule_vav_3_then,          "THEN: unitCoolingZone" }, \
      { EAlertMsg::Rule_vav_3_onFail,        "Zone too warm yet unit not cooling" }, \
      { EAlertMsg::Rule_vav_4_focus,         "cooling action" }, \
      { EAlertMsg::Rule_vav_4_if,            "IF: inputsSteady & unitClgZone & UddFull" }, \
      { EAlertMsg::Rule_vav_4_then,          "THEN: NOT(TazTrackingHigh)" }, \
      { EAlertMsg::Rule_vav_4_onFail,        "Zone temp tracking high despite unit at max cooling" }, \
      { EAlertMsg::Rule_vav_5_focus,         "cooling action" }, \
      { EAlertMsg::Rule_vav_5_if,            "IF: inputsSteady & unitCoolingZone & NOT( UddFull )" }, \
      { EAlertMsg::Rule_vav_5_then,          "THEN: NOT(TazTrackingHigh)" }, \
      { EAlertMsg::Rule_vav_5_onFail,        "Zone temp tracking high yet unit not responding with max cooling" }, \
      { EAlertMsg::Rule_vav_6_focus,         "reheat action" }, \
      { EAlertMsg::Rule_vav_6_if,            "IF: inputsSteady & ( Taz < TazSetHtg)" }, \
      { EAlertMsg::Rule_vav_6_then,          "THEN: unitReheating" }, \
      { EAlertMsg::Rule_vav_6_onFail,        "Zone too cool yet unit not reheating" }, \
      { EAlertMsg::Rule_vav_7_focus,         "reheat action" }, \
      { EAlertMsg::Rule_vav_7_if,            "IF: inputsSteady & unitReheating & UddFull" }, \
      { EAlertMsg::Rule_vav_7_then,          "THEN: NOT(TazTrackingLow)" }, \
      { EAlertMsg::Rule_vav_7_onFail,        "Zone temp tracking low despite unit at max reheat" }, \
      { EAlertMsg::Rule_vav_8_focus,         "reheat action" }, \
      { EAlertMsg::Rule_vav_8_if,            "IF: inputsSteady & unitReheating & NOT( UddFull )" }, \
      { EAlertMsg::Rule_vav_8_then,          "THEN: NOT(TazTrackingLow)" }, \
      { EAlertMsg::Rule_vav_8_onFail,        "Zone temp tracking low yet unit not responding with max reheat" }, \
      { EAlertMsg::Rule_vav_9_focus,         "unit ctrl" }, \
      { EAlertMsg::Rule_vav_9_if,            "IF: inputsSteady & UvhShut & NOT(QadZero)" }, \
      { EAlertMsg::Rule_vav_9_then,          "THEN: NOT( Tad > Tai )" }, \
      { EAlertMsg::Rule_vav_9_onFail,        "Air temp rise across unit despite reheat valve shut" }, \
      { EAlertMsg::Rule_vav_10_focus,        "zone regulation" }, \
      { EAlertMsg::Rule_vav_10_if,           "IF: ahuOutputOkay & inputsSteady & NOT(QadZero)" }, \
      { EAlertMsg::Rule_vav_10_then,         "THEN: Taz in band" }, \
      { EAlertMsg::Rule_vav_10_onFail,       "Zone temp not in setpoint band" }, \
      { EAlertMsg::RuleKit_caseCountAtMax,   "Queue of undisposed cases at limit. No new cases will be created." } \
   }

std::string IGuiShadow::LookUpText( EAlertMsg key) {

   static AlertMsgTable_t      textLookup_AlertMsg( INIT_ALERTMSG );

   if (textLookup_AlertMsg.count(key) == 0) {
      throw std::logic_error("Attempted untabulated alert msg"); // deliberately no catch
   }
   return textLookup_AlertMsg[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_DATASUFFIX \
   { \
      { EDataSuffix::Undefined,          "" }, \
      { EDataSuffix::AboutMean,          "+/- mean" }, \
      { EDataSuffix::ConsecutiveNoTrip,  " w/o trip" }, \
      { EDataSuffix::LimitHigh,          " High Limit" }, \
      { EDataSuffix::LimitLow,           " Low Limit" }, \
      { EDataSuffix::MaxSeen,            " Max Seen" }, \
      { EDataSuffix::MinSeen,            " Min Seen" }, \
      { EDataSuffix::MeanOverSecs,       "s_Mean" }, \
      { EDataSuffix::None,               "" }, \
      { EDataSuffix::StdDevOverSecs,     "s_SD" }, \
      { EDataSuffix::TimesTotal,         " X total" }, \
      { EDataSuffix::UnitHours,          "-hrs" } \
   }

std::string IGuiShadow::LookUpText( EDataSuffix key ) {

   static DataSuffixTable_t      textLookup_DataSuffix( INIT_DATASUFFIX );

   if ( textLookup_DataSuffix.count( key ) == 0 ) {
      throw std::logic_error( "Attempted untabulated qualifier code" ); // deliberately no catch
   }
   return textLookup_DataSuffix[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_DATAUNIT \
   { \
      { EDataUnit::Undefined,                "?" }, \
      { EDataUnit::Area_sqFt,                "sqFt" }, \
      { EDataUnit::Binary_Boolean,           "F/T" }, \
      { EDataUnit::Bindex_fact,              "" }, \
      { EDataUnit::Bindex_rule,              "" }, \
      { EDataUnit::Coefficient,              "" }, \
      { EDataUnit::Count_sum,                "" }, \
      { EDataUnit::Count_cycle,              "cycles" }, \
      { EDataUnit::Distance_feet,            "ft" }, \
      { EDataUnit::Distance_inch,            "in" }, \
      { EDataUnit::Distance_stdDev,          "stdDevs" }, \
      { EDataUnit::Energy_kilowattHr,        "kWh" }, \
      { EDataUnit::FlowVolume_cfm,           "cfm" }, \
      { EDataUnit::FlowVolume_gpm,           "gpm" }, \
      { EDataUnit::Frequency_cph,            "cph" }, \
      { EDataUnit::Identifier_ruleUai,       "Rule UAI" }, \
      { EDataUnit::Index,                    "" }, \
      { EDataUnit::None,                     "" }, \
      { EDataUnit::Power_kiloWatt,           "kW" }, \
      { EDataUnit::PressureAbso_psia,        "psia" }, \
      { EDataUnit::PressureDiff_psid,        "psid" }, \
      { EDataUnit::PressureGage_iwg,         "iwg" }, \
      { EDataUnit::PressureGage_psig,        "psig" }, \
      { EDataUnit::Ratio_fraction,           "/1.0" }, \
      { EDataUnit::Ratio_percent,            "%" }, \
      { EDataUnit::Temperature_degC,         "degC" }, \
      { EDataUnit::Temperature_degF,         "degF" }, \
      { EDataUnit::TemperatureDiff_Cdeg,     "Cdeg" }, \
      { EDataUnit::TemperatureDiff_Fdeg,     "Fdeg" }, \
      { EDataUnit::TimeSpan_hour,            "h" }, \
      { EDataUnit::TimeSpan_sec,             "s" }, \
      { EDataUnit::Velocity_fpm,             "fpm" } \
   }

std::string IGuiShadow::LookUpText( EDataUnit key ) {

   static DataUnitTable_t      textLookup_DataUnit( INIT_DATAUNIT );

   if ( textLookup_DataUnit.count( key ) == 0 ) {
      throw std::logic_error( "Attempted untabulated units code" ); // deliberately no catch
   }
   return textLookup_DataUnit[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_DISKFILE \
   { \
      { ERealName::Undefined,             "undefined_" }, \
      { ERealName::Domain_ibal,           "ibal_" }, \
      { ERealName::Subject_chwPlant,      "chwPlant_" }, \
      { ERealName::Subject_hwPlant,       "hwPlant_" }, \
      { ERealName::Subject_ahu1,          "ahu1_" }, \
      { ERealName::Subject_ahu2,          "ahu2_" }, \
      { ERealName::Subject_vav1,          "vav1_" }, \
      { ERealName::Subject_vav2,          "vav2_" }, \
      { ERealName::Subject_vav3,          "vav3_" }, \
      { ERealName::Subject_vav4,          "vav4_" } \
   }

std::string IGuiShadow::LookUpDiskFile( ERealName key ) {

   static DiskFileTable_t     textLookup_DiskFile( INIT_DISKFILE );

   if ( textLookup_DiskFile.count(key) == 0 ) {
      throw std::logic_error("Attempted untabulated disk filename"); // deliberately no catch
   }
   return textLookup_DiskFile[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_INFOMODE \
   { \
      { EInfoMode::Undefined,                                  "" }, \
      { EInfoMode::Histo_analog_statesOverAllCycles,           "analog state" }, \
      { EInfoMode::Histo_analog_valuesOverValidCycles,         "analog value" }, \
      { EInfoMode::Histo_fact_statesOverAllCycles,             "fact state" }, \
      { EInfoMode::Histo_rule_statesOverAllCycles,             "rule state" }, \
      { EInfoMode::Histo_ruleKit_failsOverTests,               "fails/tests" }, \
      { EInfoMode::Histo_ruleKit_testsOverValidCycles,         "tests/valid cycles" }, \
      { EInfoMode::Histo_ruleKit_validCyclesOverAllCycles,     "valid cycles/all cycles" } \
   }

std::string IGuiShadow::LookUpText( EInfoMode key ) {

   static InfoModeTable_t     textLookup_InfoMode( INIT_INFOMODE );

   if ( textLookup_InfoMode.count(key) == 0 ) {
      throw std::logic_error("Attempted untabulated info mode"); // deliberately no catch
   }
   return textLookup_InfoMode[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_TIMESPAN \
   { \
      { ETimeSpan::Undefined,             "" }, \
      { ETimeSpan::Histo_movingHour,      "moving hour" }, \
      { ETimeSpan::Histo_past24hrs,       "past 24 hrs" }, \
      { ETimeSpan::Histo_past7days,       "past 7 days" } \
   }

std::string IGuiShadow::LookUpText( ETimeSpan key ) {

   static TimeSpanTable_t     textLookup_TimeSpan( INIT_TIMESPAN );

   if ( textLookup_TimeSpan.count(key) == 0 ) {
      throw std::logic_error("Attempted untabulated time span"); // deliberately no catch
   }
   return textLookup_TimeSpan[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_REALNAME \
   { \
      { ERealName::Undefined,             "Undefined" }, \
      { ERealName::Domain_ibal,           "IBAL" }, \
      { ERealName::Subject_chwPlant,      "CHW Plant" }, \
      { ERealName::Subject_hwPlant,       "HW Plant" }, \
      { ERealName::Subject_ahu1,          "AHU-1" }, \
      { ERealName::Subject_ahu2,          "AHU-2" }, \
      { ERealName::Subject_vav1,          "VAV-1" }, \
      { ERealName::Subject_vav2,          "VAV-2" }, \
      { ERealName::Subject_vav3,          "VAV-3" }, \
      { ERealName::Subject_vav4,          "VAV-4" } \
   }

std::string IGuiShadow::LookUpText( ERealName key ) {

   static RealNameTable_t     textLookup_RealName( INIT_REALNAME );

   if ( textLookup_RealName.count(key) == 0 ) {
      throw std::logic_error("Attempted untabulated real name"); // deliberately no catch
   }
   return textLookup_RealName[key];
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

#define INIT_GUITYPE \
   { \
      { EApiType::Undefined,                    EGuiType::Undefined }, \
      { EApiType::Case,                         EGuiType::Case }, \
      { EApiType::Display_ruleKit,              EGuiType::Display_ruleKit }, \
      { EApiType::Domain,                       EGuiType::Domain }, \
      { EApiType::Feature_analog,               EGuiType::Feature_analog }, \
      { EApiType::Feature_fact,                 EGuiType::Feature_fact }, \
      { EApiType::Histogram_analog,             EGuiType::Histogram_analog }, \
      { EApiType::Histogram_fact,               EGuiType::Histogram_fact }, \
      { EApiType::Histogram_rule,               EGuiType::Histogram_rule }, \
      { EApiType::Histogram_ruleKit,            EGuiType::Histogram_ruleKit }, \
      { EApiType::HistogramBars_analogBins,     EGuiType::HistogramBars_analogBins }, \
      { EApiType::HistogramBars_analogStates,   EGuiType::HistogramBars_analogStates }, \
      { EApiType::HistogramBars_factStates,     EGuiType::HistogramBars_factStates }, \
      { EApiType::HistogramBars_ruleStates,     EGuiType::HistogramBars_ruleStates }, \
      { EApiType::HistogramBars_rulesUai,       EGuiType::HistogramBars_rulesUai }, \
      { EApiType::Knob_Boolean,                 EGuiType::Knob_takesGuiFpnAsBoolean }, \
      { EApiType::Knob_float,                   EGuiType::Knob_takesGuiFpnAsFloat }, \
      { EApiType::Knob_selectNzint,             EGuiType::Knob_selectsGuiFpnFromList }, \
      { EApiType::Knob_sint,                    EGuiType::Knob_takesGuiFpnAsInteger }, \
      { EApiType::Krono_realtime,               EGuiType::Krono_realtime }, \
      { EApiType::Krono_snapshot,               EGuiType::Krono_snapshot }, \
      { EApiType::Pane_realtime_analog,         EGuiType::Pane_analog }, \
      { EApiType::Pane_realtime_fact,           EGuiType::Pane_fact }, \
      { EApiType::Pane_realtime_rule,           EGuiType::Pane_rule }, \
      { EApiType::Pane_snapshot_analog,         EGuiType::Pane_analog }, \
      { EApiType::Pane_snapshot_fact,           EGuiType::Pane_fact }, \
      { EApiType::Pane_snapshot_rule,           EGuiType::Pane_rule }, \
      { EApiType::Subject,                      EGuiType::Subject }, \
      { EApiType::Trace_realtime_analog,        EGuiType::Trace_analog }, \
      { EApiType::Trace_realtime_fact,          EGuiType::Trace_fact }, \
      { EApiType::Trace_realtime_rule,          EGuiType::Trace_rule }, \
      { EApiType::Trace_snapshot_analog,        EGuiType::Trace_analog }, \
      { EApiType::Trace_snapshot_fact,          EGuiType::Trace_fact }, \
      { EApiType::Trace_snapshot_rule,          EGuiType::Trace_rule } \
   }

EGuiType IGuiShadow::LookUpGuiType( EApiType key ) {

   static GuiTypeTable_t      typeLookup_GuiType( INIT_GUITYPE );

   if ( typeLookup_GuiType.count( key ) == 0 ) {
      throw std::logic_error( "Attempted untabulated GUI-type lookup" ); // deliberately no catch
   }
   return typeLookup_GuiType[key];
}

//======================================================================================================/
// Public methods

NGuiKey IGuiShadow::SayGuiKey( void ) const { return ownGuiKey; }


EApiType IGuiShadow::SayApiType( void ) const { return ownApiType; }


NGuiKey IGuiShadow::GenerateNewGuiKey( void ) { return NGuiKey( nextFreshKeySeedValue++ ); }

//======================================================================================================/
// Protected methods

std::string IGuiShadow::SayIdentifierAsThreeDigitText( Nzint_t idGiven) {

   // Constrain serial number of each case on a given rule to be a three-digit zero-padded int

   if ( idGiven > 999u ) { return "???"; }

    std::ostringstream serializingStreamInProperFormat;  // streams get used, not assigned ;-)

   serializingStreamInProperFormat  << std::setw(3)
                                    << std::setfill('0')
                                    << std::to_string(idGiven);

   return serializingStreamInProperFormat.str();   
}


void IGuiShadow::WriteTimestampAsTextTo( time_t timestamp, std::string& destinRef ) {

   struct tm* p_timeX;
   p_timeX = localtime( &timestamp );

   std::string localRegstr;
#ifndef SWBSWBSWB
   std::ostringstream intStreamer;

   intStreamer.fill('0');
#endif

   // "Build" and store field (std::string) value using a field (stream object) as a buffer/format tool

   intStreamer.str( std::string() );         // wipe clean the std::ostringstream object
   intStreamer.clear();                      // "clears" error state on std::ostringstream obj
   intStreamer << std::setw(2) << ( (p_timeX->tm_mon) + 1 );   // load up ostringstream obj
   localRegstr = intStreamer.str() + "/";    // copies ostringstream contents into std::string

   intStreamer.str( std::string() );
   intStreamer.clear();
   intStreamer << std::setw(2) << ( p_timeX->tm_mday );
   localRegstr = localRegstr + intStreamer.str() + " ";

   intStreamer.str( std::string() );
   intStreamer.clear();
   intStreamer << std::setw(2) << p_timeX->tm_hour;
   localRegstr = localRegstr + intStreamer.str() + ":";

   intStreamer.str( std::string() );
   intStreamer.clear();
   intStreamer << std::setw(2) << p_timeX->tm_min;
   localRegstr = localRegstr + intStreamer.str();

   destinRef.clear();
   destinRef = localRegstr;

   return;
}

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

std::string IGuiShadow::RenderGuiTag( EDataLabel label, EDataSuffix suffix ) const {

   // Brief "tag" strings abbreviate longer labels of "text" strings where compactness is needed in GUI
   return ( LookUpTag( label ) + LookUpText( suffix ) ); 
}

std::string IGuiShadow::RenderUnits( EDataUnit unit ) const { return LookUpText( unit ); }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
