// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements classes for the Hypothesis and Evidence Item parts of a knowledge base (KB) that will
   reside in memory of the running program (i.e., not the part of KB implemented on disk via HDF5).
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "knowParts.hpp"
#include "knowBase.hpp"

#include <sstream>
#include <iomanip>
#include <exception>
#include <algorithm>
// uncomment to disable assert()
// #define NDEBUG
//#include <cassert>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Free method

std::string ConvertIdToNameInKbase( char axis, Nzint_t id ) {

   std::string replyString("");
   std::ostringstream replyStream;

   if (((axis == 'R') || (axis == 'H') || (axis == 'E')) && ((id > 0) && (id < 1000))) {

      replyStream << axis << std::setw(3) << std::setfill('0') << id;
   }

   // Inverse function (i.e., string -> id) would be something like:  std::stoul( nameStr.substr(1,3) );

   return replyStream.str();
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// STATIC fields and methods from header

IdVec_t CEvid::evidIdsInUse(0);
IdVec_t CHypo::hypoIdsInUse(0);

void CEvid::CheckIdFreeThenKeep( Nzint_t arg ) {

   if ( std::find(evidIdsInUse.begin(), evidIdsInUse.end(), arg) == evidIdsInUse.end() ) {

      evidIdsInUse.push_back( arg );
   }
   else { throw std::logic_error("Evid given duplicate UAI"); }
   return;
}


void CHypo::CheckIdFreeThenKeep( Nzint_t arg ) {

   if ( std::find(hypoIdsInUse.begin(), hypoIdsInUse.end(), arg) == hypoIdsInUse.end() ) {

      hypoIdsInUse.push_back( arg );
   }
   else { throw std::logic_error("Hypo given duplicate UAI"); }
   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CEvid

CEvid::CEvid(  Nzint_t arg0,
               std::string arg1 )
               : name ( ConvertIdToNameInKbase('E',arg0) ),
                 query(arg1),            // interrogative statement sent to GUI
                 evidId (arg0),
                 numValues (2),          // TBD for evid values other than binary 'F' or 'T'
                 userProvides (true) {   // TBD for automated evidence (i.e., "false")

   //try { CheckIdFreeThenKeep( arg0 ); }
   //catch (std::logic_error) { throw; }
}


CEvid::~CEvid( void ) {

   // Empty d-tor
}


std::string CEvid::SayName( void ) const { return name; }


std::string CEvid::SayQuery( void ) const { return query; }


Nzint_t CEvid::SayId( void ) const { return evidId; }


Nzint_t CEvid::SayNumValues( void ) const { return numValues; }


bool CEvid::DoesUserProvideEvid( void ) const { return userProvides; }


void CEvid::BuildRuleHypoAndEvidIntoKbase(   Nzint_t ruleId,
                                             Nzint_t hypoId,
                                             const CHypo* const p_Hypo,
                                             bool senseDirect,
                                             bool logicInvertible,
                                             const std::vector<Nzint_t>& idAssocHyposRef,
                                             CKnowBaseH5& KbaseRef ) {

   // once have all three elements of a R-H-E triple, call Kbase to init a node there
   KbaseRef.InitializeNodeAt( ruleId,
                              hypoId,
                              p_Hypo,
                              evidId,
                              this,
                              senseDirect,
                              logicInvertible,
                              idAssocHyposRef);

   return;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementations for CHypo

CHypo::CHypo(  Nzint_t arg0,
               std::string arg1 )
               :  evidAssocData(),
                  name (ConvertIdToNameInKbase('H', arg0)),
                  hypothesis (arg1),        // Declarative statement expressing the hypothesis
                  hypoId (arg0) {

   //try { CheckIdFreeThenKeep( arg0 ); }
   //catch (std::logic_error) { throw; }
}


CHypo::~CHypo( void ) {

   // Empty d-tor
}


std::string CHypo::SayHypothesis( void ) const { return hypothesis; }


std::string CHypo::SayName( void ) const { return name; }


Nzint_t CHypo::SayId( void ) const { return hypoId; }


void CHypo::AssociateEvid( CEvid* const p_Evid, bool initSenseDirect, bool invertible) {

/* Called on a particular CHypo object by CreateTool() in createTool_***.cpp file.
   Initial "sense" between Evid and Hypo is entered as "positive" (i.e., initSensePosi = true) if
   human expert writing CreateTool() is presuming that Evid = true implies Hypo = true.

   But, initSensePosi is used only once, to initialize the respective node of kBase to the presumed
   sense. 'Actual' sense is that expressed by relative occurence counts in that node of kBase.  Actual
   sense can later change from the presumed sense as that kBase node "learns" future case solutions.
*/
   evidAssocData.push_back( std::make_tuple(p_Evid, initSenseDirect, invertible) );

   return;
}


void CHypo::BuildRuleAndHypoIntoKbase( Nzint_t ruleId,
                                       const std::vector<Nzint_t>& idAssocHyposRef,
                                       CKnowBaseH5& KbaseRef ) {

   for ( auto& iter : evidAssocData ) {

      std::get<EVIDPTR>(iter)->BuildRuleHypoAndEvidIntoKbase(  ruleId,
                                                               hypoId,
                                                               this,
                                                               std::get<SENSEDIRECT>(iter),
                                                               std::get<INVERTIBLE>(iter),
                                                               idAssocHyposRef,
                                                               KbaseRef );
   }
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZ
