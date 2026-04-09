// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implementation of classes for Subject objects and for Domain object (one Domain per Application)
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "subject.hpp"
#include "mvc_view.hpp"    // self-register to View
#include "case.hpp"        // hits on CCaseKit methods

#include <utility>         // needed for swap
#include <sstream>
#include <iomanip>
#include <iterator>
#include <algorithm>       // find
#include <numeric>         // accumulate
#include <functional>      // plus

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation of abstract base class for all Subject objects

ASubject::ASubject(  CDomain& arg0,
                     EDataLabel arg1,
                     ERealName arg2 )
                     :  IGuiShadow( EApiType::Subject ),
                        pinnedRuleFailHistories_byRuleKit(),
                        infoText(0),
                        featureKeys(0),
                        knobKeys(0),
                        ruleKitDisplayKeys(0),
                        uaiInUse_features(0),
                        sgiInUse_ruleKits(0),
                        ownLabel (arg1),
                        ownName(arg2),
                        nextSgiForCases (1u),
                        nextSgiForRuleKits (1u),
                        unitOutputOkay (true),
                        DomainRef (arg0),
                        u_CaseKit ( std::make_unique<CCaseKit>(
                                       *this,
                                       arg0.SayEnergyPricesRef(),
                                       arg0.SayRootTextForDiskFilenames() + LookUpDiskFile( arg2 ) )
                        ) {
// empty ABC c-tor
}


ASubject::~ASubject( void ) { }


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Public methods of ASubject interface

GuiPackSubjectBasic_t ASubject::SayBasicGuiPack( void ) const {

   // *** TBD to include Subject's const params as additional 'infoLines'

   return SGuiPackSubjectBasic(  LookUpGuiType( ownApiType ),
                                 ownGuiKey,
                                 DomainRef.SayGuiKey(),
                                 LookUpText(ownName),
                                 std::vector<std::string>( 1, LookUpTag( ownLabel ) ),
                                 featureKeys,
                                 knobKeys,
                                 ruleKitDisplayKeys
   );
}


GuiPackSubjectCases_t ASubject::SayCurrentCases( void ) const {

   return ( SGuiPackSubjectCases(   u_CaseKit->SayCaseKeysByDecrRank(),
                                    u_CaseKit->SayCaseNamesByDecrRank() )
   );
}


const SEnergyPrices& ASubject::SayEnergyPricesRef( void ) const { return DomainRef.SayEnergyPricesRef(); }


Nzint_t ASubject::GenerateAndSaySgiForNewCase( void ) {

   // Cannot be class-static; need distinct S/N progression for each object of an ISubject subclass
   return ( ( nextSgiForCases < 255u ) ? nextSgiForCases++ : 1u );
}


std::string ASubject::SayRootTextForDiskFilenames( void ) const {

   return ( DomainRef.SayRootTextForDiskFilenames() + LookUpDiskFile( ownName ) );
}


std::string ASubject::SayNumRuleKitsAsText( void ) const {

   return std::to_string( ruleKitDisplayKeys.size() );
}


std::string ASubject::SayDomainAndOwnNameAsText( void ) const {

   return ( LookUpText( DomainRef.SayName() ) + ": " + LookUpText( ownName ) ) ;
}


std::string ASubject::SayNameAsText( void ) const {

   return LookUpText( ownName );
}


EDataLabel ASubject::SayLabel( void ) const { return ownLabel; }


ERealName ASubject::SayName( void ) const { return ownName; }


CCaseKit& ASubject::SayCaseKitRef( void ) const { return *u_CaseKit; }


CView& ASubject::SayViewRef( void ) const { return *(DomainRef.SayViewPtr()); }


Nzint_t ASubject::GenerateAndSaySgiForNewRuleKit( void ) {

   Nzint_t newRuleKitSgi = nextSgiForRuleKits++;  // post-increment for next use

   sgiInUse_ruleKits.push_back( newRuleKitSgi );

   std::unique_ptr<std::deque<int> > u_historyBuffer_locallyScopedToHeap =
      std::make_unique< std::deque<int> >( FIXED_SUBJECT_CYCLESPINNEDFAILHISTORY, 0 ); 

   std::pair<IndexedFifoBuffers_t::iterator, bool> bufferVerbReply =
      pinnedRuleFailHistories_byRuleKit.emplace(
         std::pair<Nzint_t, std::unique_ptr<std::deque<int>> >(
            newRuleKitSgi,
            u_historyBuffer_locallyScopedToHeap.release() )  // xfers ownership from and nulls local ptr
   );
   return newRuleKitSgi;
}   // destroys nulled local unique ptr


bool ASubject::IsUnitOutputOkay( void ) const { return unitOutputOkay; }


void ASubject::SubmitTrueIfGotFailOnPinnedRule( Nzint_t sgiOfKitReporting, bool gotFailOnPinnedRule ) {

   pinnedRuleFailHistories_byRuleKit[sgiOfKitReporting]->push_back( (gotFailOnPinnedRule ? 1 : 0) );
   pinnedRuleFailHistories_byRuleKit[sgiOfKitReporting]->pop_front();

   // "okay" means no fail from any Rule pinned to unit output in any of the Subject's rule kits
   // being marked "pinned" (or not) to unit ouput is a part of each Rule object's definition

   int countPinnedRuleFailOccurencesInHistory_allKits = 0;

   for (auto sgi : sgiInUse_ruleKits ) {

      countPinnedRuleFailOccurencesInHistory_allKits +=
         std::accumulate(  pinnedRuleFailHistories_byRuleKit[sgi]->begin(),
                           pinnedRuleFailHistories_byRuleKit[sgi]->end(),
                           0,
                           std::plus<int>()
         );
   }
   unitOutputOkay = ( countPinnedRuleFailOccurencesInHistory_allKits == 0 );
   return;
}


void ASubject::CheckFeatureUaiFreeThenKeep( Nzint_t proposedUserAssignedIdentifier ) {

   // currently reqd only for Feature, but later on could include histograms, etc.

   if (  std::find(  uaiInUse_features.begin(),
                     uaiInUse_features.end(),
                     proposedUserAssignedIdentifier )
          == uaiInUse_features.end() ) {

      uaiInUse_features.push_back( proposedUserAssignedIdentifier );
   }
   else { throw std::logic_error( "GUI Feature given UAI already in use" ); }
   return;
}


void ASubject::AddRuleKitDisplay( NGuiKey kitDisplayKey ) {

   ruleKitDisplayKeys.push_back( kitDisplayKey);
   return;
}



void ASubject::AddFeatureKey( NGuiKey keyGiven ) { featureKeys.push_back( keyGiven ); return; }


void ASubject::TagAndPostAlertToDomain(   time_t timestamp,
                                          EDataLabel sourceLabel,
                                          EAlertMsg alertFromSource ) {

   // end-to-end P.B.V. as presuming chain of copy elision by compiler whenever optimal
   DomainRef.PostAsNewAlert( timestamp, ownName, sourceLabel, alertFromSource );
   return;
 }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////

CSubj_vav_pihr::CSubj_vav_pihr(  CDomain& bArg0,
                                 EDataLabel bArg1,
                                 ERealName bArg2,
                                 ERealName arg0,
                                 ERealName arg1,
                                 float arg2,              // diaAirInch
                                 float arg3,              // cfmAirRated
                                 float arg4,              // gpmChwRated
                                 float arg5 )             // btuReheatRated
                                 :  ASubject(bArg0,
                                             bArg1,
                                             bArg2
                                    ),
                                    nameAntecedentAhu (arg0),
                                    nameAntecedentHwPlant (arg1),
                                    inchDiaDuct (arg2),
                                    sqFtAirflow ( PI_F * ( (arg2 * arg2)/144.0f) ),
                                    cfmAirRated (arg3),
                                    gpmChwRated (arg4),
                                    btuReheatRated (arg5) {

   bArg0.Register( this, ownName );
}


CSubj_vav_pihr::~CSubj_vav_pihr( void ) {

   // empty
}

ERealName CSubj_vav_pihr::SayNameOfAntecedentAhu( void ) const { return nameAntecedentAhu; }

ERealName CSubj_vav_pihr::SayNameOfAntecedentHwPlant( void ) const { return nameAntecedentHwPlant; }

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////

CSubj_ahu_sdvr::CSubj_ahu_sdvr(  CDomain& bArg0,
                                 EDataLabel bArg1,
                                 ERealName bArg2,
                                 ERealName arg0,
                                 ERealName arg1,
                                 float arg2,             // cfmAirRated
                                 float arg3,             // gpmChwRated
                                 float arg4,             // kwReheatRated
                                 float arg5 )            // minimum OA fraction
                                 :  ASubject(bArg0,
                                             bArg1,
                                             bArg2
                                    ),
                                    nameAntecedentChwPlant (arg0),
                                    nameAntecedentHwPlant (arg1),
                                    cfmAirRated (arg2),
                                    gpmChwRated (arg3),
                                    kwPreheatRated (arg4),
                                    minFracOA (arg5) {

   bArg0.Register( this, ownName );
}


CSubj_ahu_sdvr::~CSubj_ahu_sdvr( void ) {

   // empty
}

ERealName CSubj_ahu_sdvr::SayNameOfAntecedentChwPlant( void ) const { return nameAntecedentChwPlant; }

ERealName CSubj_ahu_sdvr::SayNameOfAntecedentHwPlant( void ) const { return nameAntecedentHwPlant; }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
//CDomain implementation

CDomain::CDomain( ERealName arg )
                  :  IGuiShadow( EApiType::Domain ),
                     p_SubjOutputs_byName_byLabel(),
                     p_Subjects_byName(),
                     p_View (nullptr),
                     unsaidAlertsFifo(),
                     energyPrices( SEnergyPrices(0, 0, 0, 0, 0) ),
                     domainName (arg) {
}

CDomain::~CDomain( void ) {

}


std::queue<std::string> CDomain::SayNewAlertsFifoThenClear( void ) {

   std::queue<std::string> reply;         // constructs empty

   std::swap( unsaidAlertsFifo, reply );  // exchanges values between left and right arguments

   return reply;
}


std::vector<NGuiKey>  CDomain::SaySubjectKeys( void ) const {

   std::vector<NGuiKey> reply(0);

   for ( const auto& cr_pair : p_Subjects_byName ) {

      reply.push_back( cr_pair.second->SayGuiKey() );
   }
   return reply;
}


GuiPackDomain_t CDomain::SayGuiPack( void ) const {

   return ( SGuiPackDomain(   ownGuiKey,
                              LookUpGuiType(ownApiType),
                              LookUpText(domainName),
                              SaySubjectKeys()
            )
   );
}


const SEnergyPrices& CDomain::SayEnergyPricesRef( void ) const { return energyPrices; }


std::string CDomain::SayRootTextForDiskFilenames( void ) const {

   return LookUpDiskFile( domainName );
}


ERealName CDomain::SayName( void ) const { return domainName; }


const ASubject* const CDomain::SayPtrToSubjectNamed( ERealName nameGiven ) const {

   auto iter = p_Subjects_byName.find( nameGiven );
   ASubject* reply = (  iter == p_Subjects_byName.end() ?
                        nullptr :
                        iter->second
   );
   return reply;
}                     
 

CView* const CDomain::SayViewPtr( void ) const {

   if ( p_View == nullptr ) throw std::logic_error( "Asked for Domain View while null" );
   return p_View; }


void CDomain::Register( ASubject* const p_subj, ERealName subjName ) {

   if ( p_View == nullptr ) throw std::logic_error( "Attempted adding Subject to Domain prior to View" );

   p_Subjects_byName.insert( std::make_pair(subjName, p_subj ) );

   p_View->AddSubject( *p_subj );

   return;
}


void CDomain::Register( CView* const arg ) { p_View = arg; return; }


void CDomain::PostAsNewAlert( time_t timestamp,
                              ERealName forwardingSubjectsName,
                              EDataLabel sourceLabel,
                              EAlertMsg alertFromSource ) {

   std::string timeAsText("");
   WriteTimestampAsTextTo( timestamp, timeAsText );

   unsaidAlertsFifo.push(  timeAsText + " " +
                           LookUpText( domainName ) + " " +
                           LookUpText( forwardingSubjectsName ) + " " +
                           LookUpTag( sourceLabel ) + " " +
                           LookUpText( alertFromSource )
   );
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
