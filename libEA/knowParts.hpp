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
   Declares classes for the Hypothesis and Evidence Item parts of a knowledge base (KB) that will reside
   in memory of the running program (i.e., not the part of KB implemented on disk via HDF5).
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef KNOWPARTS_HPP
#define KNOWPARTS_HPP

#include "diagnosticTypes.hpp"
#include <tuple>

// Forward declares (to avoid unnecessary #includes)
class CHypo;              // Still need fwd-declare even where class declared further down same T.U.
class CKnowBaseH5;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// File-specific definitions

// For indexing into EvidTuple_t via std::get<> calls

const size_t EVIDPTR = 0;
const size_t SENSEDIRECT = 1;
const size_t INVERTIBLE = 2;

std::string ConvertIdToNameInKbase( char, Nzint_t );

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Declare CEvid class for all objects of "evidence"
   Each object of evidence either supports or refutes the truth of any hypothesis it addresses.
   One object of evidence may address multiple hypothesis, supporting or refuting each one variously.
*/

class CEvid { 

   public:

      typedef std::tuple<CEvid*, bool, bool> EvidTuple_t;
      typedef std::vector<EvidTuple_t> EvidAssoc_t;

      CEvid( Nzint_t, std::string );
      // CEvid( std::string, std::string, std:function< TBD > );

      ~CEvid( void );

      std::string       SayName( void) const;
      std::string       SayQuery( void ) const;
      Nzint_t           SayId( void ) const;
      Nzint_t           SayNumValues( void ) const;
      bool              DoesUserProvideEvid( void ) const;
      void              BuildRuleHypoAndEvidIntoKbase(   Nzint_t,
                                                         Nzint_t,
                                                         const CHypo* const,
                                                         bool,
                                                         bool,
                                                         const std::vector<Nzint_t>&,
                                                         CKnowBaseH5& );

   private:

      // std::function<TBD>         queryLambda;
      const std::string             name;
      const std::string             query;
      static IdVec_t                evidIdsInUse;
      const Nzint_t                 evidId;
      const Nzint_t                 numValues;
      const bool                    userProvides;

      static void CheckIdFreeThenKeep( Nzint_t );

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   An evid is the innermost element in the rule-hypo-evid triple defining a specific node in a kBase.
      So, method to build such nodes resides in the CEvid class. 

^^^^ END CLASS NOTES */
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Declare CHypo class for objects of "hypothesis" used in diagnostics.
   When data acquired by the surveillance part of EA "fails" a surveillance rule sufficently to warrant
   making a "case" of it, the diagnostic part of EA uses its knowledge base to provide the user with all
   hypotheses (explanations, a.k.a. system "faults") relevant to failure of that particular rule. 
*/

class CHypo { 

   public:

      CHypo( Nzint_t, std::string );
      ~CHypo( void );

      void AssociateEvid(  CEvid* const,
                           bool,             // relation is direct (initially)?
                           bool );           // relation is invertible (initially)? 
      
      void BuildRuleAndHypoIntoKbase(  Nzint_t,
                                       const std::vector<Nzint_t>&,
                                       CKnowBaseH5& );

      std::string    SayHypothesis( void ) const;
      std::string    SayName( void ) const;
      Nzint_t        SayId( void ) const;

   private:

      CEvid::EvidAssoc_t   evidAssocData; // A "tuple" = [p_Evid, direct? (T or F), invertible? (T or F)]
      static IdVec_t       hypoIdsInUse;
      const std::string    name;
      const std::string    hypothesis;
      const Nzint_t        hypoId;

      static void CheckIdFreeThenKeep( Nzint_t );
};


#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
