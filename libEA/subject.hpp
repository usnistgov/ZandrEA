// Expert Assistant (EA) research software for automated fault detection and diagnostics (AFDD)
// File origin (= "version 1.0"):  DAV at U.S. National Institute of Standards and Technology (NIST)
/*
   Header declaring classes for Subject objects and for Domain object (one Domain per Application)
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef SUBJECT_HPP
#define SUBJECT_HPP

#include "guiShadow.hpp"      // brings customTypes.hpp, which brings exportTypes.hpp

#include <memory>

class CCaseKit;
class CDomain;
class CDisplayRuleKit;
class CPointAnalog;
class CView;

typedef std::unordered_map<ERealName, std::unordered_map<EDataLabel, CPointAnalog* const> >
   SubjOutputsTable_t;

typedef std::unordered_map<NGuiKey, CDisplayRuleKit* const> RuleKitDisplayPtrTable_t;

typedef std::unordered_map<Nzint_t, std::unique_ptr<std::deque<int>> > IndexedFifoBuffers_t;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/*
   A "Subject object" represents to the EA software each physical thing (e.g., system, equipment piece )
   placed under AFDD surveillance.  An "AFDD Tool object" owns all objects executing that surveillance.
*/ 

class ASubject : public IGuiShadow { 

   public:

      ~ASubject( void );

      GuiPackSubjectBasic_t         SayBasicGuiPack( void ) const;
      GuiPackSubjectCases_t         SayCurrentCases( void ) const;
      const SEnergyPrices&          SayEnergyPricesRef( void ) const;
      std::string                   SayRootTextForDiskFilenames( void ) const;
      std::string                   SayNumRuleKitsAsText( void ) const;
      std::string                   SayDomainAndOwnNameAsText( void ) const;
      std::string                   SayNameAsText( void ) const;
      EDataLabel                    SayLabel( void ) const;
      ERealName                     SayName( void ) const;
      CCaseKit&                     SayCaseKitRef( void ) const;
      CView&                        SayViewRef( void ) const;
      Nzint_t                       GenerateAndSaySgiForNewCase( void );
      Nzint_t                       GenerateAndSaySgiForNewRuleKit( void );
      bool                          IsUnitOutputOkay( void ) const;

      void                          SubmitTrueIfGotFailOnPinnedRule( Nzint_t, bool );
      void                          CheckFeatureUaiFreeThenKeep( Nzint_t );
      void                          AddRuleKitDisplay( NGuiKey );
      void                          AddFeatureKey( NGuiKey );
      void                          TagAndPostAlertToDomain(   time_t,
                                                               EDataLabel,                                                               
                                                               EAlertMsg );
  
   protected:

   // Fields
      IndexedFifoBuffers_t                pinnedRuleFailHistories_byRuleKit;
      std::vector<std::string>            infoText;
      std::vector<NGuiKey>                featureKeys;
      std::vector<NGuiKey>                knobKeys;
      std::vector<NGuiKey>                ruleKitDisplayKeys;
      std::vector<Nzint_t>                uaiInUse_features;
      std::vector<Nzint_t>                sgiInUse_ruleKits;

      const EUnitSystem                   unitSys;     
      const EDataLabel                    ownLabel;
      const ERealName                     ownName;
      Nzint_t                             nextSgiForCases;
      Nzint_t                             nextSgiForRuleKits;
      bool                                unitOutputOkay;

   // Handles
      CDomain&                            DomainRef;
      std::unique_ptr<CCaseKit> const     u_CaseKit;
 
   // Methods
      ASubject(   EUnitSystem,
                  CDomain&,
                  EDataLabel,
                  ERealName );
};

//=====================================================================================================/
// Concrete Subject class for (generic) VAV unit with pressure-independent control and hot-water reheat

class CSubj_vav_ibal : public ASubject {

   public:
   // Methods

      CSubj_vav_ibal(   EUnitSystem,
                        CDomain&,
                        EDataLabel,    // own label
                        ERealName,     // own name
                        ERealName,     // AHU name
                        ERealName,     // Reheat source name (electric or HW plant sim)
                        float,         // duct diameter
                        float,         // duct area
                        float,         // air flow, rated
                        float,         // reheat HW flow, rated (zero if electric RH)
                        float );       // kW reheat, rated (zero if RH by HW plant sim)

      ~CSubj_vav_ibal( void );

   private:

   // Fields
      const ERealName      nameAntecedentAhu;
      const ERealName      nameAntecedentHwPlant;      
      const float          diamDuct;
      const float          areaAirflow;
      const float          airFlowRated;
      const float          hwFlowRated;
      const float          kWReheatRated;

   // Methods
      ERealName            SayNameOfAntecedentAhu( void ) const;
      ERealName            SayNameOfAntecedentHwPlant( void ) const;

      //void                 AttachOwnKnobs( CController& );   // Knobs for Subjects and Domain are TBD
};


//=====================================================================================================/
// Concrete Subject class for (generic) AHU that is single-duct, variable-volume, with terminal reheat


class CSubj_ahu_ibal : public ASubject {

   public:
   // Methods

      CSubj_ahu_ibal(   EUnitSystem,
                        CDomain&,
                        EDataLabel,          // own label
                        ERealName,           // own name
                        ERealName,           // CHW plant name
                        ERealName,           // preheat source name
                        float,               // air flow, rated
                        float,               // CHW flow, rated
                        float,               // kW preheat, rated
                        float );             // min fraction OA

      ~CSubj_ahu_ibal( void );

   private:

   // Fields
      const ERealName      nameAntecedentChwPlant;
      const ERealName      nameAntecedentHwPlant; 
      const float          airFlowRated;
      const float          chwFlowRated;
      const float          kwPreheatRated;
      const float          minFracOA;

   // Methods
      ERealName            SayNameOfAntecedentChwPlant( void ) const;
      ERealName            SayNameOfAntecedentHwPlant( void ) const;

      //void                 AttachOwnKnobs( CController& );   // Knobs for Subjects and Domain are TBD
};


//=====================================================================================================/
// Concrete Subject class for IBAL chiller machine


class CSubj_chlr_ibal : public ASubject {

   public:
   // Methods

      CSubj_chlr_ibal(  EUnitSystem,
                        CDomain&,
                        EDataLabel,          // own  
                        ERealName,           // own
                        ERealName,           // CT name
                        float,               // kW, rated ( = 3.517 x tons)
                        float,               // evap chw flow, rated (Lpm)
                        float );             // cond ctw flow, rated (Lpm)
 
      ~CSubj_chlr_ibal( void );

   private:

   // Fields
      const ERealName      nameAntecedentCT;
      const float          kWRated;
      const float          evapFlowRated;
      const float          condFlowRated;

   // Methods
      ERealName            SayNameOfAntecedentCT( void ) const;
      //void                 AttachOwnKnobs( CController& );   // Knobs for Subjects and Domain are TBD
};

//=====================================================================================================/
// Concrete Subject class for IBAL thermal energy storage (TES) (i.e., ice) tank and ctrl valve


class CSubj_tes_ibal : public ASubject {

   public:
   // Methods

      CSubj_tes_ibal(   EUnitSystem,
                        CDomain&,
                        EDataLabel,          // own  
                        ERealName,           // own
                        ERealName,           // chw plant name
                        float,               // kW-hours, rated ( = 3.517 x ton-hours)
                        float );             // chw tube flow, rated
 
      ~CSubj_tes_ibal( void );

   private:

   // Fields
      const ERealName      nameAntecedentChwPlant;
      const float          kWhRated;
      const float          tubeFlowRated;

   // Methods
      ERealName            SayNameOfAntecedentChwPlant( void ) const;
      //void                 AttachOwnKnobs( CController& );   // Knobs for Subjects and Domain are TBD
};

//=====================================================================================================/
// Concrete Subject class for IBAL CHW plant (CHWP)
// (i.e., primary loop around two antecedent chillers, itself antecedent to TES and AHUs)


class CSubj_chwp_ibal : public ASubject {

   public:
   // Methods

      CSubj_chwp_ibal(  EUnitSystem,
                        CDomain&,
                        EDataLabel,          // own  
                        ERealName,           // own
                        ERealName,           // chillerOne name
                        ERealName,           // chillerTwo name
                        float,               // total plant kW, rated ( = 3.517 x tons)
                        float );             // pri loop CHW flow, rated
 
      ~CSubj_chwp_ibal( void );

   private:

   // Fields
      const ERealName      nameAntecedentChlrOne;
      const ERealName      nameAntecedentChlrTwo;
      const float          plantkWRated;
      const float          loopFlowRated;

   // Methods
      ERealName            SayNameOfAntecedentChlrOne( void ) const;
      ERealName            SayNameOfAntecedentChlrTwo( void ) const;
      //void                 AttachOwnKnobs( CController& );   // Knobs for Subjects and Domain are TBD
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/*
   A Domain object knows collectively all the interrelated Subject objects in the Application object
   (e.g., all the equipment to be placed under AFDD in a particular building)
*/


class CDomain : public IGuiShadow {

   public:

      explicit CDomain( ERealName );

      ~CDomain( void );

      GuiPackDomain_t                  SayGuiPack( void ) const;
      std::queue<std::string>          SayNewAlertsFifoThenClear( void );
      std::vector<NGuiKey>             SaySubjectKeys( void ) const;
      const SEnergyPrices&             SayEnergyPricesRef( void ) const;
      std::string                      SayRootTextForDiskFilenames( void ) const;
      ERealName                        SayName( void ) const;
      const ASubject* const            SayPtrToSubjectNamed( ERealName ) const;
      CView* const                     SayViewPtr( void ) const;
      void                             Register( CView* const );
      void                             Register( ASubject* const, ERealName );



      void                             PostAsNewAlert(   time_t,
                                                         ERealName,
                                                         EDataLabel,
                                                         EAlertMsg );

   private:

      SubjOutputsTable_t                                 p_SubjOutputs_byName_byLabel;
      std::unordered_map<ERealName, ASubject*>           p_Subjects_byName;
      CView*                                             p_View;
      //ParamPack_t                                      ownParamPack;
      std::queue<std::string>                            unsaidAlertsFifo;
      EnergyPrices_t                                     energyPrices;
      const ERealName                                    domainName;
   


};


#endif


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX



--------------------------------------------------------------------------------
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
