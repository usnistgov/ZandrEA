// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares "integrating" abstract base class ISeqElement, which parents all the various concrete
   subclasses of objects triggered by the Application's CSequence class "Sequencer" object. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef SEQELEMENT_HPP
#define SEQELEMENT_HPP

#include "customTypes.hpp"
#include <array>
#include <memory>
#include <functional>
#include <exception>
#include <sstream>

// fwd declare
class AKnob;
class ASubject;
class CSequence;
class CView;


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// ISeqElement integrating declaration, for all classes whose objects are triggered by a CSequencer
 
class ISeqElement {

   public:

      const SEnergyPrices&    EnergyPriceRef;

      ~ISeqElement( void );

      int                  Trigger( Nzint_t,             // Sequencer pass trigger group intended "now" 
                                    const SClockRead& ); // Sequencer pass clock reading "now"
      const ASubject&      SaySubjectRefAsConst( void ) const;
      EApiType             SayApiType( void ) const;
      EDataLabel           SayLabel(void) const;   // add after domain & subject = "full" name of object
      EDataUnit            SayUnits( void ) const;
      EDataRange           SayRange( void ) const;
      EDataSuffix          SaySuffix( void ) const;
      EPlotGroup           SayPlotGroup( void ) const;
      Nzint_t              SayOwnTriggerGroup( void ) const;     // accessor for ctors of observers
      int                  SayTriggersPerCycle( void ) const;
      int                  SaySecsPerCycle( void ) const;
      int                  SaySecsPerTrigger( void ) const;
      bool                 IsValid( void ) const;
      void                 LendKnobKeysTo( std::vector<NGuiKey>& ) const;      // See Class Note [5]
      virtual void         LendHistogramKeysTo( std::vector<NGuiKey>& ) const; // Base defaults to NOP
      virtual void         LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ) const;
  
   protected:

      ISeqElement(   CSequence&,
                     ASubject&,
                     EApiType,
                     EDataLabel,
                     EDataUnit,
                     EDataRange,
                     EDataSuffix,
                     EPlotGroup,
                     int,                 // triggers/cycle
                     Nzint_t );           // base trigger group

   // Handles
      CSequence&        SeqRef;     // Not static so each instance can rgstr/refer to a specified seq.
      ASubject&         SubjRef;    // Not const so ISeqElement can send it alerts
      std::vector<std::unique_ptr<AKnob>>    u_KnobsOwned;           // See Class Note [3]

   // Fields
      std::vector<NGuiKey>                   knobKeys_ownedAndAntecedent; // See Class Note [7]
      const EApiType                         apiType;
      const EDataLabel                       label;
      const EDataUnit                        units;
      const EDataRange                       range;
      const EDataSuffix                      suffix;
      const EPlotGroup                       plotGroup;
      int                                    triggerCount;
      int                                    triggersPerCycle;             // See Class Note [2]
      int                                    triggersUntilCycle;
      int                                    secsPerCycle;
      const Nzint_t                          baseTriggerGroup;
      Nzint_t                                ownTriggerGroup;
      bool                                   cyclingAtMaxRate;
      bool                                   cycleBeginsNewCalendarDay;
      bool                                   cycleBeginsNewClockHour;
      bool                                   validNow;
      bool                                   validWas;
 
   // Methods
      bool              ShallTriggerInvokeCycle( void ); // Fixed code common inside all Cycle() overrides
      void              TagAndPostAlertToSubject( time_t, EAlertMsg );
      virtual void      LendAntecedentKnobKeysTo( std::vector<NGuiKey>& ) const;
      virtual void      CalcOwnTriggerGroup( void ); // Every concrete subclass must implement own calc
      virtual void      Cycle( time_t ) = 0;  // See Class Note [6]


/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Hour/Day methods here will return either host time or data-provided time, depending on value of
      taskGetsDataTime field in CTaskClock object ref'd by the CSequence object ref'd by SeqRef.
      Note this allows actual source of time data accessed by this class to be opaque to the class.

[2]   Although they hold values const for life, none of these protected fields can be made const, as
      their lifetime values must be set by ConfigureCycling() overrides called by subclass c-tors.

[3]   Must override (dispatch via v-table indexing type of object actually called from, i.e., a 'derived'
      subclass), instead of overload (static dispatch by matching signature, i.e., argument list), as
       
[4]   Class given default methods so that one arbitrary object of this class can reply (without throwing
      or crashing) to knob getter and setter calls from another arbitrary object of the class, regardless
      of whether either of those two objects actually (1) "owns" a knob itself (i.e. a "native" knob),
      or (2) holds a handle to another object of a subclass that owns a native knob or holds a handle
      to an object that does ("linked" knob).

      This provides a redundant guard upon the Developer having to know which concrete subclasses of
      ISeqElement are implemented with knobs, and which are not, when writing into any particular
      CreateTool() implementation the AddFeature() calls necessary to make knobs accessible to GUI.

[5]   The "system" providing the EA GUI with "Knobs" is that ISeqElement subclass c-tors make them, but
      emplace them into a base-class look-up table of std::unique ptrs (i.e., the ISeqElement base class
      "owns" them, not the subclass) that a base class method acceses.  Thus, access to "owned" knobs
      requires only a call to one non-virtual accessor of the base class (i.e., code calling for knobs
      does not need to know which subclasses they affect). Instead, it is the follow-on virtual method,
      LendInputObjectKnobIdsTo(), non-operating in the base class, that is overridden by
      subclasses getting input from other objects upstream providing their own "owned" knobs.

[6]   PVM since cannot have a generic Cycle().  Sole arg is time_t timestampNow, since states of Boolean
      timing beeps (i.e., cycleBeginsNewCalendarDay and cycleBeginsNewClockHour) get passed to Cycle()
      as member fields.

[7]   Starts empty and passed PBR to ctors of trace and histogram, then loaded by AttchOwnKnobs()
      routine and sent to antecedent object(s) at end of subclass ctor.      
  
^^^^^ END CLASS NOTES */

};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ