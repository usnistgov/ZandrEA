// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares CForumla class to create objects doing calculations using sampled or parametric values
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

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
