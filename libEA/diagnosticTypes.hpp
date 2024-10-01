// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
 Declares Includes and customized Types used across class headers of the FDD tool diagnostic parts
   (i.e., the "expert system" components of EA code)

 This header is to the "diagnostic" part of the code what customTypes.hpp is to the "surveillance" part
      Expert system = knowledge base + inference engine
      R-H-E = Rule - Hypothesis - Evidence Item (the three axis names of a knowledge base (KB))
 */
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef DIAGNOSTICTYPES_HPP
#define DIAGNOSTICTYPES_HPP

#include <array>
#include <vector>
#include <unordered_map>

#include <string>       // ***** include only for ECHO TESTING during dev *****
#include <iostream>     // ***** include only for ECHO TESTING during dev *****


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Numerical types used throughout diagnostics

typedef unsigned int Nzint_t;    // See File Note [1]

typedef signed long long Tally_t;    // Counts "occurences", signed due to a <0 test in Bayes calc

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Constants used throughout diagnostics


const int EVIDMAXCOR = 1;


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Structs used in diagnostics
// (Structs with only public field members, since to be P.B.R. for count data pushed from Kbase object)


struct SEvidTally { 

   const std::string query;
   size_t   evidStatus;     // evid is either: 0 = false, 1 = true, 2 = unevaluated 

   SEvidTally( std::string arg )
               :  query (arg),
                  evidStatus (2) {
   }
};


struct SHypoTally {

   const std::string hypothesis;
   Tally_t  priorCountsThisHypoTrue;                     // See Class Note [2]
   Tally_t  accumPostCountsJointToThisHypoTrue;
   short    priorProb_pctX10;
   short    postProb_pctX10;
   size_t   hypoStatus;                                  // See Class Note [1] 

   explicit SHypoTally( std::string arg )
      :  hypothesis (arg),
         priorCountsThisHypoTrue (0),
         accumPostCountsJointToThisHypoTrue (0),
         priorProb_pctX10 (0),
         postProb_pctX10 (0),
         hypoStatus (2) {
   }

/* STRUCT NOTES vv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv/

[1]   A case hypothesis is always (and only) either:
         0 = likely false (KB has no counts of it ever being true jointly with the accumulated evidence) 
         1 = likely true (KB gives it highest probability of all case hypos, whether that is prior to or
             joint to the accumulated evidence (i.e, it has the MAPP))
         2 = neither 0 or 1 (i.e., there is no basis yet for that hypo being 0 or 1)

      When case is created, all hypoStatus initially are 2.

      Certainty (being actually true or actually false) is established only when User teaches EA which
      case hypo is actually true (regardless of how much progress has been made by the diagostic). All
      case hypos other than the one actually true then become actually false.  The case then calls a
      KB learning method to teach the KB all of those hypo outcomes, as joint to the value (including
      'unevaluated') the case had for each evidence item when User identified the actual "true hypo". 

[2]   "Evaluated" = either a yes/true or a no/false value assigned to an evid by the case, whether
      via User query answer or by internal getter algorithm.  "Prior" = as counted before any
      evidence has been evaluated.  "Post" = as counted considering any evaluated evidence.
      "Joint" = counts within the set intersection of the accumulated evidence evaluations and a
      specified (true or false) hypothesis value.  "Accum" = across all evidence currently evaluated.

^^^^ END STRUCT NOTES */


};



/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Custom containers used in diagnostics

typedef std::vector<Nzint_t>              IdVec_t;  // vector to hold object IDs within any given class

typedef IdVec_t::iterator                 IdVecIter_t;
typedef IdVec_t::const_iterator           IdVecCiter_t;


typedef std::array<Tally_t,2> KnodeRow_t; // 2 columns in k-base node row (hypo = false and hypo = true)

typedef std::array<KnodeRow_t,3> Knode_t; // 3 rows in k-base node (evid = false, true, or unevaluated)

typedef std::vector<Nzint_t>                          MimicAxisE_t;
typedef MimicAxisE_t::const_iterator                  MimicCiterE_t;

typedef std::unordered_map< Nzint_t, MimicAxisE_t >   MimicAxisH_t;
typedef MimicAxisH_t::const_iterator                  MimicCiterH_t;

typedef std::unordered_map< Nzint_t, MimicAxisH_t >   MimicAxisR_t;
typedef MimicAxisR_t::const_iterator                  MimicCiterR_t;


typedef std::unordered_map< Nzint_t, SHypoTally >     HypoTallyMap_t;      // See File Note [2]

typedef HypoTallyMap_t::iterator                      HypoTallyMapIter_t;
typedef HypoTallyMap_t::const_iterator                HypoTallyMapCiter_t;


typedef std::unordered_map< Nzint_t, SEvidTally >     EvidTallyMap_t;

typedef EvidTallyMap_t::iterator                      EvidTallyMapIter_t;

// remove following after check

typedef std::vector<int>                              JointCountRows_t;     // See File Note [3]

typedef std::unordered_map<Nzint_t, JointCountRows_t>  JointCountCol_t;

typedef JointCountCol_t::iterator                     JointCountColIter_t;


#endif




/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   Nzint_t for indexing rows (vectors) in deques, & bins within those rows.  As unsigned int, it
      matches size_t type returned by size() in STL containers. But is made a distinct type here because
      per C++ stds, size_t is for expressing the size of a "thing" as the number of BYTES it
      occupies, not as a count of the "thing" itself.  So, the use intended here is to say:

            number of "things" in Nzint_t =
            bytes taken up by a range of "things" in size_t / sizeof( "thing" )

      As unsigned int, it is safe only for indicies and "counts"  
      Do no more than simple, "short" +/- math upon this type [can error upon forced upcasts to signed]

[2]   Map of structs, not ptrs to structs. HypoTally has no meaning outside an object of CCase class, so
      would be dumb to declare any outside CCase scope, mapping only pointers to them.  CCase objects
      do NOT have indefinite lifetimes, thus are under smart pointers, so proper destruction and memory
      release seems simpler if SHypoTally structs are made embedded, rather than linked heap, objects.

[3]   Each "joint count" (JC) column is a vector staring empty, getting a new row pushed on as each case
      evid is evaluated either 'F' or 'T'.  Value written into the new element is the counts read from
      Kbase at intersect of of oget has one row for each evid evaluated  One column for each case hypo.
      The columns across all hypos in a case for one evid in the case form a 'page'.  The pages across
      all evids in a case form the joint counts map for that case.    

--------------------------------------------------------------------------------
XXX END FILE NOTES */



//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
