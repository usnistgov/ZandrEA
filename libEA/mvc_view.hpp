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
   Declares the concrete class CView for an EA Application's  MVC View object. 
   A CView object ultimately implements the "View" methods of the EA runtime API.
   Since the API are all pure virtual methods, the implementation first goes through a CPortOmni object.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef MVC_VIEW_HPP
#define MVC_VIEW_HPP

#include "customTypes.hpp"

#include <memory>


// Forward declares (to avoid unnecessary #includes)
class AFeature;
class AHistogram;
class AKrono;
class APane;
class ASubject;
class ATrace;

class CCaseKit;
class CController;
class CDisplayRuleKit;
class CDomain;

class CHistogramAnalog;
class CHistogramFact;
class CHistogramRule;
class CHistogramRuleKit;
class CKronoRealtime;
class CKronoSnapshot;
class CPaneRealtime;
class CPaneSnapshot;
class CTraceRealtime;
class CTraceSnapshot;

typedef std::unordered_map<NGuiKey, CCaseKit*>              CaseKitPtrTable_t;
typedef std::unordered_map<NGuiKey, AFeature*>              FeaturePtrTable_t;
typedef std::unordered_map<NGuiKey, AHistogram*>            HistoPtrTable_t;
typedef std::unordered_map<NGuiKey, AKrono*>                KronoPtrTable_t;
typedef std::unordered_map<NGuiKey, APane*>                 PanePtrTable_t;
typedef std::unordered_map<NGuiKey, CDisplayRuleKit*>       RuleDisplayPtrTable_t;
typedef std::unordered_map<NGuiKey, ASubject*>              SubjectPtrTable_t;
typedef std::unordered_map<NGuiKey, ATrace*>                TracePtrTable_t;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare a concrete class implementing all "View" functionality

class CView {

  public:

   // Methods
                     CView(   CDomain&,
                              CController& );

                     ~CView( void );

      EGuiReply                  Update( void );     // See File Note [1]

      GuiPackDomain_t            SayGuiPackFromDomain( void ) const;
      std::queue<std::string>    SayNewAlertsFifoFromDomainThenClear( void );

      GuiPackSubjectBasic_t      SayGuiPackFromSubject( NGuiKey ) const;
      GuiPackSubjectCases_t      SayCurrentCasesFromSubject( NGuiKey ) const;
      std::string                SayTextIdentifyingSubject( NGuiKey ) const;

      GuiPackRuleKitFull_t       SayFullGuiPackFromRuleKitDisplay( NGuiKey ) const;
      GuiPackRuleKitDyna_t       SayDynamicGuiPackFromRuleKitDisplay( NGuiKey ) const;
      std::string                SayTextIdentifyingRuleKit( NGuiKey ) const;

      GuiPackFeatureFull_t       SayFullGuiPackFromFeature( NGuiKey ) const;
      GuiPackFeatureDyna_t       SayDynamicGuiPackFromFeature( NGuiKey ) const;

      GuiPackCaseFull_t          SayFullGuiPackFromCase( NGuiKey ) const;
      GuiPackCaseDyna_t          SayDynamicGuiPackFromCase( NGuiKey ) const;
      EGuiReply                  AnswerCaseWithOptionIndex( NGuiKey, size_t );

      GuiPackKronoFull_t         SayFullGuiPackFromKrono( NGuiKey ) const;
      GuiPackKronoDyna_t         SayDynamicGuiPackFromKrono( NGuiKey ) const;

      GuiPackPane_t              SayGuiPackFromPane( NGuiKey ) const;

      GuiPackTraceFull_t         SayFullGuiPackFromTrace( NGuiKey ) const;
      GuiPackTraceDyna_t         SayDynamicGuiPackFromTrace( NGuiKey ) const;

      GuiPackHistogram_t         SayGuiPackFromHistogram( NGuiKey ) const;
      std::string                SayTextIdentifyingHistogram( NGuiKey ) const;
      EGuiReply                  SetModeOfHistogramToOptionIndex( NGuiKey, size_t );
      EGuiReply                  SetSpanOfHistogramToOptionIndex( NGuiKey, size_t );

      void                       AddCaseToCaseKitLookup( NGuiKey, CCaseKit* );
      void                       RemoveCaseFromCaseKitLookup( NGuiKey );

      void                       AddFeature( AFeature& );
 
      void                       AddHistogram( AHistogram& );
 
      void                       AddRuleKitDisplay( CDisplayRuleKit& );
      void                       AddSubject( ASubject& );

      void                       GainAccessTo( CKronoRealtime* const );
      void                       GainAccessTo( CKronoSnapshot* const );

      void                       GainAccessTo( CPaneRealtime* const );
      void                       GainAccessTo( CPaneSnapshot* const );

      void                       GainAccessTo( CTraceRealtime* const );
      void                       GainAccessTo( CTraceSnapshot* const );

      void                       LoseAccessTo( CKronoRealtime* const );
      void                       LoseAccessTo( CKronoSnapshot* const );

      void                       LoseAccessTo( CPaneRealtime* const );
      void                       LoseAccessTo( CPaneSnapshot* const );

      void                       LoseAccessTo( CTraceRealtime* const );
      void                       LoseAccessTo( CTraceSnapshot* const );

   private:

      CDomain&                         DomainRef;

       /*   pointer nomenclature:
               "p_" = raw (typically, a local copy of a unique ptr to an object owned elsewhere),
               "s_" = shared smart pointer, target ownership shared with other class(es),
               "u_" = unique smart pointer, target obj owned entirely by the class holding it
      */

      CaseKitPtrTable_t                   p_CaseKits_byCaseKey;
      FeaturePtrTable_t                   p_Features_byKey;
      HistoPtrTable_t                     p_Histograms_byKey;
      KronoPtrTable_t                     p_Kronos_byKey;
      PanePtrTable_t                      p_Panes_byKey;
      RuleDisplayPtrTable_t               p_RuleKitDisplays_byKey;
      SubjectPtrTable_t                   p_Subjects_byKey;
      TracePtrTable_t                     p_Traces_byKey;
 
      KitAlertsMap_t                      postedAlerts;

   // Methods
      EGuiReply                           Regen( void );

};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
