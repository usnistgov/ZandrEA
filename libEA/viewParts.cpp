// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare abstract base and concrete classes for objects (parts) accessed via View object
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "viewParts.hpp"

#include "agentTask.hpp"
#include "mvc_view.hpp"
#include "subject.hpp"
#include "rule.hpp"
#include "dataChannel.hpp"
#include "formula.hpp"
#include "state.hpp"
#include "rainfall.hpp"
#include "controlParts.hpp"

#include <algorithm>
#include <limits>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations of AFeature (abstract) and concrete subclasses

AFeature::AFeature(  EApiType bArg0,
                     ASubject& arg0,
                     CView& arg1,
                     Nzint_t arg2,
                     EDataLabel arg3,
                     EDataUnit arg4,
                     NGuiKey arg5 )
                     :  IGuiShadow( bArg0 ),
                        SubjectRef (arg0),
                        knobKeysAllSources(),
                        label (arg3),
                        units (arg4),
                        messageText (MSG_TROUBLE),
                        ownUai (arg2),
                        stateDisplayed (EGuiState::Undefined),
                        histogramKey_source (arg5) {

   try { arg0.CheckFeatureUaiFreeThenKeep( arg2 ); }
   catch ( std::logic_error ) { throw; }

   arg0.AddFeatureKey( ownGuiKey );
   arg1.AddFeature( *this );
}


//=====================================================================================================/
// Public methods

GuiPackFeatureFull_t  AFeature::SayFullGuiPack( void  ) {

   return SGuiPackFeatureFull(   LookUpGuiType(ownApiType),
                                 ownGuiKey,
                                 ownUai,
                                 LookUpTag( label ),
                                 LookUpText( units ),
                                 messageText,
                                 stateDisplayed,
                                 histogramKey_source,
                                 knobKeysAllSources );
}


GuiPackFeatureDyna_t  AFeature::SayDynamicGuiPack( void ) {

   return SGuiPackFeatureDyna(   messageText,
                                 stateDisplayed );
}


void AFeature::Update( void ) { // Decouples updates from the specific subclass fields being updated

   Regen();
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CFeatureAnalog concrete subclass of AFeature

std::ostringstream CFeatureAnalog::floatStreamer;


CFeatureAnalog::CFeatureAnalog(  ASubject& bArg0,
                                 CView& bArg1,
                                 Nzint_t bArg2,
                                 CPointAnalog& arg0,
                                 FloatGetr_t arg1 )
                                 :  AFeature(   EApiType::Feature_analog,
                                                bArg0,
                                                bArg1,
                                                bArg2,
                                                arg0.SayLabel(),
                                                arg0.SayUnits(),
                                                arg0.u_Rain->SayHistogramKey()
                                 ),
                                 SourceRef_baseClass (arg0),
                                 ReadSource (arg1) {

   arg0.LendKnobKeysTo( knobKeysAllSources );
 }


CFeatureAnalog::CFeatureAnalog(  ASubject& bArg0,
                                 CView& bArg1,
                                 Nzint_t bArg2,
                                 CFormula& arg0,
                                 FloatGetr_t arg1 )
                                 :  AFeature(   EApiType::Feature_analog,
                                                bArg0,
                                                bArg1,
                                                bArg2,
                                                arg0.SayLabel(),
                                                arg0.SayUnits(),
                                                arg0.u_Rain->SayHistogramKey()
                                 ),
                                 SourceRef_baseClass (arg0),
                                 ReadSource (arg1) {

   arg0.LendKnobKeysTo( knobKeysAllSources );   // Need ref of concrete type for polymorhism here 
 }

CFeatureAnalog::~CFeatureAnalog( void ) {

   // empty d-tor
}


void CFeatureAnalog::Regen( void ) {

   messageText = WriteOutFloat( ReadSource() );

   stateDisplayed = (  SourceRef_baseClass.IsValid() ?
                        EGuiState::Feature_neutral :
                        EGuiState::Feature_invalid );

   return;
}

std::string CFeatureAnalog::WriteOutFloat( float inputNum ) {

   floatStreamer.str( std::string() );             // wipe clean the std::stringstream object

   if (inputNum != 0.0f && inputNum < 1.0f && inputNum > -1.0f) { floatStreamer.precision(3); }
   else { floatStreamer.precision(1); }

   // 'fixed' makes precision(n) set n digits right of pt. versus n total sig figs (the STL default)
   floatStreamer << std::fixed << inputNum;

   return floatStreamer.str();      // leaves stream "dirty", OK as long as always wiped before use
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CFeatureFact concrete subclass of AFeature

CFeatureFact::CFeatureFact(   ASubject& bArg0,
                              CView& bArg1,
                              Nzint_t bArg2,
                              AFact& arg0,
                              BoolGetr_t arg1)
                              :  AFeature(   EApiType::Feature_fact,
                                             bArg0,
                                             bArg1,
                                             bArg2,
                                             arg0.SayLabel(),
                                             arg0.SayUnits(),
                                             arg0.u_Rain->SayHistogramKey()
                                 ),
                                 SourceRef_factClass (arg0),
                                 ReadSource (arg1) {

   arg0.LendKnobKeysTo( knobKeysAllSources );
}

CFeatureFact::~CFeatureFact( void ) {

   // empty d-tor
}


void CFeatureFact::Regen( void ) {

   WriteTimestampAsTextTo( SourceRef_factClass.SayTimeOfClaimNow(), messageText );

   stateDisplayed = (   SourceRef_factClass.IsValid() ?
                           ( ReadSource() ?
                              EGuiState::Feature_true :
                              EGuiState::Feature_false ) :
                        EGuiState::Feature_invalid
   ); 
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of ATrace abstract class

ATrace::ATrace(   EApiType bArg0,
                  const ARainfall& arg0,
                  const ASubject& arg1,
                  CView& arg2,
                  const std::vector<NGuiKey>& arg3,
                  NGuiKey arg4,
                  Nzint_t arg5 ) 
                  :  IGuiShadow( bArg0 ),
                     SourceRef (arg0.SourceRef ),
                     SubjectRef (arg1),
                     ViewRef (arg2),
                     knobKeys_sourceRef (arg3),
                     histogramKey_source (arg4),
                     RainRef_upcast (arg0),
                     nameText (  LookUpTag( arg0.SourceRef.SayLabel() ) +
                                    LookUpText( SourceRef.SaySuffix() )
                     ),
                     ruleUai (arg5) {

}

ATrace::ATrace(   EApiType bArg0,
                  const CRainRuleKit& arg0,
                  const CRule& arg1,
                  const ASubject& arg2,
                  CView& arg3,
                  const std::vector<NGuiKey>& arg4 ) 
                  :  IGuiShadow( bArg0 ),
                     SourceRef (arg0.SourceRef ),
                     SubjectRef (arg2),
                     ViewRef (arg3),
                     knobKeys_sourceRef (arg4),
                     histogramKey_source (
                        arg0.SayKeyToHistogramOfRule( arg1.SayRuleUai() )
                     ),
                     RainRef_upcast (arg0),
                     nameText ( arg1.SayRuleUaiText() ),
                     ruleUai ( arg1.SayRuleUai() ) {

}

ATrace::~ATrace( void ) {  }

int ATrace::SaySecondsPerIndex( void ) const { return SourceRef.SaySecsPerCycle(); }

ERealName ATrace::SayHostSubjectName( void ) const { return SubjectRef.SayName(); }

EDataUnit ATrace::SayUnits( void ) const { return SourceRef.SayUnits(); }

EDataRange ATrace::SayRange( void ) const { return SourceRef.SayRange(); }

EPlotGroup ATrace::SayPlotGroup( void ) const { return SourceRef.SayPlotGroup(); }


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Implementation of CTraceRealtime concrete class

CTraceRealtime::CTraceRealtime(  CView& bArg0,
                                 CRainAnalog& arg0,
                                 const std::vector<NGuiKey>& arg1 ) 
                                 :  ATrace(  EApiType::Trace_realtime_analog,
                                             arg0,
                                             arg0.SourceRef.SaySubjectRefAsConst(),
                                             bArg0,
                                             arg1,
                                             arg0.SayHistogramKey(),
                                             0 // passed explicitly, as snapshots also use ATrace ctor
                                    ),
                                    p_RainAnalog (&arg0),
                                    p_RainFact (nullptr),
                                    p_RainRuleKit (nullptr) {

   bArg0.GainAccessTo( this );
}

CTraceRealtime::CTraceRealtime(  CView& bArg0,
                                 CRainFact& arg0,
                                 const std::vector<NGuiKey>& arg1 ) 
                                 :  ATrace(  EApiType::Trace_realtime_fact,
                                             arg0,
                                             arg0.SourceRef.SaySubjectRefAsConst(),
                                             bArg0,
                                             arg1,
                                             arg0.SayHistogramKey(),
                                             0 // passed explicitly, as snapshots also use ATrace ctor
                                    ),
                                    p_RainAnalog (nullptr),
                                    p_RainFact (&arg0),
                                    p_RainRuleKit (nullptr) {

   bArg0.GainAccessTo( this );
}

/* C-tor for Trace of an individual Rule; as (unlike Histogram) there are no Traces of entire Rule Kits.
   Instead, a Rule Kit has knob to generate a Krono having one Pane with N individual Traces for each
   of N Rules in kit
*/
CTraceRealtime::CTraceRealtime(  CView& bArg0,
                                 CRainRuleKit& arg0,
                                 const CRule& arg1,
                                 const std::vector<NGuiKey>& arg2 )
                                 :  ATrace(  EApiType::Trace_realtime_rule,
                                             arg0,
                                             arg1,
                                             arg0.SourceRef.SaySubjectRefAsConst(),
                                             bArg0,
                                             arg2                                             
                                    ),
                                    p_RainAnalog (nullptr),
                                    p_RainFact (nullptr),
                                    p_RainRuleKit (&arg0) {

   bArg0.GainAccessTo( this );
}


CTraceRealtime::~CTraceRealtime( void ) { ViewRef.LoseAccessTo( this ); }

//=====================================================================================================/
// Public methods


void CTraceRealtime::CaptureSnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   switch ( ownApiType ) {

      case EApiType::Trace_realtime_fact:
         p_RainFact->CaptureSnapshotForSetSgi( snapshotSetSgi );
         break;

      case EApiType::Trace_realtime_analog:
         p_RainAnalog->CaptureSnapshotForSetSgi( snapshotSetSgi );
         break;

      default:
         break;
   }
   return;
}


void CTraceRealtime::DestroySnapshotForSetSgi( Nzint_t snapshotSetSgi ) {

   switch ( ownApiType ) {

      case EApiType::Trace_realtime_fact:
         p_RainFact->DestroySnapshotForSetSgi( snapshotSetSgi );
         break;

      case EApiType::Trace_realtime_analog:
         p_RainAnalog->DestroySnapshotForSetSgi( snapshotSetSgi );
         break;

      default:
         break;
   }
   return;
}


GuiPackTraceFull_t CTraceRealtime::SayFullGuiPack( void ) const {

   // note that triadics test first for most likely source type, thru to least likely source type

   return SGuiPackTraceFull(  LookUpGuiType( ownApiType ),
                              ownGuiKey,
                              nameText,  // units in pane
                              (  (ownApiType == EApiType::Trace_realtime_analog) ?
                                    p_RainAnalog->SayGuiNumbersFromBindexLog() :
                                    std::vector<GuiFpn_t>(0) 
                              ),
                              (  (ownApiType == EApiType::Trace_realtime_fact) ?
                                    p_RainFact->SayGuiStatesFromBindexLog() :
                                    ( (ownApiType == EApiType::Trace_realtime_analog) ?
                                       p_RainAnalog->SayGuiStatesFromBindexLog() :
                                       ( (ownApiType == EApiType::Trace_realtime_rule) ?
                                          p_RainRuleKit->SayGuiStatesFromBindexLogUnderRuleUai(ruleUai) :
                                          std::vector<EGuiState>(0)
                                       )
                                    )
                              ),
                              NGuiKey( histogramKey_source ),
                              std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                      knobKeys_sourceRef.end() ) 
   );
}


GuiPackTraceDyna_t CTraceRealtime::SayDynamicGuiPack( void ) const {

   return SGuiPackTraceDyna( (  (ownApiType == EApiType::Trace_realtime_analog) ?
                                    p_RainAnalog->SayGuiNumberFromNewestBindex() :
                                    NaNDBL 
                              ),  
                              (  (ownApiType == EApiType::Trace_realtime_fact) ?
                                    p_RainFact->SayGuiStateFromNewestBindex() :
                                    ( (ownApiType == EApiType::Trace_realtime_analog) ?
                                       p_RainAnalog->SayGuiStateFromNewestBindex() :
                                       ( (ownApiType == EApiType::Trace_realtime_rule) ?
                                          p_RainRuleKit->SayGuiStateFromNewestBindexUnderRuleUai(ruleUai) :
                                          EGuiState::Undefined
                                       )
                                    )
                              )
   );
}


///VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Implementation of CTraceSnapshot concrete class

CTraceSnapshot::CTraceSnapshot(  const CTraceRealtime& rtRef,
                                 Nzint_t snapshotSetSgi ) 
                                 :  ATrace(
                                       (  ( rtRef.ownApiType == EApiType::Trace_realtime_fact ) ?
                                             EApiType::Trace_snapshot_fact :
                                             ( rtRef.ownApiType == EApiType::Trace_realtime_analog ) ?
                                                EApiType::Trace_snapshot_analog :
                                                EApiType::Trace_snapshot_rule
                                       ),
                                       rtRef.RainRef_upcast,                                          
                                       rtRef.SubjectRef,
                                       rtRef.ViewRef,
                                       rtRef.knobKeys_sourceRef,
                                       rtRef.histogramKey_source,
                                       rtRef.ruleUai
                                    ),
                                    p_RainAnalog ( rtRef.p_RainAnalog ),
                                    p_RainFact ( rtRef.p_RainFact ),
                                    p_RainRuleKit ( rtRef.p_RainRuleKit ),
                                    sgiOfSnapshotSetToDisplay (snapshotSetSgi) {

   ViewRef.GainAccessTo( this );

/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Unlike realtime traces, snapshot traces are mortal, there (should be) much fewer of them (their LUT
      a much shorter lookup), so they appear on View immediately upon construction and are vanished only
      upon destruction.

[2]   No calls needed by c-tor to populate histogram and knob key lists; those are copied from
      progenitor realtime trace

'' End Method Notes ''' */   

}

CTraceSnapshot::~CTraceSnapshot( void ) { ViewRef.LoseAccessTo( this ); }

//=====================================================================================================/
// Public methods

GuiPackTraceFull_t CTraceSnapshot::SayFullGuiPack( void ) const {

   // note that triadics test first for most likely source type, thru to least likely source type

   return   SGuiPackTraceFull(
               LookUpGuiType( ownApiType ),
               ownGuiKey,
               nameText,  // units in pane
               (  (ownApiType == EApiType::Trace_snapshot_analog) ?
                     p_RainAnalog->SayGuiNumbersFromSnapshotInSet( sgiOfSnapshotSetToDisplay ) :
                     std::vector<GuiFpn_t>(0) 
               ),
               (  (ownApiType == EApiType::Trace_snapshot_fact) ?
                     p_RainFact->SayGuiStatesFromSnapshotInSet( sgiOfSnapshotSetToDisplay) :
                     (  (ownApiType == EApiType::Trace_snapshot_analog) ?
                           p_RainAnalog->
                              SayGuiStatesFromSnapshotInSet( sgiOfSnapshotSetToDisplay ) :
                           (  (ownApiType == EApiType::Trace_snapshot_rule) ?
                                 p_RainRuleKit->
                                    SayGuiStatesFromSnapshotInSet( sgiOfSnapshotSetToDisplay ) :
                                 std::vector<EGuiState>(0)
                                       )
                                    )
               ),
               NGuiKey(histogramKey_source),
               std::vector<NGuiKey>( knobKeys_sourceRef.begin(), knobKeys_sourceRef.end() )
            );
}


GuiPackTraceDyna_t CTraceSnapshot::SayDynamicGuiPack( void ) const {

   return SGuiPackTraceDyna( EGuiReply::FAIL_get_calledDynamicUpdateUsingKeyToStaticData );
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations of APane (abstract) and concrete subclasses

APane::APane(  const CTraceRealtime* const arg0,
               CView& arg1 )
              :   IGuiShadow( ( arg0->SayApiType() == EApiType::Trace_realtime_analog ) ?
                                 EApiType::Pane_realtime_analog :
                                 (  ( arg0->SayApiType() == EApiType::Trace_realtime_fact ) ?
                                       EApiType::Pane_realtime_fact :
                                       EApiType::Pane_realtime_rule )
                  ),
                  ViewRef (arg1),
                  keysOfTracesInPane( 1, arg0->SayGuiKey() ),
                  numTracesMax ( (ownApiType == EApiType::Pane_realtime_fact) ?
                                    FIXED_PANE_FACT_NUMTRACES_MAX :
                                    (  (ownApiType == EApiType::Pane_realtime_analog) ?
                                          FIXED_PANE_ANALOG_NUMTRACES_MAX :
                                          FIXED_PANE_RULE_NUMTRACES_MAX
                                    )
                  ),
                  xAxisSecsPerIndex ( arg0->SaySecondsPerIndex() ),
                  yAxisUnits ( arg0->SayUnits() ),
                  yAxisRange ( arg0->SayRange() ),
                  hostSubject ( arg0->SayHostSubjectName() ),
                  plotGroup ( arg0->SayPlotGroup() ) {

   // empty c-tor
}


APane::APane(  const CTraceSnapshot* const arg0,
               CView& arg1 )
               :   IGuiShadow(   ( arg0->SayApiType() == EApiType::Trace_snapshot_analog ) ?
                                    EApiType::Pane_snapshot_analog :
                                    (  ( arg0->SayApiType() == EApiType::Trace_snapshot_fact ) ?
                                          EApiType::Pane_snapshot_fact :
                                          EApiType::Pane_snapshot_rule )
                  ),
                  ViewRef (arg1),
                  keysOfTracesInPane( 1, arg0->SayGuiKey() ),
                  numTracesMax ( (ownApiType == EApiType::Pane_snapshot_fact) ?
                                    FIXED_PANE_FACT_NUMTRACES_MAX :
                                    (  (ownApiType == EApiType::Pane_snapshot_analog) ?
                                          FIXED_PANE_ANALOG_NUMTRACES_MAX :
                                          FIXED_PANE_RULE_NUMTRACES_MAX
                                    )
                  ),
                  xAxisSecsPerIndex ( arg0->SaySecondsPerIndex() ),
                  yAxisUnits ( arg0->SayUnits() ),
                  yAxisRange ( arg0->SayRange() ),
                  hostSubject (arg0->SayHostSubjectName()),
                  plotGroup ( arg0->SayPlotGroup() ) {

   // empty c-tor
}


APane::~APane( void ) {

   // Empty at ABC level.  Concrete subclass d-tor must direct View to LoseAccessTo() object's Key.
};

//=====================================================================================================/
// Public methods

GuiPackPane_t APane::SayGuiPack( void ) const {

   return SGuiPackPane( LookUpGuiType(ownApiType),
                        ownGuiKey,
                        LookUpText( yAxisUnits ),
                        static_cast<GuiFpn_t>( LookUpMinMax(yAxisRange)[0] ), // min, NaN if a state
                        static_cast<GuiFpn_t>( LookUpMinMax(yAxisRange)[1] ), // max, NaN if a state
                        keysOfTracesInPane );
}


int APane::SaySecondsPerCycle( void ) const { return xAxisSecsPerIndex; }


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CPaneRealtime concrete subclass of APane

CPaneRealtime::CPaneRealtime( const CTraceRealtime* const bArg1,
                              CView& bArg2 ) 
                              :  APane(   bArg1,
                                          bArg2
                                 ) {

   bArg2.GainAccessTo( this );
}


CPaneRealtime::~CPaneRealtime( void ) { ViewRef.LoseAccessTo( this ); }


bool CPaneRealtime::AddTraceIfCompatible( const CTraceRealtime* const p_traceProposed ) {
   
   bool reply = false;

   // Trace sourced from a "foreign" Subject is plotted "freely", i.e., as having no "group"
   // Pane itself is instantiated by Rule or Case, so its Subject is "native" to those
   EPlotGroup actingPlotGroup = (
      ( p_traceProposed->SayHostSubjectName() == hostSubject ) ?
         p_traceProposed->SayPlotGroup() :
         EPlotGroup::Free
   );
   if (  ( actingPlotGroup == plotGroup ) &&
         ( keysOfTracesInPane.size() <= numTracesMax ) &&
         ( p_traceProposed->SayUnits() == yAxisUnits  ) &&
         ( p_traceProposed->SayRange() == yAxisRange  ) &&
         ( p_traceProposed->SaySecondsPerIndex() == xAxisSecsPerIndex )
      ) {
      keysOfTracesInPane.push_back( p_traceProposed->SayGuiKey() );
      reply = true;
   }
   return reply;
}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CPaneSnapshot concrete subclass of APane

CPaneSnapshot::CPaneSnapshot( const CTraceSnapshot* const bArg1,
                              CView& bArg2 ) 
                              :  APane(   bArg1,
                                          bArg2
                                 ) {

   bArg2.GainAccessTo( this );
}


CPaneSnapshot::~CPaneSnapshot( void ) { ViewRef.LoseAccessTo( this ); }


bool CPaneSnapshot::AddTraceIfCompatible( const CTraceSnapshot* const p_traceProposed ) {

   bool reply = false;

   // Trace sourced from a "foreign" Subject is plotted "freely", i.e., as having no "group"
   // Pane itself is instantiated by Rule or Case, so its Subject is "native" to those
   EPlotGroup actingPlotGroup = (
      ( p_traceProposed->SayHostSubjectName() == hostSubject ) ?
         p_traceProposed->SayPlotGroup() :
         EPlotGroup::Free
   );
   if (  ( actingPlotGroup == plotGroup ) &&
         ( keysOfTracesInPane.size() <= numTracesMax ) &&
         ( p_traceProposed->SayUnits() == yAxisUnits  ) &&
         ( p_traceProposed->SayRange() == yAxisRange  ) &&
         ( p_traceProposed->SaySecondsPerIndex() == xAxisSecsPerIndex )
      ) {
      keysOfTracesInPane.push_back( p_traceProposed->SayGuiKey() );
      reply = true;
   }
   return reply;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of logic chronology "Krono objects" (concrete class)


AKrono::AKrono(   EApiType bArg0,
                  CRuleKit& arg0,
                  CController& arg1,
                  CView& arg2,
                  std::string arg3,
                  Nzint_t arg4 )
                  :  IGuiShadow( bArg0 ),
                     TimeAxisRef ( arg0.SayTimeAxisRef() ),
                     ViewRef (arg2),
                     secsLookingBack (START_DATALOG_SECSLOGGING),
                     u_Knob(  std::make_unique<CKnobSint>(  arg1,
                                                            arg0,
                                                            EDataLabel::Knob_krono_setLookback,
                                                            EDataUnit::TimeSpan_sec,
                                                            EDataSuffix::None,
                                                            (  [&]( int userInputVetted ) ->void {
                                                                  SetSecsLookingBack( userInputVetted ); 
                                                                  return; }
                                                            ),
                                                            INIT_KNOB_MINDEFMAX_DATALOG_SECSLOGGING,
                                                            secsLookingBack
                                                            
                              )
                     ), 
                     paneIdsTopToBottom(),
                     caption (arg3),
                     secsPerIndex_sharedTimeAxis ( arg0.SaySecsPerCycle() ),
                     numIndicies_sharedTimeAxis ( START_DATALOG_SECSLOGGING / arg0.SaySecsPerCycle() ),
                     snapshotSetSgi (arg4) {

   // register/unregister with CView at subclass level even when View holds base class handle
}


AKrono::~AKrono( void ) {

   // Empty at ABC level. Concrete subclass d-tor must direct View to LoseAccessTo() object's Key.
}


//=====================================================================================================/
// Public methods


GuiPackKronoFull_t AKrono::SayFullGuiPack( void ) const {

   return SGuiPackKronoFull(  LookUpGuiType( ownApiType ),
                              ownGuiKey,
                              caption,
                              std::vector<NGuiKey>( 1, u_Knob->SayGuiKey() ),
                              paneIdsTopToBottom,
                              (  snapshotSetSgi == 0 ?
                                    TimeAxisRef.DisplayAxisInRealtime() :
                                    TimeAxisRef.DisplayAxisFromSnapshotSet( snapshotSetSgi )
                              )
   );
}


size_t AKrono::SayNumCyclesLookingBack( void ) const { return numIndicies_sharedTimeAxis; }


int AKrono::GetSecsLookingBack( void ) const {

   return ( static_cast<int>(numIndicies_sharedTimeAxis) * secsPerIndex_sharedTimeAxis );
}


void AKrono::SetSecsLookingBack( int secsWanted ) {

   // $$$ TBD dev this once Kronos working. Impl must round to nearest multiple of trigger period ! $$$
   // $$$ TBD frontend to get back a EGuiReply ? ! $$$
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for realtime krono


CKronoRealtime::CKronoRealtime(  CRuleKit& bArg0,
                                 CController& bArg1,
                                 CView& bArg2,
                                 const std::string bArg3,
                                 const CPaneRealtime* const arg0 )
                                 :  AKrono(  EApiType::Krono_realtime,
                                             bArg0,
                                             bArg1,
                                             bArg2,
                                             bArg3,
                                             0        // snapshot set ID = 0 for realtime kronos
                                    ),
                                    RuleKitRef (bArg0) {

   if (  arg0->SayApiType() != EApiType::Pane_realtime_rule ) {
      throw std::logic_error( "Attempted loading top of Krono with Pane not a Rule result" );
   }

   paneIdsTopToBottom.push_back( arg0->SayGuiKey() );
   ViewRef.GainAccessTo( this );
}


CKronoRealtime::~CKronoRealtime( void ) {

   ViewRef.LoseAccessTo( this );
   RuleKitRef.ClearKitOfRealtimeKronoParts();
}

//=====================================================================================================/
// Public methods


GuiPackKronoDyna_t CKronoRealtime::SayDynamicGuiPack( void ) const {

   return SGuiPackKronoDyna( TimeAxisRef.SayTimeNewest() );
}

void CKronoRealtime::AddPaneBelowExistingPanes( const CPaneRealtime* const ptrToPane ) {

   paneIdsTopToBottom.push_back( ptrToPane->SayGuiKey() );
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for snapshot krono


CKronoSnapshot::CKronoSnapshot(  CRuleKit& bArg0,
                                 CController& bArg1,
                                 CView& bArg2,
                                 const std::string& bArg3,
                                 Nzint_t bArg4,
                                 const CPaneSnapshot* const arg0 )
                                 :  AKrono(  EApiType::Krono_snapshot,
                                             bArg0,
                                             bArg1,
                                             bArg2,
                                             bArg3,
                                             bArg4
                                    ) {

   if (  arg0->SayApiType() != EApiType::Pane_snapshot_rule ) {
      throw std::logic_error( "Attempted loading top of Krono with Pane not a Rule result" );
   }

   paneIdsTopToBottom.push_back( arg0->SayGuiKey() );
   ViewRef.GainAccessTo( this );
}


CKronoSnapshot::~CKronoSnapshot( void ) {

   ViewRef.LoseAccessTo( this );
}

//=====================================================================================================/
// Public methods

GuiPackKronoDyna_t CKronoSnapshot::SayDynamicGuiPack( void ) const {

   return SGuiPackKronoDyna( EGuiReply::FAIL_get_calledDynamicUpdateUsingKeyToStaticData );
}

void CKronoSnapshot::AddPaneBelowExistingPanes( const CPaneSnapshot* const ptrToPane ) {

   paneIdsTopToBottom.push_back( ptrToPane->SayGuiKey() );
   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations of CDisplayRuleKit

CDisplayRuleKit::CDisplayRuleKit(   ASubject& arg0,
                                    CRuleKit& arg1,
                                    CView& arg2 )
                                    :  IGuiShadow( EApiType::Display_ruleKit ),
                                       RuleKitRef (arg1) {

   arg0.AddRuleKitDisplay( SayGuiKey() );    // own key to Subject for User's getter call to obtain it
   arg2.AddRuleKitDisplay( *this );          // View holds ptr-by-key table for User to get GuiPack(s) 
}


CDisplayRuleKit::~CDisplayRuleKit( void ) { }


GuiPackRuleKitFull_t CDisplayRuleKit::SayFullGuiPack( void ) const {

   // $$$ TBD as whether this works okay as is (via RVO ?) or should redesign to speed it up $$$

   return SGuiPackRuleKitFull(   LookUpGuiType(ownApiType),
                                 ownGuiKey,
                                 RuleKitRef.SayKitCaption(),
                                 RuleKitRef.SayRuleKitKnobKeys(),                   // See Class Note [1]
                                 RuleKitRef.SayRuleLabels_GuiTopToBottom(),
                                 RuleKitRef.SayRuleTexts_If_GuiTopToBottom(),
                                 RuleKitRef.SayRuleTexts_Then_GuiTopToBottom(),
                                 RuleKitRef.SayNewestRuleStates_GuiTopToBottom(),
                                 RuleKitRef.SayKnobKeyOfEachRule_GuiTopToBottom(),  // See Class Note [2]
                                 RuleKitRef.SayHistogramKeyOfEachRule_GuiTopToBottom(),
                                 RuleKitRef.SayHistogramKey(), // kit histo, shows all rules at once
                                 RuleKitRef.SayRealtimeKronoKey()
   );

/* Start Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Kit's own knobs = (1) idle all rules? [bool]; (2) load r-t krono with all rules? [bool]; (3) load
      r-t krono with one rule and its antecedents [UAI of rule wanted]

[2]   Each rule has only one knob = idle? [bool]

'' End Class Notes '''' */

}


GuiPackRuleKitDyna_t CDisplayRuleKit::SayDynamicGuiPack( void ) const {

   return SGuiPackRuleKitDyna(   RuleKitRef.SayNewestRuleStates_GuiTopToBottom(),
                                 RuleKitRef.SayRealtimeKronoKey()
   );
}


std::string CDisplayRuleKit::SayRuleKitCaption( void ) const {

   return RuleKitRef.SayKitCaption();
}



/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Histogram-related Implementations

// Histogram "slice" classes

SHistoSliceAnalog::SHistoSliceAnalog(   size_t arg1,     // one-hour depth in cycles of source rainfall
                                        int arg2,        // secsPerCycle of source
                                        ETimeSpan arg3,
                                        bool initSliceForSummingLog ) // See Method Note [3]
                                        :  binSums_analogValue( BinSumsAnalogValue_t() ),
                                           binSums_analogState( BinSumsAnalogState_t() ),
                                           numCyclesInSpan (
                                              (arg3 == ETimeSpan::Histo_movingHour) ?
                                                 arg1 :
                                                 (arg3 == ETimeSpan::Histo_past24hrs) ?
                                                    // See Method Note [2]
                                                    (arg1 * 24u) :
                                                    (arg1 * 168u)
                                           ),
                                           timeOfFrontEdge (0),
                                           count_allCycles (
                                              static_cast<BinSum_t>(numCyclesInSpan)
                                           ),
                                           secsPerCycle (arg2),
                                           span (arg3) {

   // partially initializing slice to show 100% "unavailable" :
   binSums_analogValue.fill(0);  // See Method Note [1]
   binSums_analogState.fill(0);
   if ( ! initSliceForSummingLog ) {
      binSums_analogState[INDEX_BINSUMS_ANALOGSTATE_UNAVAIL] = count_allCycles;
   }
 
/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   binSums_analogValue[bin of first valid sample from source] is not known until runtime, so
      completion of initializing any analog slice is done by CHistogramKitAnalog when it gets first
      active runtime value.

[2]   one hour = e.g., 60 cycles of source, log/rainfall cycle indicies (depths) 0 through 59,
      if source secPerCycle = 60.  So, "span" of moving hour = 60.  Past 24h then = 60*24 = 1440 cycles.
      Since SHistoSlice c-tors are all passed the number of data cycles in a one hour span as an
      argument, they adjust automatically to various data rates.

[3]    

'' End Method Notes ''' */

};

//======================================================================================================/
SHistoSliceFact::SHistoSliceFact( size_t arg1,     // one-hour depth in cycles of source rainfall
                                  int arg2,        // secsPerCycle of source
                                  ETimeSpan arg3,
                                  bool initSliceForSummingLog )
                                  :  binSums_factState ( BinSumsFactState_t() ),
                                     numCyclesInSpan (
                                        (arg3 == ETimeSpan::Histo_movingHour) ?
                                           arg1 :
                                           (arg3 == ETimeSpan::Histo_past24hrs) ?
                                              (arg1 * 24u) : // See Method Note [2]
                                              (arg1 * 168u)
                                     ),
                                     timeOfFrontEdge (0),
                                     count_allCycles (
                                       static_cast<BinSum_t>(numCyclesInSpan)
                                     ),
                                     secsPerCycle (arg2),
                                     span (arg3) {

   // finish initializing slice to show 100% "unavailable" :
   binSums_factState.fill(0);
   if ( ! initSliceForSummingLog ) {
      binSums_factState[INDEX_BINSUMS_FACT_UNAVAIL] = count_allCycles;
   }
};


//======================================================================================================/

SHistoSliceRule::SHistoSliceRule( size_t arg1,     // one-hour depth in cycles of source rainfall
                                  int arg2,        // secsPerCycle of source
                                  ETimeSpan arg3,
                                  bool initSliceForSummingLog  )
                                  :  binSums_ruleState ( BinSumsRuleState_t() ),
                                     numCyclesInSpan (
                                        (arg3 == ETimeSpan::Histo_movingHour) ?
                                           arg1 :
                                           (arg3 == ETimeSpan::Histo_past24hrs) ?
                                              (arg1 * 24u) : // See Method Note [2]
                                              (arg1 * 168u)
                                     ),
                                     timeOfFrontEdge (0),
                                     count_allCycles (static_cast<BinSum_t>(numCyclesInSpan)),
                                     secsPerCycle (arg2),
                                     span (arg3) {

   // finish initializing slice to show 100% "unavailable" :
   binSums_ruleState.fill(0);
   if ( ! initSliceForSummingLog ) {
      binSums_ruleState[INDEX_BINSUMS_RULE_UNAVAIL] = count_allCycles;
   }
};


//======================================================================================================/

SHistoSliceRuleKit::SHistoSliceRuleKit( size_t arg1,  // number of rules in rule kit
                                        size_t arg2,  // one-hour span in cycles
                                        int arg3,
                                        ETimeSpan arg4 )
                                        :  binSums_validsOnEachRule_barsLeftToRight(arg1,0),
                                           binSums_testsOnEachRule_barsLeftToRight(arg1,0),
                                           binSums_failsOnEachRule_barsLeftToRight(arg1,0),
                                           numRulesInKit (arg1),
                                           numCyclesInSpan (
                                             (arg4 == ETimeSpan::Histo_movingHour) ?
                                                 arg2 :
                                                 (arg4 == ETimeSpan::Histo_past24hrs) ?
                                                    (arg2 * 24u) : // See Method Note [2]
                                                    (arg2 * 168u)
                                           ),
                                           timeOfFrontEdge (0),
                                           count_allCycles (static_cast<BinSum_t>(numCyclesInSpan)),
                                           secsPerCycle (arg3),
                                           span (arg4) {
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Histogram abstract base class implementation

// Static memory; Easier to just manually keep these consistent:

std::array<ETimeSpan,3> AHistogram::spansSupported = {   ETimeSpan::Histo_movingHour,
                                                         ETimeSpan::Histo_past24hrs,
                                                         ETimeSpan::Histo_past7days };

// Used only to label set-span options sent to GUI; faster to simply index than via LookUpText(-) 
std::array<std::string,3> AHistogram::spanLabels = {  "Moving hr",
                                                      "Past 24hr",
                                                      "Past 7days" };

//======================================================================================================/

AHistogram::AHistogram( const ASubject& arg0,
                        CPointAnalog& arg1,
                        const std::vector<NGuiKey>& arg2,
                        size_t arg3 )
                        :  IGuiShadow( EApiType::Histogram_analog ),
                           SourceRef (arg1),
                           ViewRef ( arg0.SayViewRef() ),
                           knobKeys_sourceRef (arg2),
                           captionTextLine_sourceInfo ( LookUpTag( arg1.SayLabel() ) ),
                           captionTextLine_barLabelLegend_1 (""),
                           captionTextLine_barLabelLegend_2 (""),
                           movingHourSpanInSourceCycles (arg3),
                           secsPerSourceCycle ( arg1.SaySecsPerCycle() ),
                           modeActive (EInfoMode::Histo_analog_valuesOverValidCycles),
                           spanActive (ETimeSpan::Histo_movingHour),
                           modeActive_index (1),
                           spanActive_index (0),
                           firstCycle (true) {

}

AHistogram::AHistogram( const ASubject& arg0,
                        CFormula& arg1,
                        const std::vector<NGuiKey>& arg2,
                        size_t arg3 )
                        :  IGuiShadow( EApiType::Histogram_analog ),
                           SourceRef (arg1),
                           ViewRef ( arg0.SayViewRef() ),
                           knobKeys_sourceRef (arg2),
                           captionTextLine_sourceInfo ( LookUpTag( arg1.SayLabel() ) ),
                           captionTextLine_barLabelLegend_1 (""),
                           captionTextLine_barLabelLegend_2 (""),
                           movingHourSpanInSourceCycles (arg3),
                           secsPerSourceCycle ( arg1.SaySecsPerCycle() ),
                           modeActive (EInfoMode::Histo_analog_valuesOverValidCycles),
                           spanActive (ETimeSpan::Histo_movingHour),
                           modeActive_index (1),
                           spanActive_index (0),
                           firstCycle (true) {

}

AHistogram::AHistogram( const ASubject& arg0,
                        AFact& arg1,
                        const std::vector<NGuiKey>& arg2,
                        size_t arg3 )
                        :  IGuiShadow( EApiType::Histogram_fact ),
                           SourceRef (arg1),
                           ViewRef ( arg0.SayViewRef() ),
                           knobKeys_sourceRef (arg2),
                           captionTextLine_sourceInfo ( LookUpTag( arg1.SayLabel() ) ),
                           captionTextLine_barLabelLegend_1 (""),
                           captionTextLine_barLabelLegend_2 (""),
                           movingHourSpanInSourceCycles (arg3),
                           secsPerSourceCycle ( arg1.SaySecsPerCycle() ),
                           modeActive (EInfoMode::Histo_fact_statesOverAllCycles),
                           spanActive (ETimeSpan::Histo_movingHour),
                           modeActive_index (0),
                           spanActive_index (0),
                           firstCycle (true) {

}


AHistogram::AHistogram( const ASubject& arg0,
                        CRule& arg1,
                        CRuleKit& arg2,
                        const std::vector<NGuiKey>& arg3,
                        size_t arg4 )
                        :  IGuiShadow( EApiType::Histogram_rule ),
                           SourceRef (arg2),
                           ViewRef ( arg0.SayViewRef() ),
                           knobKeys_sourceRef (arg3),
                           captionTextLine_sourceInfo (  arg0.SayNameAsText() +
                                                         ":" + arg2.SayKitSgiOfNumberAsText() + " :" +
                                                         arg1.SayRuleUaiText() // includes "Rule-"
                           ),
                           captionTextLine_barLabelLegend_1
                              ("Mark: F=fail, P=pass, S=skip, X=invalid, U=unavail"),
                           captionTextLine_barLabelLegend_2 ("Mode: a=auto, c=case, i=idle"), 
                           movingHourSpanInSourceCycles (arg4),
                           secsPerSourceCycle ( arg2.SaySecsPerCycle() ),
                           modeActive (EInfoMode::Histo_rule_statesOverAllCycles),
                           spanActive (ETimeSpan::Histo_movingHour),
                           modeActive_index (0),
                           spanActive_index (0),
                           firstCycle (true) {

}



AHistogram::AHistogram( const ASubject& arg0,
                        CRuleKit& arg1,
                        const std::vector<NGuiKey>& arg2,
                        size_t arg3 )
                        :  IGuiShadow( EApiType::Histogram_ruleKit ),
                           SourceRef (arg1),
                           ViewRef ( arg0.SayViewRef() ),
                           knobKeys_sourceRef (arg2),
                           captionTextLine_sourceInfo ( arg1.SayKitCaption() ),
                           captionTextLine_barLabelLegend_1 (""),
                           captionTextLine_barLabelLegend_2 (""),
                           movingHourSpanInSourceCycles (arg3),
                           secsPerSourceCycle ( arg1.SaySecsPerCycle() ),
                           modeActive (EInfoMode::Histo_ruleKit_failsOverTests),
                           spanActive (ETimeSpan::Histo_movingHour),
                           modeActive_index (0),
                           spanActive_index (0),
                           firstCycle (true) {

}

AHistogram::~AHistogram( void ) {

   // empty d-tor. Memory released by concrete subclasses
}


std::vector<std::string> AHistogram::GenerateCaptionText( time_t frontEdgeTime ) {

/*
   Recall from .hpp:
   caption[0] = top line, from captionTextLine_sourceInfo field
   caption[1] = second line, mode of histogram (a constant only for Facts (which have one mode)) 
   caption[2] = third line, span of histogram (lookback time) per ETimeSpan defns available
   caption[3] = fourth line, saying time of histogram "front edge"
   caption[4] = captionTextLine_barLabelLegend_1, which can be empty
   caption[5] = captionTextLine_barLabelLegend_2, which can be empty
*/
   std::string frontEdgeTimeAsText = ""; 
   WriteTimestampAsTextTo( frontEdgeTime, frontEdgeTimeAsText );
   
   // enable RVO move semantics by constructing inside the return
   return std::vector<std::string>( {  captionTextLine_sourceInfo,
                                       "Showing: " + LookUpText( modeActive ),
                                       "Look back: " + LookUpText( spanActive ),
                                       "From: " + frontEdgeTimeAsText,
                                        captionTextLine_barLabelLegend_1,
                                        captionTextLine_barLabelLegend_2 } );
}


EGuiReply AHistogram::SetSpanActiveToOptionIndex( size_t indexGivenByUser ) {

      if (  ! ( indexGivenByUser < spansSupported.size() ) ||
            std::find(  spansSupported.begin(),
                        spansSupported.end(),
                        spansSupported[indexGivenByUser] )
            == spansSupported.end() ) {
         return EGuiReply::FAIL_set_givenValueOutOfRangeAllowed;
      }
      spanActive_index = indexGivenByUser;
      spanActive = spansSupported[indexGivenByUser];
      return EGuiReply::OKAY_allDone;
}

std::string AHistogram::SayIdentifyingText( void ) const { return captionTextLine_sourceInfo; }


EGuiReply AHistogram::SetModeActiveToOptionIndex( size_t indexGivenByUser ) {

   return EGuiReply::OKAY_allDone;
}



//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for histograms owned by analog-handling sources (points, formula, charts)

std::array<EInfoMode,2> CHistogramAnalog::modesSupported_analog =
   {  EInfoMode::Histo_analog_statesOverAllCycles,
      EInfoMode::Histo_analog_valuesOverValidCycles };

std::array<std::string,2> CHistogramAnalog::modeLabels_analog = { "Analog State", "Analog Value" };

std::array<std::string,FIXED_RAIN_ANALOGSTATE_NUMBINS> CHistogramAnalog::barLabels_analogStates =
   GUILABELS_ANALOGSTATEBINS;


CHistogramAnalog::CHistogramAnalog( const ASubject& bArg0,
                                    CPointAnalog& bArg1,
                                    const std::vector<NGuiKey>& bArg2,
                                    size_t bArg3,
                                    float arg1,
                                    float arg2 )
                                    :  AHistogram( bArg0,
                                                   bArg1,
                                                   bArg2,
                                                   bArg3                                                         
                                       ),
                                       sliceLog_past24clockHrs (
                                          std::deque<SHistoSliceAnalog>(
                                             24,
                                             SHistoSliceAnalog(  bArg3,
                                                                 SourceRef.SaySecsPerCycle(),
                                                                 ETimeSpan::Histo_movingHour
                                             )
                                          )
                                       ),
                                       sliceLog_past7calendarDays (
                                          std::deque<SHistoSliceAnalog>(
                                             7,
                                             SHistoSliceAnalog(  bArg3,
                                                                 SourceRef.SaySecsPerCycle(),
                                                                 ETimeSpan::Histo_past24hrs
                                             )
                                          )
                                       ),
                                       realtimeSlice_movingHour ( SHistoSliceAnalog(
                                                                     bArg3,
                                                                     SourceRef.SaySecsPerCycle(),
                                                                     ETimeSpan::Histo_movingHour
                                                                 )
                                       ),
                                       labelLeftBar_analogValue (arg1),
                                       labelIncrPerBar_analogValue (arg2) {

   ViewRef.AddHistogram( *this );
}

CHistogramAnalog::CHistogramAnalog( const ASubject& bArg0,
                                    CFormula& bArg1,
                                    const std::vector<NGuiKey>& bArg2,
                                    size_t bArg3,
                                    float arg1,
                                    float arg2 )
                                    :  AHistogram( bArg0,
                                                   bArg1,
                                                   bArg2,
                                                   bArg3                                                         
                                       ),
                                       sliceLog_past24clockHrs (
                                          std::deque<SHistoSliceAnalog>(
                                             24,
                                             SHistoSliceAnalog(  bArg3,
                                                                 SourceRef.SaySecsPerCycle(),
                                                                 ETimeSpan::Histo_movingHour )
                                          )
                                      ),
                                      sliceLog_past7calendarDays (
                                          std::deque<SHistoSliceAnalog>(
                                             7,
                                             SHistoSliceAnalog(  bArg3,
                                                                 SourceRef.SaySecsPerCycle(),
                                                                 ETimeSpan::Histo_past24hrs
                                             )
                                          )
                                       ),
                                       realtimeSlice_movingHour ( SHistoSliceAnalog(
                                                                     bArg3,
                                                                     SourceRef.SaySecsPerCycle(),
                                                                     ETimeSpan::Histo_movingHour
                                                                 )
                                       ),
                                       labelLeftBar_analogValue (arg1),
                                       labelIncrPerBar_analogValue (arg2) {

   ViewRef.AddHistogram( *this );
}
    

CHistogramAnalog::~CHistogramAnalog( void ) {

}


std::vector<GuiFpn_t>
CHistogramAnalog::GenerateBarHeightsFromSlice( const SHistoSliceAnalog& sliceRef ) {

   if ( modeActive == EInfoMode::Histo_analog_statesOverAllCycles ) {

      std::vector<GuiFpn_t> reply( sliceRef.binSums_analogState.size(), 0.0f );

      std::transform(   sliceRef.binSums_analogState.begin(),
                        sliceRef.binSums_analogState.end(),
                        reply.begin(),
                        std::bind2nd(  std::divides<float>(),
                                       sliceRef.count_allCycles )
      );
      return reply; 
   }
   else {

      std::vector<GuiFpn_t> reply( sliceRef.binSums_analogValue.size(), 0.0f );

      std::transform(   sliceRef.binSums_analogValue.begin(),
                        sliceRef.binSums_analogValue.end(),
                        reply.begin(),
                        std::bind2nd(  std::divides<float>(),
                                       sliceRef.count_allCycles )
      );
      return reply; 
   }
}

//======================================================================================================/

SHistoSliceAnalog CHistogramAnalog::GenerateSliceSummedOnPast24hrs( void ) {

    SHistoSliceAnalog summedSlice_24hrs( movingHourSpanInSourceCycles,
                                         secsPerSourceCycle,
                                         ETimeSpan::Histo_past24hrs,
                                         true
   );

   for ( const auto& clockHourSliceByCref : sliceLog_past24clockHrs ) {

      std::transform(   clockHourSliceByCref.binSums_analogState.begin(),
                        clockHourSliceByCref.binSums_analogState.end(),
                        summedSlice_24hrs.binSums_analogState.begin(),
                        summedSlice_24hrs.binSums_analogState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
      std::transform(   clockHourSliceByCref.binSums_analogValue.begin(),
                        clockHourSliceByCref.binSums_analogValue.end(),
                        summedSlice_24hrs.binSums_analogValue.begin(),
                        summedSlice_24hrs.binSums_analogValue.begin(),
                        std::plus<BinSum_t>()
      );            
   }
   summedSlice_24hrs.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;

   return summedSlice_24hrs; // Counting on compiler doing NRVO to avoid chain of copies
}

//======================================================================================================/

SHistoSliceAnalog CHistogramAnalog::GenerateSliceSummedOnPast7days( void ) {

    SHistoSliceAnalog summedSlice_7days( movingHourSpanInSourceCycles,
                                         secsPerSourceCycle,
                                         ETimeSpan::Histo_past7days,
                                         true
   );

   for ( const auto& calendarDaySliceByCref : sliceLog_past7calendarDays ) {

      std::transform(   calendarDaySliceByCref.binSums_analogState.begin(),
                        calendarDaySliceByCref.binSums_analogState.end(),
                        summedSlice_7days.binSums_analogState.begin(),
                        summedSlice_7days.binSums_analogState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
      std::transform(   calendarDaySliceByCref.binSums_analogValue.begin(),
                        calendarDaySliceByCref.binSums_analogValue.end(),
                        summedSlice_7days.binSums_analogValue.begin(),
                        summedSlice_7days.binSums_analogValue.begin(),
                        std::plus<BinSum_t>()
      );            
   }
   summedSlice_7days.timeOfFrontEdge = sliceLog_past7calendarDays.front().timeOfFrontEdge;

   return summedSlice_7days; // Counting on compiler doing NRVO to avoid chain of copies
}


 void CHistogramAnalog::InitializeAnalogSliceOnValue( SHistoSliceAnalog& sliceRef,
                                                      Bindex_t value ) {
   sliceRef.binSums_analogValue.fill(0);
   sliceRef.binSums_analogValue[ value ] = sliceRef.count_allCycles;
   return;
}

//======================================================================================================/

EGuiReply CHistogramAnalog::SetModeActiveToOptionIndex( size_t indexGivenByUser ) {

      if (  ! ( indexGivenByUser < modesSupported_analog.size() ) ||
            std::find(  modesSupported_analog.begin(),
                        modesSupported_analog.end(),
                        modesSupported_analog[indexGivenByUser] )
            == modesSupported_analog.end() ) {
         return EGuiReply::FAIL_set_givenHistogramModeNotAvailable;
      }
      modeActive_index = indexGivenByUser;
      modeActive = modesSupported_analog[indexGivenByUser];
      return EGuiReply::OKAY_allDone;
}

//======================================================================================================/

GuiPackHistogram_t CHistogramAnalog::SayGuiPack( void ) {

   // See Method Note [1] regarding line below
   SHistoSliceAnalog sliceShowing( (spanActive == ETimeSpan::Histo_movingHour) ?
                                      realtimeSlice_movingHour :
                                      (spanActive == ETimeSpan::Histo_past24hrs) ?
                                         GenerateSliceSummedOnPast24hrs() :
                                         GenerateSliceSummedOnPast7days() // assumed as 3rd span
   );

   if ( modeActive == EInfoMode::Histo_analog_statesOverAllCycles ) {

      return SGuiPackHistogram(  EGuiType::Histogram_analog,
                                 ownGuiKey, 
                                 EGuiType::HistogramBars_analogStates,
                                 sliceShowing.binSums_analogState.size(),
                                 GenerateCaptionText( sliceShowing.timeOfFrontEdge ),
                                 GenerateBarHeightsFromSlice( sliceShowing ),
                                 NaNFLOAT,
                                 NaNFLOAT,
                                 std::vector<std::string>(  barLabels_analogStates.begin(),
                                                            barLabels_analogStates.end()
                                 ),
                                 std::vector<std::string>(  modeLabels_analog.begin(),
                                                            modeLabels_analog.end()
                                 ),
                                 std::vector<std::string>(  spanLabels.begin(),
                                                            spanLabels.end()
                                 ),
                                 modeActive_index,
                                 spanActive_index,
                                 std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                         knobKeys_sourceRef.end()
                                 )
      );
   }
   // Only other analog mode possible is "values"
   return SGuiPackHistogram(  EGuiType::Histogram_analog,
                              ownGuiKey,
                              EGuiType::HistogramBars_analogBins,
                              sliceShowing.binSums_analogValue.size(),
                              GenerateCaptionText( sliceShowing.timeOfFrontEdge ),
                              GenerateBarHeightsFromSlice( sliceShowing ),
                              labelLeftBar_analogValue,
                              labelIncrPerBar_analogValue,
                              std::vector<std::string>(0),
                              std::vector<std::string>(  modeLabels_analog.begin(),
                                                         modeLabels_analog.end()
                              ),
                              std::vector<std::string>(  spanLabels.begin(),
                                                         spanLabels.end()
                              ),
                              modeActive_index,
                              spanActive_index,
                              std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                      knobKeys_sourceRef.end()
                              )
   );

/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Call to (compiler-generated) copy-constructor necessary due to const members in the slice classes,
      making assignments (copy or move) impossible. Since slices are small objects, this is not seen as
      signif slowdown compared to hazard of inadvertent chg to some slice params.
      $$$ TBD to validate that assumption $$$ (typ for all slice classes)
 
'' End Method Notes ''''' */                                    
}

//======================================================================================================/ 

void CHistogramAnalog::Cycle( time_t newTimestampForFrontEdge,
                              bool beginNewClockHour,
                              bool beginNewCalendarDay,
                              Bindex_t bindexOfStateEnteringFrontEdge,
                              Bindex_t bindexOfStateLeavingOneHourEdge,
                              Bindex_t bindexOfValueEnteringFrontEdge,
                              Bindex_t bindexOfValueLeavingOneHourEdge ) {


   if ( firstCycle ) {

      // First, analog histogram does same runtime values initialization as performed by analog rainfall
      // (This is not required for analog state, or Fact or Rule slices, which don't need runtime init.)

      InitializeAnalogSliceOnValue( realtimeSlice_movingHour, bindexOfValueEnteringFrontEdge );

      for ( auto& sliceRef : sliceLog_past24clockHrs ) {
         InitializeAnalogSliceOnValue( sliceRef, bindexOfValueEnteringFrontEdge );
      }          
      for ( auto& sliceRef : sliceLog_past7calendarDays ) {
         InitializeAnalogSliceOnValue( sliceRef, bindexOfValueEnteringFrontEdge );
      }

      // load s/u timestamp into front edge of both slice logs
      sliceLog_past24clockHrs.front().timeOfFrontEdge = newTimestampForFrontEdge;
      sliceLog_past7calendarDays.front().timeOfFrontEdge = newTimestampForFrontEdge;

      firstCycle = false;
   }

   // See Method Note [1]

   if ( beginNewClockHour ) {

      // most recent slice is at lowest index ("front"); oldest at highest index ("back")
      sliceLog_past24clockHrs.pop_back();
      sliceLog_past24clockHrs.push_front( realtimeSlice_movingHour );
   }
   if ( beginNewCalendarDay ) {

      sliceLog_past7calendarDays.pop_back();
      sliceLog_past7calendarDays.push_front( GenerateSliceSummedOnPast24hrs() );
   }       
       
   // Update moving hour, see Method Note [2]

   ++realtimeSlice_movingHour.binSums_analogState[ bindexOfStateEnteringFrontEdge ];
   --realtimeSlice_movingHour.binSums_analogState[ bindexOfStateLeavingOneHourEdge ];
   ++realtimeSlice_movingHour.binSums_analogValue[ bindexOfValueEnteringFrontEdge ];
   --realtimeSlice_movingHour.binSums_analogValue[ bindexOfValueLeavingOneHourEdge ];
   realtimeSlice_movingHour.timeOfFrontEdge = newTimestampForFrontEdge;

   return;

/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   A new clock hour or new calendar day means the arriving data is part of that hour or day, not the
      completed one.  So, actions to save the hour or day are taken PRIOR to updating the moving hour.

[2]   No 'safeties' on these unsigned subtractions, as could never call a subtract on a zero binSum
      (I hope!)

'' End Method Notes ''''' */

}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for histograms owned by an object of an AFact subclass

std::array<EInfoMode,1> CHistogramFact::modesSupported_fact =
   { EInfoMode::Histo_fact_statesOverAllCycles };

std::array<std::string,1> CHistogramFact::modeLabels_fact = { "Fact State" };

std::array<std::string, FIXED_RAIN_FACTSTATE_NUMBINS> CHistogramFact::barLabels_fact =
   GUILABELS_FACTBINS;


CHistogramFact::CHistogramFact(  const ASubject& bArg0,
                                 AFact& bArg1,
                                 const std::vector<NGuiKey>& bArg2,
                                 size_t bArg3 )
                                 :  AHistogram( bArg0,
                                                bArg1,
                                                bArg2,
                                                bArg3                                                         
                                    ),
                                    sliceLog_past24clockHrs (
                                       std::deque<SHistoSliceFact>(
                                          24,
                                          SHistoSliceFact( bArg3,
                                                               SourceRef.SaySecsPerCycle(),
                                                               ETimeSpan::Histo_movingHour )
                                       )
                                    ),
                                    sliceLog_past7calendarDays (
                                       std::deque<SHistoSliceFact>(
                                          7,
                                          SHistoSliceFact( bArg3,
                                                               SourceRef.SaySecsPerCycle(),
                                                               ETimeSpan::Histo_past24hrs )
                                       )
                                    ),
                                    realtimeSlice_movingHour ( SHistoSliceFact(
                                                                  bArg3,
                                                                  SourceRef.SaySecsPerCycle(),
                                                                  ETimeSpan::Histo_movingHour
                                                               )
                                    ) {

  ViewRef.AddHistogram( *this );
}


CHistogramFact::~CHistogramFact( void ) {

}

//======================================================================================================/

std::vector<GuiFpn_t>
CHistogramFact::GenerateBarHeightsFromSlice( const SHistoSliceFact& sliceRef ) {

   std::vector<GuiFpn_t> reply( sliceRef.binSums_factState.size(), 0.0f );

   std::transform(   sliceRef.binSums_factState.begin(),
                     sliceRef.binSums_factState.end(),
                     reply.begin(),
                     std::bind2nd(  std::divides<float>(),
                                    sliceRef.count_allCycles )
   );
   return reply;
}

//======================================================================================================/

SHistoSliceFact CHistogramFact::GenerateSliceSummedOnPast24hrs( void ) {

   SHistoSliceFact summedSlice_24hrs( movingHourSpanInSourceCycles,
                                      secsPerSourceCycle,
                                      ETimeSpan::Histo_past24hrs,
                                      true
   );

   for ( const auto& clockHourSliceByCref : sliceLog_past24clockHrs ) {

      std::transform(   clockHourSliceByCref.binSums_factState.begin(),
                        clockHourSliceByCref.binSums_factState.end(),
                        summedSlice_24hrs.binSums_factState.begin(),
                        summedSlice_24hrs.binSums_factState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
   }
   summedSlice_24hrs.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;
   return summedSlice_24hrs;
}

//======================================================================================================/

SHistoSliceFact CHistogramFact::GenerateSliceSummedOnPast7days( void ) {

   SHistoSliceFact summedSlice_7days( movingHourSpanInSourceCycles,
                                      secsPerSourceCycle,
                                      ETimeSpan::Histo_past7days,
                                      true
   );

   for ( const auto& calendarDaySliceByCref : sliceLog_past7calendarDays ) {

      std::transform(   calendarDaySliceByCref.binSums_factState.begin(),
                        calendarDaySliceByCref.binSums_factState.end(),
                        summedSlice_7days.binSums_factState.begin(),
                        summedSlice_7days.binSums_factState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
   }
   summedSlice_7days.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;
   return summedSlice_7days;
}

//======================================================================================================/

GuiPackHistogram_t CHistogramFact::SayGuiPack( void ) {

    SHistoSliceFact sliceShowing(  (spanActive == ETimeSpan::Histo_movingHour) ?
                                      realtimeSlice_movingHour :
                                      (spanActive == ETimeSpan::Histo_past24hrs) ?
                                         GenerateSliceSummedOnPast24hrs() :
                                         GenerateSliceSummedOnPast7days() // assumed as 3rd span
   );

   // Only mode possible for a Fact source is EInfoMode::Fact_statesOverAllCycles

   return SGuiPackHistogram(  EGuiType::Histogram_fact,
                              ownGuiKey,
                              EGuiType::HistogramBars_factStates,
                              sliceShowing.binSums_factState.size(),
                              GenerateCaptionText( sliceShowing.timeOfFrontEdge ),
                              GenerateBarHeightsFromSlice( sliceShowing ),
                              NaNFLOAT,
                              NaNFLOAT,
                              std::vector<std::string>(  barLabels_fact.begin(),
                                                         barLabels_fact.end()
                              ),
                              std::vector<std::string>(  modeLabels_fact.begin(),
                                                         modeLabels_fact.end()
                              ),
                              std::vector<std::string>(  spanLabels.begin(),
                                                         spanLabels.end()
                              ),
                              modeActive_index,
                              spanActive_index,
                              std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                      knobKeys_sourceRef.end()
                              )
   );                                    
} 

//======================================================================================================/

void CHistogramFact::Cycle(   time_t newTimestampForFrontEdge,
                                 bool beginNewClockHour,
                                 bool beginNewCalendarDay,
                                 Bindex_t bindexOfStateEnteringFrontEdge,
                                 Bindex_t bindexOfStateLeavingOneHourEdge ) {

   if ( firstCycle ) {

      // load s/u timestamp into front edge of both slice logs
      sliceLog_past24clockHrs.front().timeOfFrontEdge = newTimestampForFrontEdge;
      sliceLog_past7calendarDays.front().timeOfFrontEdge = newTimestampForFrontEdge;

      firstCycle = false;
   }

   // See Method Note [1]

   if ( beginNewClockHour ) {

      // most recent slice is at lowest index ("front"); oldest at highest index ("back")
      sliceLog_past24clockHrs.pop_back();
      sliceLog_past24clockHrs.push_front( realtimeSlice_movingHour );
   }
   if ( beginNewCalendarDay ) {

      sliceLog_past7calendarDays.pop_back();
      sliceLog_past7calendarDays.push_front( GenerateSliceSummedOnPast24hrs() );
   }       
       
   // Update moving hour, see Method Note [2]

   ++realtimeSlice_movingHour.binSums_factState[ bindexOfStateEnteringFrontEdge ];
   --realtimeSlice_movingHour.binSums_factState[ bindexOfStateLeavingOneHourEdge ];
   realtimeSlice_movingHour.timeOfFrontEdge = newTimestampForFrontEdge;

   return;

/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   A new clock hour or new calendar day means the arriving data is part of that hour or day, not the
      completed one.  So, actions to save the hour or day are taken PRIOR to updating the moving hour.

[2]   No 'safeties' on these unsigned subtractions, as could never call a subtract on a zero binSum
      (I hope!)

'' End Method Notes ''''' */

}

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for histograms owned by a CRule object


std::array<EInfoMode,1> CHistogramRule::modesSupported_rule =
   { EInfoMode::Histo_rule_statesOverAllCycles };

std::array<std::string,1> CHistogramRule::modeLabels_rule = { "Rule State" };

std::array<std::string, FIXED_RAIN_RULESTATE_NUMBINS> CHistogramRule::barLabels_rule =
   GUILABELS_RULEBINS;


CHistogramRule::CHistogramRule(  const ASubject& bArg0,
                                 CRule& bArg1,
                                 CRuleKit& bArg2,
                                 const std::vector<NGuiKey>& bArg3,
                                 size_t bArg4 )
                                 :  AHistogram( bArg0,
                                                bArg1,
                                                bArg2,
                                                bArg3,
                                                bArg4                                                         
                                    ),
                                    sliceLog_past24clockHrs (
                                       std::deque<SHistoSliceRule>(
                                          24,
                                          SHistoSliceRule( bArg4,
                                                           SourceRef.SaySecsPerCycle(),
                                                           ETimeSpan::Histo_movingHour
                                          )
                                       )
                                    ),
                                    sliceLog_past7calendarDays (
                                       std::deque<SHistoSliceRule>(
                                          7,
                                          SHistoSliceRule( bArg4,
                                                           SourceRef.SaySecsPerCycle(),
                                                           ETimeSpan::Histo_past24hrs
                                          )
                                       )
                                    ),
                                    realtimeSlice_movingHour ( SHistoSliceRule(
                                                                  bArg4,
                                                                  SourceRef.SaySecsPerCycle(),
                                                                  ETimeSpan::Histo_movingHour
                                                               )
                                    ) {

   ViewRef.AddHistogram( *this );
}


CHistogramRule::~CHistogramRule( void ) {

}


//======================================================================================================/

std::vector<GuiFpn_t>
CHistogramRule::GenerateBarHeightsFromSlice( const SHistoSliceRule& sliceRef ) {

   std::vector<GuiFpn_t> reply( sliceRef.binSums_ruleState.size(), 0.0f );

   std::transform(   sliceRef.binSums_ruleState.begin(),
                     sliceRef.binSums_ruleState.end(),
                     reply.begin(),
                     std::bind2nd(  std::divides<float>(),
                                    sliceRef.count_allCycles ) // non-zero const
   );
   return reply;
}

//======================================================================================================/

SHistoSliceRule CHistogramRule::GenerateSliceSummedOnPast24hrs( void ) {

   SHistoSliceRule summedSlice_24hrs( movingHourSpanInSourceCycles,
                                      secsPerSourceCycle,
                                      ETimeSpan::Histo_past24hrs,
                                      true
   );

   for ( const auto& clockHourSliceByCref : sliceLog_past24clockHrs ) {

      std::transform(   clockHourSliceByCref.binSums_ruleState.begin(),
                        clockHourSliceByCref.binSums_ruleState.end(),
                        summedSlice_24hrs.binSums_ruleState.begin(),
                        summedSlice_24hrs.binSums_ruleState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
   }
   summedSlice_24hrs.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;
   return summedSlice_24hrs;
}

//======================================================================================================/

SHistoSliceRule CHistogramRule::GenerateSliceSummedOnPast7days( void ) {

   SHistoSliceRule summedSlice_7days( movingHourSpanInSourceCycles,
                                      secsPerSourceCycle,
                                      ETimeSpan::Histo_past7days,
                                      true
   );

   for ( const auto& calendarDaySliceByCref : sliceLog_past7calendarDays ) {

      std::transform(   calendarDaySliceByCref.binSums_ruleState.begin(),
                        calendarDaySliceByCref.binSums_ruleState.end(),
                        summedSlice_7days.binSums_ruleState.begin(),
                        summedSlice_7days.binSums_ruleState.begin(), // begin(), as this is dest
                        std::plus<BinSum_t>()
      );
   }
   summedSlice_7days.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;
   return summedSlice_7days;
}

//======================================================================================================/

GuiPackHistogram_t CHistogramRule::SayGuiPack( void ) {

   SHistoSliceRule sliceShowing(   (spanActive == ETimeSpan::Histo_movingHour) ?
                                      realtimeSlice_movingHour :
                                      (spanActive == ETimeSpan::Histo_past24hrs) ?
                                         GenerateSliceSummedOnPast24hrs() :
                                         GenerateSliceSummedOnPast7days() // assumed as 3rd span
   );

   // Only mode possible for a Fact source is EInfoMode::Fact_statesOverAllCycles

   return SGuiPackHistogram(  EGuiType::Histogram_rule,
                              ownGuiKey,
                              EGuiType::HistogramBars_ruleStates,
                              sliceShowing.binSums_ruleState.size(),
                              GenerateCaptionText( sliceShowing.timeOfFrontEdge ),
                              GenerateBarHeightsFromSlice( sliceShowing ),
                              NaNFLOAT,
                              NaNFLOAT,
                              std::vector<std::string>(  barLabels_rule.begin(),
                                                         barLabels_rule.end()
                              ),
                              std::vector<std::string>(  modeLabels_rule.begin(),
                                                         modeLabels_rule.end()
                              ),
                              std::vector<std::string>(  spanLabels.begin(),
                                                         spanLabels.end()
                              ),
                              modeActive_index,
                              spanActive_index,
                              std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                      knobKeys_sourceRef.end()
                              )
   );                                    
} 

//======================================================================================================/

void CHistogramRule::Cycle(   time_t newTimestampForFrontEdge,
                                 bool beginNewClockHour,
                                 bool beginNewCalendarDay,
                                 Bindex_t bindexOfStateEnteringFrontEdge,
                                 Bindex_t bindexOfStateLeavingOneHourEdge ) {

   if ( firstCycle ) {

      // load s/u timestamp into front edge of both slice logs
      sliceLog_past24clockHrs.front().timeOfFrontEdge = newTimestampForFrontEdge;
      sliceLog_past7calendarDays.front().timeOfFrontEdge = newTimestampForFrontEdge;
      firstCycle = false;
   }

   // See Method Note [1]

   if ( beginNewClockHour ) {

      // most recent slice is at lowest index ("front"); oldest at highest index ("back")
      sliceLog_past24clockHrs.pop_back();
      sliceLog_past24clockHrs.push_front( realtimeSlice_movingHour );
   }
   if ( beginNewCalendarDay ) {

      sliceLog_past7calendarDays.pop_back();
      sliceLog_past7calendarDays.push_front( GenerateSliceSummedOnPast24hrs() );
   }       
       
   // Update moving hour, see Method Note [2]

   ++realtimeSlice_movingHour.binSums_ruleState[ bindexOfStateEnteringFrontEdge ];
   --realtimeSlice_movingHour.binSums_ruleState[ bindexOfStateLeavingOneHourEdge ];
   realtimeSlice_movingHour.timeOfFrontEdge = newTimestampForFrontEdge;

   return;

/* Start Method Notes ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   A new clock hour or new calendar day means the arriving data is part of that hour or day, not the
      completed one.  So, actions to save the hour or day are taken PRIOR to updating the moving hour.

[2]   No 'safeties' on these unsigned subtractions, as could never call a subtract on a zero binSum
      (I hope!)

'' End Method Notes ''''' */

}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Concrete class for histogram owned by a CRuleKit object

std::array<EInfoMode,3> CHistogramRuleKit::modesSupported_ruleKit =
   {  EInfoMode::Histo_ruleKit_failsOverTests,
      EInfoMode::Histo_ruleKit_testsOverValidCycles,
      EInfoMode::Histo_ruleKit_validCyclesOverAllCycles };

std::array<std::string,3> CHistogramRuleKit::modeLabels_ruleKit = {  "Fails/Tests",
                                                                     "Tests/Cycles Valid",
                                                                     "Cycles Valid/Cycles" };
// "Fails" is summed across all three Rule modes: Auto, Case, and Idle.
// "Cycles" is count of all source object cycles, incl. those with input data invalid or unavailable.
// Bar labels for CHistogramKitRuleKit cannot be static, but are runtime information. 


CHistogramRuleKit::CHistogramRuleKit(  const ASubject& bArg0,
                                       CRuleKit& bArg1,
                                       const std::vector<NGuiKey>& bArg2,
                                       size_t bArg3,                       // one-hour depth
                                       const std::vector<Nzint_t>& arg0 )  // rule Uai
                                       :  AHistogram( bArg0,
                                                      bArg1,
                                                      bArg2,
                                                      bArg3                                                         
                                          ),
                                          barLabels_ruleIdentifiers (
                                             ConfigureBarLabelsFromRuleKit( arg0 )
                                          ),
                                          ruleUais_barsLeftToRight (arg0),
                                          numRulesInKit ( arg0.size() ),
                                          sliceLog_past24clockHrs (
                                             std::deque<SHistoSliceRuleKit>(
                                                24,
                                                SHistoSliceRuleKit(
                                                   arg0.size(),
                                                   bArg3,
                                                   SourceRef.SaySecsPerCycle(),
                                                   ETimeSpan::Histo_movingHour
                                                )
                                             )
                                          ),
                                          sliceLog_past7calendarDays (
                                             std::deque<SHistoSliceRuleKit>(
                                                7,
                                                SHistoSliceRuleKit(
                                                   arg0.size(),
                                                   bArg3,
                                                   SourceRef.SaySecsPerCycle(),
                                                   ETimeSpan::Histo_past24hrs
                                                )
                                             )
                                          ),
                                          realtimeSlice_movingHour ( SHistoSliceRuleKit(
                                                                        arg0.size(),
                                                                        bArg3,
                                                                        SourceRef.SaySecsPerCycle(),
                                                                        ETimeSpan::Histo_movingHour )
                                          ) {

   ViewRef.AddHistogram( *this );
}


CHistogramRuleKit::~CHistogramRuleKit( void ) {

}

//======================================================================================================/


std::vector<std::string>
CHistogramRuleKit::ConfigureBarLabelsFromRuleKit( const std::vector<Nzint_t>& ruleUaiInGuiOrder ) {

   std::vector<std::string> reply(0);

   for ( const auto& uaiCiter : ruleUaiInGuiOrder ) {

      reply.push_back( "R-" + std::to_string( uaiCiter ) );
   }
   return reply;
}

//======================================================================================================/   


std::vector<GuiFpn_t>
CHistogramRuleKit::GenerateBarHeightsFromSlice( const SHistoSliceRuleKit& sliceRef ) {

   std::vector<GuiFpn_t> reply( sliceRef.binSums_failsOnEachRule_barsLeftToRight.size(), 0.0f);

   switch ( modeActive ) {

      case ( EInfoMode::Histo_ruleKit_failsOverTests ) :

         std::transform(   sliceRef.binSums_failsOnEachRule_barsLeftToRight.begin(),
                           sliceRef.binSums_failsOnEachRule_barsLeftToRight.end(),
                           sliceRef.binSums_testsOnEachRule_barsLeftToRight.begin(),
                           reply.begin(),
                           FDivideIfDefinedElseZero<GuiFpn_t>()
         ); 
         break;

      case ( EInfoMode::Histo_ruleKit_testsOverValidCycles ) :

         std::transform(   sliceRef.binSums_testsOnEachRule_barsLeftToRight.begin(),
                           sliceRef.binSums_testsOnEachRule_barsLeftToRight.end(),
                           sliceRef.binSums_validsOnEachRule_barsLeftToRight.begin(),
                           reply.begin(),
                           FDivideIfDefinedElseZero<GuiFpn_t>()
         ); 
         break;

      case ( EInfoMode::Histo_ruleKit_validCyclesOverAllCycles ) :

         // won't div-by-zero, as count_allCycles is nonzero const

         std::transform(   sliceRef.binSums_validsOnEachRule_barsLeftToRight.begin(),
                           sliceRef.binSums_validsOnEachRule_barsLeftToRight.end(),
                           reply.begin(),
                           std::bind2nd(  std::divides<float>(),
                                          sliceRef.count_allCycles )
         ); 
         break;
   }
   return reply;
}


//======================================================================================================/

EGuiReply CHistogramRuleKit::SetModeActiveToOptionIndex( size_t indexGivenByUser ) {

      if (  ! ( indexGivenByUser < modesSupported_ruleKit.size() ) ||
            std::find(  modesSupported_ruleKit.begin(),
                        modesSupported_ruleKit.end(),
                        modesSupported_ruleKit[indexGivenByUser] )
            == modesSupported_ruleKit.end() ) {
         return EGuiReply::FAIL_set_givenHistogramModeNotAvailable;
      }
      modeActive_index = indexGivenByUser;
      modeActive = modesSupported_ruleKit[indexGivenByUser];
      return EGuiReply::OKAY_allDone;
}

//======================================================================================================/

SHistoSliceRuleKit CHistogramRuleKit::GenerateSliceSummedOnPast24hrs( void ) {

   SHistoSliceRuleKit summedSlice_24hrs( numRulesInKit,
                                         movingHourSpanInSourceCycles,
                                         secsPerSourceCycle,
                                         ETimeSpan::Histo_past24hrs
   );

   for ( const auto& clockHourSliceByCref : sliceLog_past24clockHrs ) {

      std::transform(   clockHourSliceByCref.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        clockHourSliceByCref.binSums_validsOnEachRule_barsLeftToRight.end(),
                        summedSlice_24hrs.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_24hrs.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );

      std::transform(   clockHourSliceByCref.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        clockHourSliceByCref.binSums_testsOnEachRule_barsLeftToRight.end(),
                        summedSlice_24hrs.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_24hrs.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );

      std::transform(   clockHourSliceByCref.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        clockHourSliceByCref.binSums_failsOnEachRule_barsLeftToRight.end(),
                        summedSlice_24hrs.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_24hrs.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );
   }

   summedSlice_24hrs.timeOfFrontEdge = sliceLog_past24clockHrs.front().timeOfFrontEdge;

   return summedSlice_24hrs;
}

//======================================================================================================/

SHistoSliceRuleKit CHistogramRuleKit::GenerateSliceSummedOnPast7days( void ) {

   SHistoSliceRuleKit summedSlice_7days( numRulesInKit,
                                         movingHourSpanInSourceCycles,
                                         secsPerSourceCycle,
                                         ETimeSpan::Histo_past7days
   );

   for ( const auto& calendarDaySliceByCref : sliceLog_past7calendarDays ) {

      std::transform(   calendarDaySliceByCref.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        calendarDaySliceByCref.binSums_validsOnEachRule_barsLeftToRight.end(),
                        summedSlice_7days.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_7days.binSums_validsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );

      std::transform(   calendarDaySliceByCref.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        calendarDaySliceByCref.binSums_testsOnEachRule_barsLeftToRight.end(),
                        summedSlice_7days.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_7days.binSums_testsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );

      std::transform(   calendarDaySliceByCref.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        calendarDaySliceByCref.binSums_failsOnEachRule_barsLeftToRight.end(),
                        summedSlice_7days.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        summedSlice_7days.binSums_failsOnEachRule_barsLeftToRight.begin(),
                        std::plus<BinSum_t>()
      );
   }

   summedSlice_7days.timeOfFrontEdge = sliceLog_past7calendarDays.front().timeOfFrontEdge;

   return summedSlice_7days;
}


//======================================================================================================/

GuiPackHistogram_t CHistogramRuleKit::SayGuiPack( void ) {

   SHistoSliceRuleKit sliceShowing(   (spanActive == ETimeSpan::Histo_movingHour) ?
                                         realtimeSlice_movingHour :
                                         (spanActive == ETimeSpan::Histo_past24hrs) ?
                                            GenerateSliceSummedOnPast24hrs() :
                                            GenerateSliceSummedOnPast7days() // assumed as 3rd span
  );

   return SGuiPackHistogram(  EGuiType::Histogram_ruleKit,
                              ownGuiKey,
                              EGuiType::HistogramBars_rulesUai,
                              static_cast<GuiUin_t>(numRulesInKit),
                              GenerateCaptionText( sliceShowing.timeOfFrontEdge ),
                              GenerateBarHeightsFromSlice( sliceShowing ),
                              NaNFLOAT,
                              NaNFLOAT,
                              std::vector<std::string>(  barLabels_ruleIdentifiers.begin(),
                                                         barLabels_ruleIdentifiers.end()
                              ),
                              std::vector<std::string>(  modeLabels_ruleKit.begin(),
                                                         modeLabels_ruleKit.end()
                              ),
                              std::vector<std::string>(  spanLabels.begin(),
                                                         spanLabels.end()
                              ),
                              modeActive_index,
                              spanActive_index,
                              std::vector<NGuiKey>(   knobKeys_sourceRef.begin(),
                                                      knobKeys_sourceRef.end()
                              )
   );

}

//======================================================================================================/ 

void CHistogramRuleKit::Cycle(
   time_t newTimestampForFrontEdge,
   bool beginNewClockHour,
   bool beginNewCalendarDay,
   const std::unordered_map<Nzint_t,Bindex_t>& bindexEnteringFrontEdge_byRuleUai,
   const std::unordered_map<Nzint_t,Bindex_t>& bindexLeavingOneHourEdge_byRuleUai ) {

   if ( firstCycle ) {

      // load s/u timestamp into front edge of both slice logs
      sliceLog_past24clockHrs.front().timeOfFrontEdge = newTimestampForFrontEdge;
      sliceLog_past7calendarDays.front().timeOfFrontEdge = newTimestampForFrontEdge;
      firstCycle = false;
   }

   if ( beginNewClockHour ) {

      // most recent slice is at lowest index ("front"); oldest at highest index ("back")
      sliceLog_past24clockHrs.pop_back();
      sliceLog_past24clockHrs.push_front( realtimeSlice_movingHour );
   }
   if ( beginNewCalendarDay ) {

      sliceLog_past7calendarDays.pop_back();
      sliceLog_past7calendarDays.push_front( GenerateSliceSummedOnPast24hrs() );
   }           

   // initialize locals; believe using these is faster than accessing member fields on every Cycle()
   EGuiState stateEnteringBar = EGuiState::Undefined;
   EGuiState stateLeavingBar = EGuiState::Undefined;
   size_t iRuleBar = 0;

   for ( auto ruleUaiOfBar : ruleUais_barsLeftToRight ) {

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
// Increment binSums and non-const cycle counts based on bindex entering front edge of moving hour

      stateEnteringBar =
         GUISTATES_RULEBINS[ static_cast<size_t>(bindexEnteringFrontEdge_byRuleUai.at(ruleUaiOfBar)) ];

      switch ( stateEnteringBar ) {

         case ( EGuiState::Rule_autoMode_fail ) :

            ++realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_autoMode_pass) :

            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_autoMode_skip ) :

            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_invalid ) :

            break;

         case ( EGuiState::Rule_caseMode_fail ) :

            ++realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_caseMode_pass ) :

            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_caseMode_skip ) :

            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_fail ) :

            ++realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_pass ) :

            ++realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_skip ) :

            ++realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_unavailable ) :

            break;
      }

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::/
// Decrement binSums and non-const cycle counts based on bindex leaving back edge of moving hour

      stateLeavingBar =
         GUISTATES_RULEBINS[ static_cast<size_t>(bindexLeavingOneHourEdge_byRuleUai.at(ruleUaiOfBar)) ];

      switch ( stateLeavingBar ) {

         case ( EGuiState::Rule_autoMode_fail ) :

            --realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_autoMode_pass) :

            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_autoMode_skip ) :

            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_invalid ) :

            break;

         case ( EGuiState::Rule_caseMode_fail ) :

            --realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_caseMode_pass ) :

            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_caseMode_skip ) :

            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_fail ) :

            --realtimeSlice_movingHour.binSums_failsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_pass ) :

            --realtimeSlice_movingHour.binSums_testsOnEachRule_barsLeftToRight[iRuleBar];
            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_idleMode_skip ) :

            --realtimeSlice_movingHour.binSums_validsOnEachRule_barsLeftToRight[iRuleBar];
            break;

         case ( EGuiState::Rule_unavailable ) :

            break;
      }
      ++iRuleBar; // advance to next bar of Rule Kit histogram
   }  // close for-loop iterating ruleUaiOfBar through ruleUais_barsLeftToRight 

   realtimeSlice_movingHour.timeOfFrontEdge = newTimestampForFrontEdge;

   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
