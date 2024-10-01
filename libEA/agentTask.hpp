// This file is in EA library of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declaration of CSequence (distributes trigger command to all ISeqElement objects in the
   Application) and CSeqTimeAxis class (provides time axis to time-series plots on GUI)
   There is no "agentTask.cpp" file.  These classes are implemented in "sequence.cpp"   
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef AGENTTASK_HPP
#define AGENTTASK_HPP

#include "customTypes.hpp"
#include <memory>

// Forward declarations
class AChart;
class ADataChannel;
class AFact;

class CAgent;
class CClockPerPort;
class CFormula; 
class CRuleKit;
class CSeqTimeAxis;

class ISeqElement;


const size_t NUMCLASSESINTRIGGERLOOP = 4;
  // 1-channels ("points"), 2-formulas, 3-charts, 4-facts (rules trigger later, outside of "loop")

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* Only one "agent task" currently defined -> a real-time "Sequence" of data acquistion, processing, and
   analysis
*/

class CSequence { 

   public:
   // Handles

      const CClockPerPort&                   SeqClockRef;
      std::unique_ptr<CSeqTimeAxis>          u_TimeAxis;
   
   // Methods

     CSequence(  CAgent&,
                 CClockPerPort& );

     ~CSequence( void );

     EGuiReply          Trigger( const SClockRead& );    // See Class Note [1]
     int                SayTriggerPeriodSecs( void );

     void              Register( ADataChannel* );
     void              Register( CFormula* );
     void              Register( AChart* );
     void              Register( AFact* );
     void              Register( CRuleKit* ); 

   private:

   // Fields
      std::vector<int>        numObjectsOfClass;   // int vs. size_t since called by STL accumulator
      std::vector<int>        objectsTriggered;    // Indicies: Pts.=0, Formula=1, Chart=2, Fact=3
      std::vector<Nzint_t>    baseTriggerGrp;
      std::vector<bool>       allTriggered;
      int                     totalObjects;
      time_t                  timeByHost;
      Nzint_t                 groupTargeted;
      Nzint_t                 lapIncrement; // goes ++ when full lap of while-loop leaves objects untriggered
      bool                    allObjectsUpdated;
      bool                    configured;

      std::vector<ISeqElement*>              p_Points;
      std::vector<ISeqElement*>              p_Formulae;
      std::vector<ISeqElement*>              p_Charts;
      std::vector<ISeqElement*>              p_Facts;
      std::vector<CRuleKit*>                 p_RuleKits;

   // Methods

      void                                   Configure( void );

/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Triggering sends out basic info from clock so ISeqElement objects downstream do not need to run
      getters back to AClock just to get basic time-of-day info (i.e., saves CPU cycles).  ISeqElement
      subclasses needing more than basic info will hold a const ref back to the AClock object.
 
^^^^ END CLASS NOTES */


};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CSeqTimeAxis class is link between AClock and a time series of multiple triggers by a CSequence
object of many objects of ISeqElement concrete subclasses.  One CSequence object may own multiple
CSeqTimeAxis so to time-stamp cycles of ISeqElements working at different "triggers per cycle" rates

CSeqTimeAxis is then known (but not owned) and utilized by CView object of each ISubject to provide
time axis for realtime Kronos.  Both CView and CSeqTimeAxis have same indef lifetime so raw ptr is used
*/

class CSeqTimeAxis {

   public:

   // Methods
      CSeqTimeAxis(  CSequence&,
                     int );

      ~CSeqTimeAxis( void );

      void                          Trigger( time_t );

      KronoTimeStampsOldToNew_t     DisplayAxisFromSnapshotSet( Nzint_t ) const;
      KronoTimeStampsOldToNew_t     DisplayAxisInRealtime( void ) const;
      time_t                        SayTimeNewest( void ) const;
      int                           SaySecsPerCycle( void ) const;
      EGuiReply                     ResizeLoggingToAtLeastSecsAgo( int );
      void                          CreateSnapshotForSetSgi( Nzint_t );
      void                          DestroySnapshotForSetSgi( Nzint_t );

   private:

   // Fields
      SnapshotTimeAxesBank_t        snapshots_bySetSgi;
      std::deque<time_t>            timesHeld_newestToOldest;
      const std::string             caption;
      const int                     secsPerCycle;
      int                           secsLogging;
      bool                          firstCall;
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
