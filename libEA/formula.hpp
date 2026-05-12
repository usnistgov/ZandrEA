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
   Declares CForumla class to create objects doing calculations using sampled or parametric values
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef FORMULA_HPP
#define FORMULA_HPP

#include "seqElement.hpp"      // Inheiritance requires type completion, also brings "customTypes.hpp"
#include "guiShadow.hpp"
#include <functional>


// Forward declares (to avoid unnecessary #includes)
class CPointAnalog;
class CRainAnalog;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CFormula concrete class template declaration.  Class is for objects evaluating a formulation of
   analog (i.e., floating point) values of acquired data (i.e., from rainfall(s) ), with the formulated
   value run and returned into "result" by Cycle(). (i.e., "Formulate" is not an equation by itself.)
*/

class CFormula  : public ISeqElement { 

   public:

   // Handle
      const std::unique_ptr<CRainAnalog>              u_Rain;

   // Methods
      CFormula(   CSequence&,
                  ASubject&,
                  EDataLabel,
                  EDataUnit,
                  EDataRange,
                  EDataSuffix,
                  EPlotGroup, 
                  std::vector<CPointAnalog*>,
                  std::function<float(void)> );
  
      ~CFormula( void );

      virtual void      LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;
      virtual void      LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ) const override;

   private:

      const std::unique_ptr<CTraceRealtime>     u_RealtimeTrace;
      const std::vector<CPointAnalog*>          p_Operands;
 
   // Lambda of a formula, wrapped in std::function, expressing RHS only of an eqn assigning "result"  
      const std::function<float(void)>       Calculate; 
                       
   // Fields
      float                                  resultNow;
      float                                  resultLastValid;

   // Methods
      bool                                   PullValidity( void );
      void                                   ConfigureCycling( void );

      virtual void                           CalcOwnTriggerGroup( void ) override;
      virtual void                           Cycle( time_t ) override;

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Typedef pointers to getter methods of specific classes declared above

typedef bool ( CFormula::*PtrFormGetr_t )( void ) const;

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
