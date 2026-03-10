// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares AProcess abstract class and concrete subclass CProcessExp
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

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