// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implements "integrating" abstract base class ISeqElement, which parents all the various concrete
   subclasses of objects triggered by the Application's CSequence class "Sequencer" object. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "seqElement.hpp"
#include "subject.hpp"
#include "controlParts.hpp"
#include "agentTask.hpp"
#include "taskClock.hpp"

#include <iomanip>
#include <algorithm>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract Base class c-tor called by c-tors of all ISeqElement subclasses

ISeqElement::ISeqElement(  CSequence& arg0,
                           ASubject& arg1,
                           EApiType arg2,
                           EDataLabel arg3,
                           EDataUnit arg4,
                           EDataRange arg5,
                           EDataSuffix arg6,
                           EPlotGroup arg7,
                           int arg8,
                           Nzint_t arg9 )
                           :  EnergyPriceRef ( arg1.SayEnergyPricesRef() ),
                              SeqRef (arg0),         // TBD change to ptr so handle can be static
                              SubjRef (arg1),
                              u_KnobsOwned(0),
                              knobKeys_ownedAndAntecedent(0),
                              apiType (arg2),
                              label (arg3),
                              units (arg4),
                              range (arg5),
                              suffix (arg6),
                              plotGroup (arg7),
                              triggerCount (0),
                              triggersPerCycle (arg8),  // See File Note [1]
                              triggersUntilCycle (1),
                              secsPerCycle ( SeqRef.SayTriggerPeriodSecs() * arg8 ),
                              baseTriggerGroup (arg9),
                              ownTriggerGroup (arg9),   // All concrete ctors must overwrite this
                              cyclingAtMaxRate (arg8 == 1),
                              cycleBeginsNewCalendarDay (false),
                              cycleBeginsNewClockHour (false),
                              validNow (true),
                              validWas (true) {

   if ( arg8 > FIXED_SEQELEMENT_TRIGGERSPERCYCLE_MAX ) {
      throw std::logic_error( "Seq element tpc greater than allowed" );
   }

};


ISeqElement::~ISeqElement( void ) { };


//=====================================================================================================/
// Private Methods

void ISeqElement::LendAntecedentKnobKeysTo( std::vector<NGuiKey>& borrowerRef) const {

   // Base class defaults to NOP; concrete subclasses having antecedent object(s) must override
   return;
}


void ISeqElement::CalcOwnTriggerGroup( void ) { ownTriggerGroup = baseTriggerGroup; return; }

bool ISeqElement::ShallTriggerInvokeCycle( void ) {

   bool reply = false;

   --triggersUntilCycle;   // So, all ISeqElement-derived obj must initialize triggersUntilCycle = 1  

   if (triggersUntilCycle == 0) { // side-eff of call is to decrement counter that trips cycling
      reply = true;
      triggersUntilCycle = triggersPerCycle;
   }
   return reply;
}


void ISeqElement::TagAndPostAlertToSubject(  time_t timestampFromSubclassCycleContext,
                                             EAlertMsg alertFromSource ) {

   SubjRef.TagAndPostAlertToDomain( timestampFromSubclassCycleContext,
                                    label,
                                    alertFromSource );
   return;
 }

//=====================================================================================================/
// Public Methods

// *** TBD for making fields holding AClock data as STATIC versus non-static members ***

int ISeqElement::Trigger( Nzint_t groupTargeted, const SClockRead& clockReadingNow ) {

   int reply = 0; // Want int reply, so can tally replies across iterated containers
   triggerCount = clockReadingNow.triggerCount; // better this than a simple increment (re. limiter)

   if ( ownTriggerGroup == groupTargeted ) {

      // Any reset here "sticks" until read and cleared upon next cycle
      if ( clockReadingNow.newHour ) { 
         if ( clockReadingNow.newDay ) { // never starting a new day without also starting a new hour
            cycleBeginsNewCalendarDay = true;
         }
         cycleBeginsNewClockHour = true;
      }

      // Simpler/faster test of bool avoids many unnecessary calls to ShallTriggerInvokeCycle()
      if ( cyclingAtMaxRate ) {
         Cycle( clockReadingNow.timestamp );
         cycleBeginsNewCalendarDay = false;
         cycleBeginsNewClockHour = false;
      } 
      else if ( ShallTriggerInvokeCycle() ) {
         Cycle( clockReadingNow.timestamp );
         cycleBeginsNewCalendarDay = false;
         cycleBeginsNewClockHour = false;
      }

/* Whether/not Cycle() executed, when targeted trigger grp matches own, object reply = 1
   ("trigger counted"), since rounds of triggers do not necessarily produce equal numbers of cycles.   
*/
      reply = 1;
   }
   return reply;
}


const ASubject& ISeqElement::SaySubjectRefAsConst( void ) const { return SubjRef; }

EApiType ISeqElement::SayApiType(void) const { return apiType; }

EDataLabel ISeqElement::SayLabel( void ) const { return label; }

EDataUnit ISeqElement::SayUnits( void ) const { return units; }

EDataRange ISeqElement::SayRange( void ) const { return range; }

EDataSuffix ISeqElement::SaySuffix( void ) const { return suffix; }

EPlotGroup ISeqElement::SayPlotGroup( void ) const { return plotGroup; }

Nzint_t ISeqElement::SayOwnTriggerGroup( void ) const { return ownTriggerGroup; }

int ISeqElement::SayTriggersPerCycle( void ) const { return triggersPerCycle; }

int ISeqElement::SaySecsPerCycle( void ) const { return secsPerCycle; }

int ISeqElement::SaySecsPerTrigger( void ) const { return SeqRef.SayTriggerPeriodSecs(); }

bool ISeqElement::IsValid( void ) const { return validNow; }


void ISeqElement::LendKnobKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   bool borrowerNotAlreadyHoldingSameKey = NaNBOOL;

   for ( const auto& cp_Knob : u_KnobsOwned ) {

      NGuiKey keyHeld = cp_Knob->SayGuiKey();

      borrowerNotAlreadyHoldingSameKey = (   (  std::find(
                                                   borrowerRef.begin(),
                                                   borrowerRef.end(),
                                                   keyHeld
                                                ) == borrowerRef.end() ?
                                                true :
                                                false
                                             )
      ); 
      if ( borrowerNotAlreadyHoldingSameKey ) {

         borrowerRef.push_back( keyHeld );
      }
   }

   // following sub-call polymorphic; relies on this method being called on a concrete-class handle (?)
   LendAntecedentKnobKeysTo( borrowerRef );
   return;
}


void ISeqElement::LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const {

   // Base class defaults to NOP; a concrete subclass having rainfall and histogram must override
   return;
}


void ISeqElement::LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTable ) const {

   // Base class defaults to NOP; concrete subclasses having analog real-time Traces must override
   return;
}


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   Typ for protected fields related to cycling, initialized with default values that may be
      overwritten by ConfigureCycling() override call in c-tor of ISeqElement subclasses that can cycle
      at rates below the maximum (i.e., at less the rate the object is triggered to cycle).

      These fields cannot be const, as some ISeqElement subclasses must call ConfigureCycling() to
      calculate triggers/cycle from the particular object(s) from which they obtain data.

[2]   Side-effect of CycleFromTriggerNow() is to decrement counter to next cycle.  Current design does
      not differentiate in reply as to whether 
       
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ