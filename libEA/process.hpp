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
   Declares AProcess abstract class and concrete subclass CProcess
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "seqElement.hpp"    // Inheiritance requires type completion, also brings "customTypes.hpp"
#include "guiShadow.hpp"
#include "/ea/grpc/include/grpcpp/grpcpp.h"

// Prior to EA build, compile .proto files using protoc with the --cpp_out option to generate these:
#include "microservice.pb.h"
#include "microservice.grpc.pb.h"

// Forward declares (to avoid unnecessary #includes)
class CPointAnalog;

//------------------------------------------------------------------------------------------------------/

class CProcess  : public ISeqElement { 

   public:

   // Methods
      CProcess(   CSequence&,
                  ASubject&,
                  Nzint_t,       // TPC for querying remote procedure
                  std::vector<CPointAnalog*>
      );
  
      ~CProcess( void );

      bool SayBoolPosted( void ) const;

   private:

   // Handles
      const std::vector<CPointAnalog*>                         p_Operands;
      std::shared_ptr<grpc::Channel>                           s_rpcChannel;
      std::unique_ptr<microservice::MakeComparisons::Stub>     u_rpcStub;

   // Fields
      bool                                   firstCall;
      bool                                   resultNow;
      bool                                   resultLastValid;

   // Methods
      bool                                   PullValidity( void );
      void                                   ConfigureCycling( void );
      virtual void                           CalcOwnTriggerGroup( void ) override;
      virtual void                           Cycle( time_t ) override;
};

#endif