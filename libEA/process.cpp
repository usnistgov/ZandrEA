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
   Implements CProcess class, which is EA interface to experimental algorithms in "experior" container
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#include "process.hpp"
#include "agentTask.hpp"      // register to sequence
#include "dataChannel.hpp"
#include "subject.hpp"        // call getters on subject
#include <algorithm>          // std::max_element() in CalcOwnTriggerGroup()

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Implementation for CFormula

// constructors

CProcess::CProcess(  CSequence& bArg0,
                     ASubject& bArg1,
                     Nzint_t,
                     std::vector<CPointAnalog*> arg1 )
                     :  ISeqElement(   bArg0,
                                       bArg1,
                                       EApiType::Process_expClient,
                                       EDataLabel::Process_expClient,
                                       EDataUnit::Binary_Boolean,
                                       EDataRange::Boolean,
                                       EDataSuffix::None,
                                       EPlotGroup::Undefined,
                                       1,        // could be min(tpc of inputs), but =1 is simpler
                                       BASETRIGGRP_PROCESS
                        ),
                        p_Operands (arg1),
                        s_rpcChannel (
                           grpc::CreateChannel( "experior:50051", grpc::InsecureChannelCredentials()) ),
                        u_rpcStub ( microservice::MakeComparisons::NewStub(s_rpcChannel) ),
                        firstCall (true),
                        resultNow (false),
                        resultLastValid (false) {

   CalcOwnTriggerGroup();                                              
   bArg0.Register( this );
   ConfigureCycling();

}
 
CProcess::~CProcess( void ) { }


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CProcess methods - private

void CProcess::CalcOwnTriggerGroup( void ) {

   std::vector<Nzint_t> inputTriggerGroups(0);
   for ( auto& ptr : p_Operands ) {
      inputTriggerGroups.push_back( ptr->SayOwnTriggerGroup() );
   }
   Nzint_t latestInputTriggerGroup =
      *std::max_element( inputTriggerGroups.begin(), inputTriggerGroups.end(), std::less<Nzint_t>() );
   
   if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
      ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
   }
   else { ownTriggerGroup = baseTriggerGroup; }
   return;
}


void CProcess::ConfigureCycling( void ) {

   std::vector<int> stackVec(0);

   for ( std::vector<CPointAnalog*>::const_iterator iter = p_Operands.begin();
         iter != p_Operands.end();
         ++iter ) {

            stackVec.push_back( (*iter)->SayTriggersPerCycle() );
   }
   
   // object to cycle at min triggers/cycle of all upstream objects sending input to it
   triggersPerCycle = *std::min_element( stackVec.begin(), stackVec.end() );
   if (triggersPerCycle == 1 ) { cyclingAtMaxRate = true; }
   triggersUntilCycle = triggersPerCycle;
   secsPerCycle = triggersPerCycle * SeqRef.SayTriggerPeriodSecs();

   return;
}

bool CProcess::PullValidity( void ) {

   bool reply = p_Operands.at(0)->IsValid();   // Get validity of 1st operand...

   for (size_t i=1; i<p_Operands.size(); ++i) {          // ...then enjoin any/all other operands
      reply = ( reply && (p_Operands.at(i)->IsValid() ) );
   }
   return reply;
}


void CProcess::Cycle( time_t timestampNow ) {

   validNow = PullValidity();    // reset own object validity up to point of calling calulation

   if ( validNow && !firstCall ) {

    // Create a request message
    microservice::MessageTypeA request;
    request.set_x(20.0);
    request.set_y(5.0);
    request.set_z(3.0);

    // Call the service
    microservice::MessageTypeB response;
    grpc::ClientContext context;

    grpc::Status status = u_rpcStub->Is_x_GT_y(&context, request, &response);
    resultNow = response.reply();
   }

   resultLastValid = ( validNow ? resultNow : resultLastValid );
   firstCall = false;
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CProcess methods - public

bool CProcess::SayBoolPosted( void ) const { return resultNow; }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ