// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares a concrete class CKnowBaseH5 that is the higher-level intermediary between the inferencing
   engine of a CCase object and a knowledge base (KB) on disk.
   The CKnowBaseH5 methods instantiate and destroy objects of the namespace H5Kit, which are the 
   lower-level of intermediary code, calling functions of the HDF5 C-language API.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef KNOWBASE_HPP
#define KNOWBASE_HPP

#include "diagnosticTypes.hpp"

class CEvid;
class CHypo;

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Knowledge base

class CKnowBaseH5 {

public:

   explicit CKnowBaseH5( std::string );
   ~CKnowBaseH5( void );

   void     InitializeNodeAt( Nzint_t,
                              Nzint_t,
                              const CHypo* const,
                              Nzint_t,
                              const CEvid* const,
                              bool,
                              bool,
                              const std::vector<Nzint_t>& );

   void     PanCursorToNodeAt( Nzint_t, Nzint_t, Nzint_t );
   bool     ReadCursorToImage( void );
   bool     WriteImageToCursor( void );
   size_t   SayNumNodesActive( void ) const;
   
   void     InitializeCaseTallies(  Nzint_t,                      // See Class Note [1]
                                    HypoTallyMap_t&,
                                    EvidTallyMap_t&,
                                    Tally_t& );

   Tally_t  PriorCountsHypoTrueWhenEvid( size_t ) const;
   Tally_t  PriorCountsHypoFalseWhenEvid( size_t ) const;
   Tally_t  PriorCountsHypoTrueNeverFalseWhenEvid( size_t ) const;
   bool     HasHypoEverBeenFalseWhenEvid( size_t ) const;
   bool     IsCursorAtActiveNode( void ) const;
   void     IncrementHypoTrueForEvid( size_t );
   void     IncrementHypoFalseForEvid( size_t );
   void     DevolveCounts( size_t );
   void     DevolveCounts( void );


private:

   // Fields

   const std::string                                  hdf5Filename;
   Knode_t                                            nodeImage;
   std::unordered_map<Nzint_t, const CHypo* const>    p_HypoById;
   std::unordered_map<Nzint_t, const CEvid* const>    p_EvidById;
   MimicAxisR_t                                       mimicBase;
   Nzint_t                                            ruleX;
   Nzint_t                                            hypoY;
   Nzint_t                                            evidZ;
   size_t                                             numNodesActive;
   bool                                               cursorAtActiveNode;
   bool                                               kbaseIsPreexistantHdf5File;

   // Methods
   void              RezeroImage( void );
   bool              RebuildMimicFromFile( void );

};


#endif 

//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX
/* NOTES

[1]   All member functions implicitly (meaning, it doesn't look like it from their signatures) have
      their object's 'this' pointer supplied as their first argument.  STL way of pointing to a member
      function must "stick-shift in" the object pointer as a member functions "hidden" first arg.



^^^^^^^^^^^^^^END OF NOTES
*/


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ