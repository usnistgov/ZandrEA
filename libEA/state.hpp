// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Header declaring abstract base class AFact for all "Fact objects", plus all concrete classes derived
   from AFact.  Also here is implementation of templated concrete classes derived from AFact.
 */
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef FACT_HPP
#define FACT_HPP

// $$$ TBD get rid of templating in this header and move these includes from .hpp (risky) to .cpp $$$ 

#include "agentTask.hpp"      // register to sequence 
#include "seqElement.hpp"     // Inheiritance requires type completion, also brings customTypes
#include "stateParts.hpp"
#include "viewParts.hpp"      // get NGuiKey of traces, call d-tor on smart ptr to trace
#include "controlParts.hpp"   // Needed for upcast of CKnobFloat (for hyster) to AKnob
#include "rainfall.hpp"       // Needed for method calls in templated subclasses

#include <numeric>
#include <functional>
#include <string>
#include <memory>

// Forward declares (to avoid unnecessary #includes)
class AClock;
class ASubject;

class CController;
class CDomain;
class CPointAnalog;
class CPointBinary;
class CTraceRealtime;
class CView;

enum struct ESustainedAs : unsigned char {

   False = 0u,
   True,
   TrueXorFalse
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Declare an interface (top-of-chain abstract base class (ABC)) for ALL fact objects 

class AFact : public ISeqElement {

   public:

      const std::unique_ptr<CRainFact>          u_Rain;    // ptr is const, not the obj it points to.

   // Methods
      // Any ABC must define a virtual public d-tor so subclass dtors get called on its pointers
      virtual           ~AFact( void ) {  }  // empty d-tor

      // accessors, see file note [2]
      float             SaySpillage( void ) const;
      time_t            SayTimeOfClaimNow( void ) const;
      bool              Now( void ) const;
      bool              HasClaimFlipped( void ) const;

      void              LendRealtimeAccessTo( RtTraceAccessTable_t& ) const;

   protected:

   // Handles
      const std::unique_ptr<CTraceRealtime>  u_RealtimeTrace;
   // Vector of handles to input facts (operands)
      std::vector<AFact*>     p_Operands;                         // See Class Note [1]

   // Fields
      const size_t            numOperands;
      float                   spillage;
      time_t                  timeOfClaimNow;
      bool                    firstCycle;
      bool                    claimNow;
      bool                    claimWas;
      bool                    claimHasFlipped;
 

      // Methods

      AFact(  CSequence&,
               ASubject&,
               EDataLabel
      );

      AFact(  CSequence&,
               ASubject&,
               EDataLabel,
               std::vector<AFact*>
      );
};

//======================================================================================================/
/* Typedef a pointer to call any bool(void) getter method [ e.g., Now() ] on an AFact-derived object.*/

typedef bool (AFact::*PtrFact_t)( void );  // Pre-C++11 "member function pointer" (MFP) typedef syntax


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFactFromChart concrete class template declaration.

template <typename TTChart, typename TTChartMFptr>

/* 
   This concrete class must be templated because it must hold handle to a concrete subclass of chart, as
   it must hit a subclass-specific method for its chart result.
   Specify TTChart on any IChart concrete subclass having typedefs for pointer(s) to the member function
   (getter) accessing a subclass-specific chart result.  Specify TTChartMFptr on those typedefs.
   Those typdefs are declared at bottom of chart.hpp
*/
 
class CFactFromChart : public AFact { 

   public:

   // Methods
      CFactFromChart<TTChart, TTChartMFptr> (   CSequence& bArg0,
                                                ASubject& bArg1,
                                                EDataLabel bArg2,
                                                TTChart& arg0,
                                                TTChartMFptr arg1 )
                                                :  AFact(   bArg0,
                                                            bArg1,
                                                            bArg2
                                                   ),
                                                   ChartRef (arg0),
                                                   p_Getter (arg1) {
         CalcOwnTriggerGroup();
         ConfigureCycling();
         LendKnobKeysTo( knobKeys_ownedAndAntecedent );
       }
 
      ~CFactFromChart( void ) {  }


      void LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const override {

         NGuiKey ownHistogramKey = u_Rain->SayHistogramKey();

         bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                      borrowerRef.begin(),
                                                      borrowerRef.end(),
                                                      ownHistogramKey
                                                   ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( ownHistogramKey );
         }

         ChartRef.LendHistogramKeysTo( borrowerRef );

         return;
      }


      virtual void LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTableRef ) const override {

          ChartRef.LendRealtimeAnalogAccessTo( ptrTableRef );
         return;
      }


//=====================================================================================================/

   private:

   // Handles
      TTChart&       ChartRef;
      TTChartMFptr   p_Getter;    
 
   // Methods

      virtual void CalcOwnTriggerGroup( void ) override {

         Nzint_t latestInputTriggerGroup = ChartRef.SayOwnTriggerGroup();

         if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
            ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
         }
         else { ownTriggerGroup = baseTriggerGroup; }
         return;  
      }


      virtual void LendAntecedentKnobKeysTo( std::vector<NGuiKey>& borrowerRef ) const override {

         return ChartRef.LendKnobKeysTo( borrowerRef );
      }

      void ConfigureCycling( void ) {

         triggersPerCycle = ChartRef.SayTriggersPerCycle();
         triggersUntilCycle = triggersPerCycle;
         secsPerCycle = ( triggersPerCycle * SeqRef.SayTriggerPeriodSecs() );
         return;
      }


      virtual void Cycle(  time_t timestampNow ) override {

         validWas = validNow;
         validNow = ChartRef.IsValid();   // Passed from data channel object(s) upstream of Chart

         if ( validNow ) { 

            claimWas = claimNow;

            claimNow = (ChartRef.*p_Getter)();        // chart mbr function ptr to, for e.g., IsSteady()

            claimHasFlipped = ( claimNow != claimWas );

            timeOfClaimNow = (   ( claimHasFlipped || (validNow != validWas) || firstCycle ) ?
                                    timestampNow :
                                    timeOfClaimNow
            );
         }   
 
         u_Rain->Cycle( timestampNow,
                        cycleBeginsNewClockHour,
                        cycleBeginsNewCalendarDay,
                        claimNow,
                        validNow
         );

         firstCycle = false;
         return;
      }
   
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CFactRelatingLeftToRight concrete class template declaration.
   This class creates fact objects testing a logical relation (<, >, == ) between two ("Left" and
   "Right") analog (float) quantities, where both are primary (acquired) data (i.e., pulled from
   respective CRainAnalog objects).  The relation is tested via an embedded functor of a concrete
   subclass, derived from the CRelnFunctor A.B.C., upon which this class is templated.  If one of the 
   quantities is considered a guide to which the other refers, then "Right" must be that guide.
*/
template <  typename TTLeftObj,
            typename TTLeftMFptr,
            typename TTReln,
            typename TTRightObj,
            typename TTRightMFptr >

class CFactRelatingLeftToRight : public AFact {

   public:
   // Methods

       CFactRelatingLeftToRight< TTLeftObj,
                                 TTLeftMFptr,
                                 TTReln,
                                 TTRightObj,
                                 TTRightMFptr>( CSequence& bArg0,
                                                ASubject& bArg1,
                                                EDataLabel bArg2,
                                                TTLeftObj* const arg0,
                                                TTLeftMFptr arg1,
                                                TTRightObj* const arg2,
                                                TTRightMFptr arg3,
                                                std::array<float,3> arg4, // hysteresis minDefMax
                                                std::array<float,3> arg5, // slack minDefMax
                                                CController& arg6 ) 
                                                :  AFact(  bArg0,
                                                            bArg1,
                                                            bArg2
                                                   ),
                                                   // functor copy ctor (Deitel p. 482) :
                                                   Relate (arg4[1], arg5[1]),
                                                   p_LeftObject (arg0),
                                                   p_RightObject (arg2),
                                                   p_LeftGetter (arg1),
                                                   p_RightGetter (arg3),
                                                   p_LeftRain ( arg0->u_Rain.get() ),
                                                   p_RightRain ( arg2->u_Rain.get() ) {

         CalcOwnTriggerGroup(); //  see File Note [1]
         ConfigureCycling();
         AttachOwnKnobs( arg6, arg4, arg5, Relate.SayHysterCref(), Relate.SaySlackCref() );
         LendKnobKeysTo( knobKeys_ownedAndAntecedent );
      }

 
      ~CFactRelatingLeftToRight( void ) { /* empty */ }


      void LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const override {

         NGuiKey ownHistogramKey = u_Rain->SayHistogramKey();
         NGuiKey histogramKeyFromLeftObject = p_LeftRain->SayHistogramKey();
         NGuiKey histogramKeyFromRightObject = p_RightRain->SayHistogramKey();



         bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                      borrowerRef.begin(),
                                                      borrowerRef.end(),
                                                      ownHistogramKey
                                                   ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( ownHistogramKey );
         }

         borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                histogramKeyFromLeftObject
                                                ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( histogramKeyFromLeftObject );
         }

         borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                histogramKeyFromRightObject
                                                ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( histogramKeyFromRightObject );
         }
         return;
      }


      virtual void LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTableRef ) const override {

         p_LeftObject->LendRealtimeAnalogAccessTo( ptrTableRef );
         p_RightObject->LendRealtimeAnalogAccessTo( ptrTableRef );
         return;
      }

   private:
   // Functor
      TTReln               Relate;

   // Handles
      TTLeftObj* const     p_LeftObject; // template avoids separate field for every possible obj type
      TTRightObj* const    p_RightObject;

   // Pointers to specific member functions (defined by args passed) of CRain class
      TTLeftMFptr          p_LeftGetter;    // class MF ptrs bound to host objects at points of call
      TTRightMFptr         p_RightGetter;

      CRainAnalog* const   p_LeftRain;
      CRainAnalog* const   p_RightRain;

   // Methods

      virtual void CalcOwnTriggerGroup( void ) override {

         std::vector<Nzint_t> inputTriggerGroups(0);
         inputTriggerGroups.push_back( p_LeftObject->SayOwnTriggerGroup() );
         inputTriggerGroups.push_back( p_RightObject->SayOwnTriggerGroup() );
         Nzint_t latestInputTriggerGroup = *std::max_element(  inputTriggerGroups.begin(),
                                                               inputTriggerGroups.end(),
                                                               std::less<Nzint_t>()
         );   

         if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
            ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
         }
         else { ownTriggerGroup = baseTriggerGroup; }

         return;  
      }


      void AttachOwnKnobs( CController& ctrlrRef,
                           std::array<float,3> hysterMinDefMax,
                           std::array<float,3> slackMinDefMax,
                           const float& hysterCref,
                           const float& slackCref ) {

         u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                                    ctrlrRef,
                                    *this,
                                    EDataLabel::Knob_factRelate_hyster,
                                    p_LeftObject->SayUnits(),
                                    EDataSuffix::None,
                                    (  [&]( float userInputVetted ) -> void {
                                          Relate.SetHyster( userInputVetted );
                                          return; }
                                    ),
                                    hysterMinDefMax,
                                    hysterCref
                                 )
         );

         u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                                    ctrlrRef,
                                    *this,
                                    EDataLabel::Knob_factRelate_slack,
                                    p_LeftObject->SayUnits(),
                                    EDataSuffix::None,
                                    (  [&]( float userInputVetted ) -> void {
                                          Relate.SetSlack( userInputVetted );
                                          return; }
                                    ),
                                    slackMinDefMax,
                                    slackCref
                                 )
         );
         return;
       }


      void ConfigureCycling( void ) {

         triggersPerCycle = std::min(  p_LeftObject->SayTriggersPerCycle(),
                                       p_RightObject->SayTriggersPerCycle()
         );

         triggersUntilCycle = triggersPerCycle;
         secsPerCycle = ( triggersPerCycle * SeqRef.SayTriggerPeriodSecs() );

         return;
      }


      virtual void Cycle( time_t timestampNow ) override {

         validWas = validNow;
         validNow = (   p_LeftObject->IsValid() &&
                        p_RightObject->IsValid() );

        if ( validNow ) { 

            claimWas = claimNow;

            // " ->* " syntax binds method pointer to object pointer, properly completing call
            claimNow = Relate(   (p_LeftRain->*p_LeftGetter)(),
                                 (p_RightRain->*p_RightGetter)() );

            claimHasFlipped = ( claimWas != claimNow );

            timeOfClaimNow = (   ( claimHasFlipped || (validWas != validNow) || firstCycle ) ?
                                    timestampNow :
                                    timeOfClaimNow
            );
         }   

         u_Rain->Cycle( timestampNow,
                        cycleBeginsNewClockHour,
                        cycleBeginsNewCalendarDay,
                        claimNow,
                        validNow 
         );
 
         spillage = Relate.SaySpillage();
         firstCycle = false;
         return;
      }

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CStateRelateLeftParam concrete class template declaration.
   Class instance relates acquired data ("Left") to ref. parameter ("Param") via lambda call to ASubject
*** TBD the part of this fetching param value from an ASubject object. Currently param is hard coded *** 
*/
template <  typename TTLeftObj,
            typename TTLeftMFptr,
            typename TTReln >

   class CFactRelatingLeftToParam : public AFact { // 

   public:
      // Methods

      CFactRelatingLeftToParam<  TTLeftObj,
                                 TTLeftMFptr,
                                 TTReln>( CSequence& bArg0,
                                          ASubject& bArg1,
                                          EDataLabel bArg2,
                                          TTLeftObj* const arg0,
                                          TTLeftMFptr arg1,
                                          //const std::function< float (void) > arg2,
                                          float param,    // $$$ TBD to not hard code this $$$
                                          std::array<float,3> arg2, // hysteresis minDefMax
                                          std::array<float,3> arg3, // slack minDefMax
                                          CController& arg4 ) 
                                          :  AFact(  bArg0,
                                                      bArg1,
                                                      bArg2
                                             ),
                                             Relate (arg2[1], arg3[1]), // functor copy ctor (Deitel p. 482)
                                             p_LeftObject (arg0),
                                             p_LeftGetter(arg1),
                                             p_LeftRain ( arg0->u_Rain.get() ),
                                             paramNow (param) {

         CalcOwnTriggerGroup();
         ConfigureCycling();
         AttachOwnKnobs( arg4, arg2, arg3, Relate.SayHysterCref(), Relate.SaySlackCref() );
         LendKnobKeysTo( knobKeys_ownedAndAntecedent );
      }


      ~CFactRelatingLeftToParam( void ) { }

      void LendHistogramKeysTo( std::vector<NGuiKey>& borrowerRef ) const override {

         NGuiKey ownHistogramKey = u_Rain->SayHistogramKey();
         NGuiKey histogramKeyFromLeftObject = p_LeftRain->SayHistogramKey();

         bool borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                      borrowerRef.begin(),
                                                      borrowerRef.end(),
                                                      ownHistogramKey
                                                   ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( ownHistogramKey );
         }
         borrowerNotAlreadyHoldingSameKey = ( std::find(
                                                borrowerRef.begin(),
                                                borrowerRef.end(),
                                                histogramKeyFromLeftObject
                                                ) == borrowerRef.end() ?
                                                   true :
                                                   false
         ); 
         if ( borrowerNotAlreadyHoldingSameKey ) {

            borrowerRef.push_back( histogramKeyFromLeftObject );
         }
         return;
      }

   
      virtual void LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ptrTableRef ) const override {

         p_LeftObject->LendRealtimeAnalogAccessTo( ptrTableRef );
         return;
      }

   private:

      // Functor
      TTReln                                 Relate;

      // Handles
      TTLeftObj* const                       p_LeftObject;

      // Pointers to specific member functions (defined by args passed) of CRain class
      // class MF ptrs bound to host objects at points of call
      TTLeftMFptr                            p_LeftGetter;

      CRainAnalog* const                     p_LeftRain;

      //const std::function<float(void)>       GetParam; // Calls via SubjRef held by ISeqElement

      // Fields
      float                                  paramNow;

      // Methods

      virtual void CalcOwnTriggerGroup( void ) override {

         Nzint_t latestInputTriggerGroup = p_LeftObject->SayOwnTriggerGroup();

         if ( ! (latestInputTriggerGroup < baseTriggerGroup) ) {
            ownTriggerGroup = ( latestInputTriggerGroup + 1u ); 
         }
         else { ownTriggerGroup = baseTriggerGroup; }
         return;  
      }


      void AttachOwnKnobs( CController& ctrlrRef,
                           std::array<float,3> hysterMinDefMax,
                           std::array<float,3> slackMinDefMax,
                           const float& hysterCref,
                           const float& slackCref ) {

         u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                                    ctrlrRef,
                                    *this,
                                    EDataLabel::Knob_factRelate_hyster,
                                    p_LeftObject->SayUnits(),
                                    EDataSuffix::None,
                                    (  [&]( float userInputVetted ) -> void {
                                          Relate.SetHyster( userInputVetted );
                                          return; }
                                    ),
                                    hysterMinDefMax,
                                    hysterCref
                                 )
          );

         u_KnobsOwned.push_back( std::make_unique<CKnobFloat>(
                                    ctrlrRef,
                                    *this,
                                    EDataLabel::Knob_factRelate_slack,
                                    p_LeftObject->SayUnits(),
                                    EDataSuffix::None,
                                    (  [&]( float userInputVetted ) -> void {
                                          Relate.SetSlack( userInputVetted );
                                          return; }
                                    ),
                                    slackMinDefMax,
                                    slackCref
                                 )
          );

      }


      void ConfigureCycling( void ) {

         triggersPerCycle = p_LeftObject->SayTriggersPerCycle();

         triggersUntilCycle = triggersPerCycle;
         secsPerCycle = ( triggersPerCycle * SeqRef.SayTriggerPeriodSecs() );

         return;
      }


      virtual void Cycle(  time_t timestampNow ) override {

         validWas = validNow;
         validNow = p_LeftObject->IsValid();

         if ( validNow ) { 

            claimWas = claimNow;

            // " ->* " syntax binds method pointer to object pointer, properly completing call
            claimNow = Relate( (p_LeftRain->*p_LeftGetter)(), paramNow );

            claimHasFlipped = (claimNow != claimWas);
            timeOfClaimNow = (   ( claimHasFlipped || (validWas != validNow) || firstCycle ) ?
                                    timestampNow :
                                    timeOfClaimNow
            );
         }   

         u_Rain->Cycle( timestampNow,
                        cycleBeginsNewClockHour,
                        cycleBeginsNewCalendarDay,
                        claimNow,
                        validNow
         );
         spillage = Relate.SaySpillage();

         firstCycle = false;
         return;
      }

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFactFromFacts concrete class template declaration.

class CFactFromFacts : public AFact { 

   public:
   // Methods
      CFactFromFacts(   CSequence&,
                        ASubject&,
                        EDataLabel,
                        std::vector<AFact*>,
                        std::function<bool(void)> );     // Must be PBV [see tool.cpp]
  
      ~CFactFromFacts( void );

      virtual void      LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;


   private:

   // Lambda of boolean expression, capturing operands by ref, wrapped in std::function 
      const std::function<bool(void)>  Statement; 

   // Methods
      void              ConfigureCycling( void );
      bool              PullValidity( void );
      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      LendAntecedentKnobKeysTo( std::vector<NGuiKey>& ) const override;
      virtual void      Cycle( time_t ) override;

      
/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

[1]   Handles to the input objects are required to get validities, etc. These requirements are not 
      addressed via the wrapped callbacks that Cycle() makes to methods residing on input objects to
      yield result.  STL containers such as vector<> do not allow references as elements (elements must
      be "assignable"), so pointers must be used.  Since ownership of target is not being transferred, a
      raw pointer is appropriate, a smart ptr is unnecessary.

^^^^ END CLASS NOTES */

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFactFromCloclStateSched concrete class template declaration.

class CFactFromClock : public AFact { 

   public:
   // Methods
      CFactFromClock(   CSequence&,
                        ASubject&,
                        EDataLabel );
  
      ~CFactFromClock( void );
      void SetCondition( std::function<bool(void)> ); 

      // getters calling getters on ClockRef (public to allow lambda defn, TBD for a private approach)
      int Hour( void ) const;       // integer 24-hr clock
      int Day( void ) const;        // integer from 0 (Sunday) thru 6 (Saturday)

   private:
 
   // Lambda defining the schedule condition using "clock repeater" fields, wrapped in std::function 
      std::function<bool(void)>  Condition;  // condition is either in effect (true) or not (false)

      const AClock&              ClockRef;   // This AFact subclass needs more info than in SClockRead

   // Methods
      virtual void               Cycle( time_t ) override;
      virtual void               CalcOwnTriggerGroup( void ) override;

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////

class CFactFromAntecedentSubject : public AFact { 

   public:

   // Methods
      CFactFromAntecedentSubject(   CSequence&,
                                    ASubject&,        // Owning Tool object's own Subject
                                    EDataLabel,       // Fact's own label
                                    CDomain&,
                                    ERealName );      // antecedent subj name 
  
      ~CFactFromAntecedentSubject( void );

      virtual void      LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;

      const ASubject* const               p_AnteSubj;
      const ERealName                     anteSubjName;

   // Methods
      virtual void      Cycle( time_t ) override;
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
/* CStateAnalogSrc (an analog source [of binary true/false values]) = concrete class for binary
   information that tool must receive as floating point numbers, arriving at tool thru a CPointAnalog
   object
*/ 

class CFactFromPoint : public AFact { 

   public:

   // Methods
      CFactFromPoint(  CSequence&,
                        ASubject&,
                        EDataLabel,
                        const CPointBinary& ); 
  
      ~CFactFromPoint( void );

      virtual void      LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;

   private:

   // Handle
      const CPointBinary&     PointRef;

   // Field
      bool                    binaryPostedByPoint;
                        
   // Methods
      virtual void            Cycle( time_t ) override;

};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// CFactSustainedAfter concrete class template declaration.

class CFactSustained : public AFact { 

   public:
   // Methods
      CFactSustained(   CSequence&,
                        ASubject&,
                        EDataLabel,
                        const AFact&,
                        const ESustainedAs,
                        std::array<int,3>,
                        CController& );
  
      ~CFactSustained( void );

      virtual void  LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;

   private:
   // Handle
      const AFact&     FactToWatchRef;

   // Fields
      const ESustainedAs   watchMode;
      int                  minCyclesToSustain;
      int                  cyclesSustained;
      bool                 claimToWatch;

   // Methods
      void              AttachOwnKnobs( CController&, std::array<int,3>);
      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      Cycle( time_t ) override;

      
/* CLASS NOTES vvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

^^^^ END CLASS NOTES */
};


#endif

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   'this' pointer of a non-template interface must be used to register objects of templated subclass,
      otherwise the template param has to be supplied, making impossible one vector holding handles to
      all AFact objects.

--------------------------------------------------------------------------------
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
