// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implement abstract base and concrete classes for objects receiving sampled data (e.g., "points")
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "dataChannel.hpp"
#include "agentTask.hpp"      // register to sequence
#include "state.hpp"          // for d-tor to destroy Fact object by smart pointer in CPointBinary
#include "rainfall.hpp"       // call methods on rainfall
#include "viewParts.hpp"      // get NGuiKey of traces, call d-tor of trace u-ptr
#include "subject.hpp"        // call getters on subject
#include "mvc_ctrlr.hpp"      // register point to ctrlr (the only way sampled data enters app)


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implement concrete subclass for points handling analog data

ADataChannel::ADataChannel( CSequence&  bArg0,
                        ASubject& bArg1,
                        EApiType bArg2,
                        EDataLabel bArg3,
                        EDataUnit bArg4,
                        EDataRange bArg5,
                        EDataSuffix bArg6,
                        EPlotGroup bArg7,
                        int bArg8,
                        EPointName arg1 )
                        :  ISeqElement(   bArg0,
                                          bArg1,
                                          bArg2,
                                          bArg3,
                                          bArg4,
                                          bArg5,
                                          bArg6,
                                          bArg7,
                                          bArg8,
                                          BASETRIGGRP_POINT
                           ),
                           xGivenDbl (NaNDBL),
                           xPrevDbl (NaNDBL),
                           pointName (arg1) {

}

ADataChannel::~ADataChannel( void ) { /* empty d-tor */ };

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Private methods


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Public methods

EPointName ADataChannel::SayPointName( void ) const { return pointName; }


void ADataChannel::ReadFromPortAsNextValue( GuiFpn_t arg ) {

   xPrevDbl = xGivenDbl;
   xGivenDbl = arg;
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implement concrete subclass for points handling analog data


CPointAnalog::CPointAnalog(   CSequence&  bArg0,
                              ASubject& bArg1,
                              EDataLabel bArg2,
                              EDataUnit bArg3,
                              EDataRange bArg4,
                              EPlotGroup bArg5,
                              EPointName bArg6,
                              CController& arg0 )
                              :  ADataChannel( bArg0,
                                             bArg1,
                                             EApiType::Point_analogReadAsAnalog,
                                             bArg2,
                                             bArg3,
                                             bArg4,
                                             EDataSuffix::None,   // "suffix" is for CFormula objects
                                             bArg5,   // group data is to plot with others in same Pane
                                             1,       // All input points work at tpc=1
                                             bArg6
                                 ),
                                 u_Rain ( std::make_unique<CRainAnalog>(   *this,
                                                                           knobKeys_ownedAndAntecedent,
                                                                           bArg1,
                                                                           bArg4 )
                                 ),
                                 u_RealtimeTrace ( std::make_unique<CTraceRealtime>(
                                                      bArg1.SayViewRef(),
                                                      *u_Rain,
                                                      knobKeys_ownedAndAntecedent  // must be PBR
                                                   )
                                 ),
                                 xPosted (NaNFLOAT),
                                 xValidMin (NaNFLOAT),
                                 xValidMax (NaNFLOAT),
                                 xLastValid (NaNFLOAT),
                                 sameDblAsPrev (false) {

   CalcOwnTriggerGroup();
   bArg0.Register( this );
   arg0.RegisterBasPointToSubjectKey( this, bArg1.SayGuiKey() );
}

CPointAnalog::~CPointAnalog( void ) { /* empty d-tor */ }; // no ptr clean-up as this class is immortal

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Private methods

void CPointAnalog::Cycle( time_t timestampNow ) {

   //sameDblAsPrev = ( xGivenDbl == xPrevDbl );      // $$$ TBD whether to do anything with this $$$

   u_Rain->Cycle( timestampNow,
                  cycleBeginsNewClockHour,
                  cycleBeginsNewCalendarDay,
                  static_cast<float>( xGivenDbl ),
                  validNow
   );
   return;
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Public methods

void CPointAnalog::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

/* Note what method must do here:  e.g., a chart has no own rain or own histogram, but a chart's
   (analog) input object has both, and the Fact(s) a chart feeds into has both.  Lending keys out so
   to be delivered to GUI in a GuiPack must accommodate that.
   not workable to have u_Rain in ARainfall
*/
   NGuiKey ownHistogramKey = u_Rain->SayHistogramKey();

   bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                ownHistogramKey
                                             ) == borrowerRef.end() ?
                                                true :
                                                false
   ); 
   if ( borrowerNotAlreadyHoldingSameKey ) {

      borrowerRef.push_back( ownHistogramKey );
   }
   return;
}

void CPointAnalog::LendRealtimeAnalogAccessTo(  RtTraceAccessTable_t& ptrTableRef ) const {

   // C++11 allows for direct c-tor calls on std::pair<>() vs. calling std::make_pair

   std::pair<RtTraceAccessTable_t::iterator, bool> tableVerbReply =
      ptrTableRef.emplace(
         std::pair< NGuiKey, CTraceRealtime*>(
            u_RealtimeTrace->SayGuiKey(),
            u_RealtimeTrace.get()
         )
      );
   return;
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implement concrete subclass for points handling binary data

CPointBinary::CPointBinary(   CSequence&  bArg0,
                              ASubject& bArg1,
                              EDataLabel bArg2,
                              EDataLabel bArg3,
                              EPointName bArg4,
                              CController& arg0 )
                              :  ADataChannel( bArg0,
                                             bArg1,
                                             EApiType::Point_analogReadAsBinary,
                                             bArg2,
                                             EDataUnit::Binary_Boolean,
                                             EDataRange::Boolean,
                                             EDataSuffix::None,
                                             EPlotGroup::Free, // See Class Note [1]
                                             1,                // All input points work at tpc = 1
                                             bArg4
                                 ),
                                 u_Fact (std::make_unique<CFactFromPoint>( bArg0,
                                                                           bArg1,
                                                                           bArg3,
                                                                           *this )
                                 ),
                                 binaryPosted (NaNBOOL) {

   CalcOwnTriggerGroup();
   bArg0.Register( this );
   arg0.RegisterBasPointToSubjectKey( this, bArg1.SayGuiKey() );

/* Start Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Binary data need no distinctive "plot group" (i.e., EPlotGroup = "free")

'' End Class Notes '''' */ 

}


CPointBinary::~CPointBinary( void ) { /* empty d-tor */ }; // no ptr clean-up as this class is immortal


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Private methods

void CPointBinary::CalcOwnTriggerGroup( void ) { ownTriggerGroup = baseTriggerGroup; return; }


void CPointBinary::Cycle( time_t timestampNow ) {

// $$$ TBD for an actual way to set validNow per state of the information source $$$

   validNow = ( std::floor( xGivenDbl) < 1.10 );

   // Unlike CPointAnalog, here no push to a rainfall. Owned CFactFromPoint obj runs getter and rainfall
   binaryPosted = ( (xGivenDbl > 0.5) ? true : false );
   return;
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/
// Public methods


bool CPointBinary::SayBinaryPosted( void ) const { return binaryPosted; }


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   Calling a PVF of the ISequenceObsvr superclass is okay in CPointAnalog class ctor because the PVF
      has an implementation defined in the CPointAnalog class. 

--------------------------------------------------------------------------------
XXX END FILE NOTES */

  
//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
