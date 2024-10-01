// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements a concrete class CKnowBaseH5 that is the higher-level intermediary between the inferencing
   engine of a CCase object and a knowledge base (KB) on disk.
   The CKnowBaseH5 methods instantiate and destroy objects of the namespace H5Kit, which are the 
   lower-level of intermediary code, calling functions of the HDF5 C-language API.
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "knowBase.hpp"
#include "knowParts.hpp"
#include "HDF5Parts.hpp"

#include <iostream>
#include <algorithm>

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV5
// Implementations for CKnowBaseH5

CKnowBaseH5::CKnowBaseH5(  std::string arg)
                           :  hdf5Filename (arg),
                              p_HypoById(),
                              p_EvidById(),
                              mimicBase(),
                              ruleX (0),
                              hypoY (0),
                              evidZ (0),
                              numNodesActive (0),
                              cursorAtActiveNode (false),
                              kbaseIsPreexistantHdf5File (false) {

   H5Kit::CFileCreatedIffAbsent KbaseCreator( arg );
   if ( KbaseCreator.WasPreexistingKbaseFound() ) {
      kbaseIsPreexistantHdf5File = RebuildMimicFromFile();
   }

}


CKnowBaseH5::~CKnowBaseH5(void) {

   // Empty d-tor
 }


//======================================================================================================/
// Private methods

void CKnowBaseH5::RezeroImage(void) {

   for (size_t row = 0; row < nodeImage.size(); ++row) {
      nodeImage.at(row).fill(0);
   }
   return;
}


bool CKnowBaseH5::RebuildMimicFromFile( void ) {  // **** TBD that this checks against known Rule Kit **

   bool reply = false;
   mimicBase.clear();   // TBD to check this releases all allocated memory including sub-elements
   H5Kit::CFileOpenedReadOnly FileOpenRO( hdf5Filename );
   hid_t fid = FileOpenRO.SayId();

   std::string pathRuleToHypo("");
   std::string pathFileToRule("");
   std::string pathRelativeToFile("/");

   H5Kit::CGroupIteratedOnSubgrps LookUpRulesHeld( FileOpenRO.SayId(), pathRelativeToFile, 'R' );

   if ( LookUpRulesHeld.SayNumSubgrpsFound() < 0 ) { return reply; }       // rule extraction fail
   else if ( LookUpRulesHeld.SayNumSubgrpsFound() == 0 ) { return reply; } // no rules found

   else {

      std::vector<Nzint_t> ruleIdsFound( LookUpRulesHeld.SayIdsFound() );

      for ( size_t i_r=0; i_r < ruleIdsFound.size(); ++i_r ) {

         pathFileToRule = "/" + ConvertIdToNameInKbase('R', ruleIdsFound.at(i_r));
         pathRelativeToFile = pathFileToRule;

         H5Kit::CGroupIteratedOnSubgrps LookUpHyposOnRule( FileOpenRO.SayId(), pathRelativeToFile, 'H' );

         std::vector<Nzint_t> hypoIdsOnRule( LookUpHyposOnRule.SayIdsFound() );

         for ( size_t i_h=0; i_h < hypoIdsOnRule.size(); ++i_h ) {

            pathRuleToHypo = "/" + ConvertIdToNameInKbase('H', hypoIdsOnRule.at(i_h));
            pathRelativeToFile = pathFileToRule + pathRuleToHypo;

            H5Kit::CGroupIteratedOnSubgrps LookUpEvidsOnHypo( FileOpenRO.SayId(), pathRelativeToFile, 'E' );

            std::vector<Nzint_t> evidIdsOnHypo( LookUpEvidsOnHypo.SayIdsFound() );
            std::vector<std::string> evidNamesOnHypo( LookUpEvidsOnHypo.SayNamesFound() );

            for ( size_t i_e=0; i_e < evidIdsOnHypo.size(); ++i_e ) {

               mimicBase[ ruleIdsFound.at(i_r) ][ hypoIdsOnRule.at(i_h) ].push_back( evidIdsOnHypo.at(i_e) );
            }
         }
      }
   }
   reply = true;
   return reply;
}


//======================================================================================================/
// Public methods


void CKnowBaseH5::InitializeNodeAt( Nzint_t ruleId,
                                    Nzint_t nomHypoId,   // hypo named ("nominal") in assoc to evid
                                    const CHypo* const p_nomHypo,
                                    Nzint_t evidId,
                                    const CEvid* const p_Evid,
                                    bool senseDirect,
                                    bool logicInvertible,
                                    const std::vector<Nzint_t>& idAllHyposAssocToRule) {

   if ( ! kbaseIsPreexistantHdf5File ) {

      /* "By design" because this method is called to initialize the knowledge base (KB) nodes needed
          to meet the expert system designer's original intent, versus any more nodes the KB learns
         "from experience" need to be activated later.
      */
      const size_t numHypos = idAllHyposAssocToRule.size();
      const bool oneOrMoreAltHypos = (numHypos > 1);
      H5Kit::CFileOpenedReadWrite KbaseOpenRW( hdf5Filename );

      /*
      At each node of Kbase is a type Knode "JOT" (joint occurence table) holding occurences (counts)
      joint to the R-H-E (Rule, Hypothesis, Evidence Item) intersection of that node.  Counts come
      from these two sources:
         (1) initialization by multiple, additive applications of various JOT update tables (JUT),
         which have same form as JOT and essentially are overlays used to modify them.
         (2) increments from solving diagnostic Cases (i.e., "experiences"). 
      A single local image of a JOT (nodeImage) is used for ALL reads and writes of Kbase nodes

      Interpret contig. 2-d STL containers (i.e., arrays, vectors) as [row][column]
      */

      RezeroImage();

      if (senseDirect) {   // evidence has "direct sense" to the nominal hypo, i.e., E -> H_nom

         nodeImage.at(1).at(1) = 1;       // mark up image with E -> H_nom

         if (logicInvertible) {

            nodeImage.at(0).at(0) = (numHypos - 1);    // then also mark up image with !E -> !H_nom
         }
      }
      else {               // evidence has "opposed sense" to the hypo, i.e., E -> !H_nom

         nodeImage.at(0).at(1) = 1;       // mark up image with !E -> H_nom

         if (logicInvertible) {

            nodeImage.at(1).at(0) = (numHypos - 1);    // then also mark up image with E -> !H_nom
         }
      }

      PanCursorToNodeAt( ruleId, nomHypoId, evidId );

      if ( cursorAtActiveNode ) { WriteImageToCursor(); }


   //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      /* Are other (alternative) hypos associated to the rule named by the caller besides the hypo
         named by the caller?
      */
      if (oneOrMoreAltHypos) {     // only true if numHypos >= 2

         /* If true, increment POA counts at all nodes where evidId intersect with an alternative hypo
         Proper POA increment at alternative nodes depends upon:
            1) the number of alternative hypos the rule has (i.e., those besides the nominal hypo)
            2) whether "sense" of evid to the nominal hypo is "direct" or "inverse".
            3) whether logic being asserted by the sense is or is not "invertible"
         */
         for (auto& altHypoId : idAllHyposAssocToRule) {

            if (altHypoId == nomHypoId) { continue; }

            RezeroImage();

            //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
            // Mark up image for hypo iterated as alternative to H_nominal

            if (senseDirect) {   // E -> H_nom

               nodeImage.at(0).at(0) = (numHypos - 2); nodeImage.at(0).at(1) = 1;
               nodeImage.at(1).at(0) = 1;
            }
            else {   // "opposite sense",  E -> !H_nom

               nodeImage.at(0).at(0) = 1;
               nodeImage.at(1).at(0) = (numHypos - 2); nodeImage.at(1).at(1) = 1;
            }

            //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
            // work complete on node at H_alt, write to kBase

            PanCursorToNodeAt( ruleId, altHypoId, evidId );

            if (cursorAtActiveNode) { WriteImageToCursor(); }
         }
      }
   }  // Close "if" on (! kbaseIsPreexistantHdf5File)

   //  kBase to hold pointers to all hypos and evids, so CCase objects only need hold a p_Rule

   p_HypoById.insert( std::pair<Nzint_t, const CHypo* const>( nomHypoId, p_nomHypo ) );
   p_EvidById.insert( std::pair<Nzint_t, const CEvid* const>( evidId, p_Evid ) );

   return;
}


void CKnowBaseH5::PanCursorToNodeAt( Nzint_t ruleId, Nzint_t hypoId, Nzint_t evidId) {

   /*    Pans cursor away from present coordinates only to the extent that each coordinate given is
   actually active on the axis to which it applies. If the three R-H-E coordinates given all lead to
   an active node, cursorAtActiveNode is set true
   */

   char progress = 'X';
   bool panAllowed = false;

   if ( (ruleId > 0) && (hypoId > 0) && (evidId > 0) ) {

      panAllowed = true;
      cursorAtActiveNode = false;

      // Do the R-H-E coords given intersect at a node "known" to Kbase (i.e., is node "active")?

      if ( mimicBase.count(ruleId) > 0 ) {
         ruleX = ruleId;
         progress = 'R';
         if ( mimicBase.at(ruleX).count(hypoId) > 0) {
            hypoY = hypoId;
            progress = 'H';
            if (  std::find(  mimicBase.at(ruleX).at(hypoY).begin(), // find() on a vector iterator
                              mimicBase.at(ruleX).at(hypoY).end(),
                              evidId)
                  != mimicBase.at(ruleX).at(hypoY).end()
            ) {
               evidZ = evidId;
               progress = 'E';
               cursorAtActiveNode = true;
            }
         }
      }
   }

   if ( panAllowed && ( !cursorAtActiveNode ) ) {

      H5Kit::CFileOpenedReadWrite KbaseOpenRW( hdf5Filename );
      std::string nameRuleToLink = ConvertIdToNameInKbase('R', ruleId);
      std::string nameHypoToLink = ConvertIdToNameInKbase('H', hypoId);
      std::string nameEvidToLink = ConvertIdToNameInKbase('E', evidId);

      switch ( progress ) {

         case 'X':   // Entire R-H-E intersection previously unknown to Kbase.  Link all three.
            {     // C++ requires declarations in a case stmt to be scoped locally to the case.
               H5Kit::CGroupCreated RuleLinked( KbaseOpenRW.SayId(), nameRuleToLink);
               H5Kit::CGroupCreated HypoLinked( RuleLinked.SayId(), nameHypoToLink);
               H5Kit::CDatasetCreated NodeMade( HypoLinked.SayId(), nameEvidToLink, 3, 2, 0 );
            }
            break;

         case 'R':   // Rule known, but it has no links (subgroups) for the hypo and evid given.
            {
               std::string dirPathToRule = ("/" + nameRuleToLink);
               H5Kit::CGroupOpened RuleOpenForLinks(KbaseOpenRW.SayId(), dirPathToRule);
               H5Kit::CGroupCreated HypoLinked(RuleOpenForLinks.SayId(), nameHypoToLink);
               H5Kit::CDatasetCreated NodeMade(HypoLinked.SayId(), nameEvidToLink, 3, 2, 0);
            }
            break;

         case 'H':   // Rule and hypo are known to be linked, but evid given is not linked to hypo.
            {
               std::string dirPathToHypoViaRule = ("/" + nameRuleToLink + "/" + nameHypoToLink);
               H5Kit::CGroupOpened HypoOpenForLinks(KbaseOpenRW.SayId(), dirPathToHypoViaRule);
               H5Kit::CDatasetCreated NodeMade(HypoOpenForLinks.SayId(), nameEvidToLink, 3, 2, 0);
            }
            break;

         default:
            break;
      }

      // Update mimicBase and counter to reflect the new node added to actual knowledge base
      mimicBase[ruleId][hypoId].push_back(evidId);
       ++numNodesActive;

      // Set cursor to the new node
      ruleX = ruleId;
      hypoY = hypoId;
      evidZ = evidId;
      cursorAtActiveNode = true;
   }
   return;
}


size_t CKnowBaseH5::SayNumNodesActive( void ) const { return numNodesActive; }


void CKnowBaseH5::InitializeCaseTallies(  Nzint_t caseRule,
                                          HypoTallyMap_t& caseHypoTally,
                                          EvidTallyMap_t& caseEvidTally,
                                          Tally_t& sumTruesOfAllCaseHypos ) {

   if (( ! mimicBase.empty() ) && ( mimicBase.count(caseRule) > 0) ) {

      // caseRule exists in kBase

      ruleX = caseRule;

      sumTruesOfAllCaseHypos = 0;               // Ensure zero in totalizing register of CCase caller
      Tally_t countsThisHypoTrue = 0;           // Declare/init registers for loops that follow

      MimicCiterH_t hypoCiter = mimicBase.at(ruleX).begin();

      //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      // Walk R-H-E space along the H-parallel at case rule.  Each element encountered is a "case hypo"

      while (hypoCiter != mimicBase.at(ruleX).end()) { // begin = end for any empty STL container

         // hypo (std::map) iter sees at least one element (hypo) associated to the case rule

         hypoY = hypoCiter->first;

         // insert case hypo into hypoTally
         caseHypoTally.insert( std::pair< Nzint_t, SHypoTally>
                                 ( hypoY, SHypoTally( p_HypoById.at(hypoY)->SayHypothesis() ))
         );
         MimicCiterE_t evidCiter = mimicBase.at(caseRule).at(hypoY).begin();

         //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
         // Walk along the E-parallel at current case hypo.  Each element encountered is a "case evid"

         while (evidCiter != mimicBase.at(caseRule).at(hypoY).end()) {

            // evid (std::vector) iter sees at least one element (evid) assoc. to the current case hypo

            evidZ = *evidCiter;
            cursorAtActiveNode = true; // Must be true if evidCiter hasn't yet hit end()

            /* Map insert non-ops if key already present (i.e, another case hypo already inserted the
               current evid into the case's EvidTally
            */
            caseEvidTally.insert(   std::pair<Nzint_t, SEvidTally>
                                       ( evidZ, SEvidTally( p_EvidById.at(evidZ)->SayQuery() ))
            );

            /* Consolidate from current kBase node all counts of current hypo = true across all values of
            current evid.  This happens even if another hypo has already inserted the current evid
            */
            RezeroImage();
            ReadCursorToImage();

            countsThisHypoTrue += PriorCountsHypoTrueWhenEvid(0);    // ...when evid false
            countsThisHypoTrue += PriorCountsHypoTrueWhenEvid(1);    // ...when evid true
            countsThisHypoTrue += PriorCountsHypoTrueWhenEvid(2);    // ...when evid unevaluated

            /* Step to next evid having any prior count joint to the current hypo being true
            (an "association")
            */

            ++evidCiter;
            evidZ = 0;                    // defaults for loop exit should evidCiter -> end
            cursorAtActiveNode = false;
         }
         //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
         // Current evid loaded into evidTally

         caseHypoTally.at(hypoY).priorCountsThisHypoTrue = countsThisHypoTrue;
         sumTruesOfAllCaseHypos += countsThisHypoTrue;
         countsThisHypoTrue = 0;
         ++hypoCiter;
         hypoY = 0;                   // default for loop exit should hypoCiter -> end
      }
      //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/
      // Current hypo loaded into hypoTally

      ruleX = 0;  // pan cursor to origin upon method rtn
   }
   return;
}


Tally_t CKnowBaseH5::PriorCountsHypoTrueWhenEvid( size_t iNodeRow ) const {

   // Caller must know whether node is active to correctly interpret a return value of "0" (zero)

   return (cursorAtActiveNode ? nodeImage.at(iNodeRow).at(1) : 0);
}


Tally_t CKnowBaseH5::PriorCountsHypoFalseWhenEvid( size_t iNodeRow ) const {

   // Caller must know whether node is active to correctly interpret a return value of "0" (zero)

   return (cursorAtActiveNode ? nodeImage.at(iNodeRow).at(0) : 0);
}


Tally_t CKnowBaseH5::PriorCountsHypoTrueNeverFalseWhenEvid( size_t iNodeRow ) const {

   return (PriorCountsHypoFalseWhenEvid(iNodeRow) == 0 ? PriorCountsHypoTrueWhenEvid(iNodeRow) : 0);
}


bool CKnowBaseH5::HasHypoEverBeenFalseWhenEvid( size_t evidValue ) const {

   // "Ever" versus "Only"

   return (PriorCountsHypoFalseWhenEvid(evidValue) == 0 ? false : true);
}


bool CKnowBaseH5::IsCursorAtActiveNode(void) const { return cursorAtActiveNode; }


void CKnowBaseH5::IncrementHypoTrueForEvid( size_t iNodeRow ) {

   if (cursorAtActiveNode) {

      ++nodeImage.at(iNodeRow).at(1);
   }
   return;
}


void CKnowBaseH5::IncrementHypoFalseForEvid( size_t iNodeRow ) {

   if (cursorAtActiveNode) {

      ++nodeImage.at(iNodeRow).at(0);
   }
   return;
}


void CKnowBaseH5::DevolveCounts( size_t numHyposPrior ) {

   size_t accumulate = 0;
   size_t numCasesTaught = 0;

   if (cursorAtActiveNode) {

      for ( auto nodeRow : nodeImage ) {

         // walking all rows: 'false', 'true', and 'unevaluated'

         for (auto countsInColumnOfRow : nodeRow) {

            // walking both cols: hypo = true and hypo = false

            accumulate += countsInColumnOfRow;
         }
      }
      numCasesTaught = accumulate - numHyposPrior;
   }
   return;
}



void CKnowBaseH5::DevolveCounts(void) {
   /*
   "Devolving" the counts held by a KB node is intended to reduce to minimimum the PROBABILITY of
   the hypothesis at that node being true or false for a future case, while retaining the POSSIBILITY
   of it being true or false for that case.  Such is only done on a hypo that the KB previously learned
   as actually true or false from a past case where the fault/cause associated with the hypo was
   subsequently fixed (which should nullify the high prior probability the KB would have for it).
   */

   if (cursorAtActiveNode) {

      for (auto nodeRow : nodeImage ) {

         // walks all rows, whether 'false', 'true', or 'unevaluated'

         for (auto countsInColumnOfRow : nodeRow) { // walks both cols, hypo = true and hypo = false

            if (countsInColumnOfRow > 1) { countsInColumnOfRow = 1; }
         }
      }
   }
   return;
}


bool CKnowBaseH5::ReadCursorToImage( void ) {

   bool reply = false;
   RezeroImage();

   if ( cursorAtActiveNode ) {

      H5Kit::CFileOpenedReadOnly KbaseOpenRO( hdf5Filename );
      std::string pathToNode =   "/" + ConvertIdToNameInKbase('R', ruleX) +
                                 "/" + ConvertIdToNameInKbase('H', hypoY) +
                                 "/" + ConvertIdToNameInKbase('E', evidZ);

      H5Kit::CDatasetOpened NodeOpenRO( KbaseOpenRO.SayId(), pathToNode );
      H5Kit::CDatasetMediator MediatorRO( nodeImage, false );
      MediatorRO.SetDatasetIdTo( NodeOpenRO.SayId() );
      MediatorRO.CopyDatasetToImage();
      reply = true;
   }
   return reply;
}


bool CKnowBaseH5::WriteImageToCursor( void ) {

   bool reply = false;

   if ( cursorAtActiveNode ) {

      H5Kit::CFileOpenedReadWrite KbaseOpenRW( hdf5Filename );
      std::string pathToNode =   "/" + ConvertIdToNameInKbase( 'R', ruleX ) +
                                 "/" + ConvertIdToNameInKbase( 'H', hypoY ) +
                                 "/" + ConvertIdToNameInKbase( 'E', evidZ );

      H5Kit::CDatasetOpened NodeOpenRW( KbaseOpenRW.SayId(), pathToNode );
      H5Kit::CDatasetMediator MediatorRW( nodeImage, true );
      MediatorRW.SetDatasetIdTo( NodeOpenRW.SayId() );
      MediatorRW.CopyImageToDataset();
      MediatorRW.FlushAfterWrite();
      reply = true;
    }
   return reply;
}


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
