// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements CForumla class to create objects doing calculations using sampled or parametric values
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

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
                        resultNow (NaNBOOL),
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

   }

   resultLastValid = ( validNow ? resultNow : resultLastValid );
   firstCall = false;
   return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CProcess methods - public

bool CProcess::SayBoolPosted( void ) const { return resultNow; }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ