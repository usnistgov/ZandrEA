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
   Implements the concrete class CController for an EA Application's  MVC Controller object. 
   A CController object ultimately implements the "Controller" methods of the EA runtime API.
   Since the API are all pure virtual methods, the implementation first goes through a CPortOmni object.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#include "mvc_view.hpp"
#include "case.hpp"
#include "dataChannel.hpp"
#include "controlParts.hpp"
#include "taskClock.hpp"
#include "subject.hpp"
#include "mvc_ctrlr.hpp"
#include "HDF5Parts.hpp"


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Controller implementations

CController::CController(  CClockPerPort& arg0 )
                           :  ClockRef (arg0),
                              p_Knobs_byKey(),
                              pointNamesZeroToN_bySubjKey(),
                              pointObjectsZeroToN_bySubjKey() {

}


CController::~CController( void ) {

   // empty interface d-tor
}


//======================================================================================================/
// Private Methods



//======================================================================================================/
// Public Methods

EGuiReply CController::PrepareApplicationForShutdown( void ) {

   herr_t reply = H5close();
   return ( (reply < 0) ? EGuiReply::FAIL_any_calledFunctionNotYetImplemented : EGuiReply::OKAY_allDone );
}


EGuiReply CController::SetTimeStampInDomain( std::tm tmStruct ) {

   return ClockRef.SetTimeFromPort( tmStruct );
}


std::vector<EPointName>
CController::SayInputPointNameOrderExpectedBySubject( NGuiKey subjectKey ) const {

   return pointNamesZeroToN_bySubjKey.at( subjectKey );
}


EGuiReply CController::ReadInDataForSubject( const std::vector<GuiFpn_t>& newSamplesRef,
                                             NGuiKey subjectKey  ) {

   if ( newSamplesRef.size() == pointObjectsZeroToN_bySubjKey.at(subjectKey).size() ) {

      size_t inputChannelOnSubject = 0;
      for ( auto p_PointObj : pointObjectsZeroToN_bySubjKey.at(subjectKey) ) {

         p_PointObj->ReadFromPortAsNextValue( newSamplesRef[inputChannelOnSubject] );

         ++inputChannelOnSubject;
      }
      return EGuiReply::OKAY_allDone;
   }
   return EGuiReply::FAIL_set_givenContainerWrongSizeForKeyGiven;
}


EGuiReply CController::SingleStepModelOnTimeAndInputs( void ) {

   return ( (ClockRef.RingTaskBell() == EGuiReply::OKAY_allDone) ?
               p_View->Update() :
               EGuiReply::WARN_ranSeqToExitWithObjectsYetToCycle_fixApi );
}


GuiPackKnob_t  CController::GetGuiPackFromKnob( NGuiKey knobKey ) const {

   return ( p_Knobs_byKey.count( knobKey ) == 0 ?
      SGuiPackKnob( EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled ) :
      p_Knobs_byKey.at( knobKey )->GetGuiPack()  // must use at() since method is const, [] is non-const
   ); 
}


EGuiReply CController::SetKnobToValue( NGuiKey knobKey, GuiFpn_t valueGiven ) {

   return ( p_Knobs_byKey.count( knobKey ) == 0 ?
               EGuiReply::FAIL_any_givenKeyNotValidForFunctionCalled :
               p_Knobs_byKey.at( knobKey )->SetValueTo( valueGiven )
   );
}


std::string  CController::SayTextIdentifyingKnob( NGuiKey knobKey ) const {

   return p_Knobs_byKey.at( knobKey )->SayIdentifyingText(); 
}


//vvvvvvv1vvvvvvvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

void CController::Register( CView* arg ) { p_View = arg;  return; }


void CController::Register( AKnob* const knobPtr ) {

   // map insert() nops upon finding key already in container.  Emplace() not advantageous enough here.

   p_Knobs_byKey.insert( std::pair<NGuiKey, AKnob* const>( knobPtr->SayGuiKey(), knobPtr ) );
   return;
}


void CController::RegisterBasPointToSubjectKey( ADataChannel* ptr, NGuiKey subjectKey ) {

   // map insert() nops upon finding key already in unordered map.  Emplace() not advantageous here.

   pointObjectsZeroToN_bySubjKey.insert( std::pair< NGuiKey,
                                                   std::vector<ADataChannel*> >(
                                                      subjectKey,
                                                      std::vector<ADataChannel*>(0) )
   );

   pointObjectsZeroToN_bySubjKey.at( subjectKey ).push_back( ptr );

   pointNamesZeroToN_bySubjKey.insert( std::pair<   NGuiKey,
                                                   std::vector<EPointName> >(
                                                      subjectKey,
                                                      std::vector<EPointName>(0) )
   );

   pointNamesZeroToN_bySubjKey.at( subjectKey ).push_back( ptr->SayPointName() );
   return;
}


void CController::DeregisterKnob( const NGuiKey knobKey ) {

   p_Knobs_byKey.erase( knobKey );     // erase on maps (vs. vectors) can take key (vs. iterator)
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
