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
   Declares abstract base class AChart and concrete classes for all Chart objects.    
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef CHART_HPP
#define CHART_HPP

#include "seqElement.hpp"
#include <functional>
#include <string>

// Forward declares (to avoid unnecessary #includes)
class ASubject;
class CController;
class CFormula;
class CPointAnalog;
class CRainAnalog;
class CSequence;


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Declare an abstract base class (ABC) for ALL chart objects.
/*
What all "charts" have in common is that they process values and statistics pulled from rainfall objects
(whether analog or binary) into one or more boolean variables evaluating as true/false some dynamic
feature of the acquired data held in that rainfall (e.g., that the data is "steady", etc.).
*/ 

class AChart  : public ISeqElement {

   public:
   // Methods
      // Any ABC must define a virtual public dtor so subclass dtors get called on its pointers
      virtual ~AChart( void ) { /* empty */ }

      void     LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ) const override;

   protected:

    // Handles
      CPointAnalog* const              p_ObsvdPoint;
      CFormula* const                  p_ObsvdFormula;
      CPointAnalog* const              p_GuidePoint;
      CFormula* const                  p_GuideFormula;

      CRainAnalog* const               p_ObsvdRain;
      CRainAnalog* const               p_GuideRain;

   // Fields
      bool  chartRunning;
      bool  resetPending;   // approach taken is to NOT re-zero register fields DIRECTLY

   // Methods

      AChart(  CSequence&,
               ASubject&,
               EApiType,
               CPointAnalog* const,
               CFormula* const,
               CPointAnalog* const,
               CFormula* const);

      static size_t        FitRainCyclesToSecondsOrRtnNaN( int, int );  

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CChartShewhart concrete class declaration
// Applies Shewhart-like chart to detect when an input departs or resumes "steady" behavior
 
class CChartShewhart : public AChart { 

   public:
   // Methods
      CChartShewhart(   CSequence&,
                        ASubject&,
                        CPointAnalog* const,
                        CController& );

      CChartShewhart(   CSequence&,
                        ASubject&,
                        CFormula* const,
                        CController& );

      ~CChartShewhart( void );

      size_t         GetNumCyclesBeingUsed( void ) const;
      bool           IsSteady( void ) const;
      virtual void   LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;

 
  private:

  // Fields
      float    stdDevRef;
      float    stdDevRefNew;
      float    xMean;
      float    xNow;
      float    xStdDev;
      float    zNow;
      float    zPass;
      size_t   numCyclesBeingUsed;
      size_t   rainDepthBeingUsed;
      int      numSecsBeingUsed; // Need only if Knob object dialog with GUI is in secs, not cycles
      int      tripFreeCount;
      int      tripFreeMargin;
      bool     abovePassband;
      bool     belowPassband;
      bool     chartTrip;
      bool     flipState;
      bool     isSteadyNow;
      bool     wasSteadyOnLastValid;
 
  // Methods
 
      void              ConfigureCycling( void );
      void              AttachOwnKnobs( CController& );
      bool              PullInput( void );
      float             SignX( float );
      EGuiReply         SetNumSecsBeingUsedTo( int ); // Private, as only accessed via knob (unlike getter)
      bool              ApplyChart( void );

      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      Cycle( time_t ) override;
      virtual void      LendAntecedentKnobKeysTo( std::vector<NGuiKey>& ) const override;
};



//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CChartTracking concrete class declaration
// Applies CUSUM-like chart to detect hunt or drift of an input off of its own (i.e., "auto") mean

class CChartTracking : public AChart { 

   public:
   // Constructors/destructor

       // tracking referenced to self (autoregressive), suspensions per Shewhart chart result
      CChartTracking(   CSequence&,
                        ASubject&,
                        CPointAnalog*,
                        CChartShewhart*,
                        std::array<float,3>,      // halfband = (+ or -) in units of x
                        std::array<float,3>,      // warn = (units-of-x)-minutes over a reset
                        CController& );

      // $$$ c-tor for parametric tracker is TBD

      // tracking referenced to second CRainfall object, suspensions per EITHER Shewhart result
      CChartTracking(   CSequence&,
                        ASubject&,
                        CPointAnalog*,
                        CChartShewhart*,
                        CPointAnalog*,
                        CChartShewhart*,
                        std::array<float,3>,      // halfband = (+ or -) in units of x
                        std::array<float,3>,      // warn = (units-of-x)-minutes over a reset
                        CController& );

 
      ~CChartTracking( void );

      bool           IsFalling( void ) const;
      bool           IsHunting( void ) const;
      bool           IsRising( void ) const;
      virtual void   LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;


   private:
   // Handles (implementing by pointers allows loading nullptr into handles not used by ctor invoked)
   // $$$ TBD whether these can be const

      CChartShewhart* const            p_ObsvdShew;
      CChartShewhart* const            p_GuideShew;

  // Fields
      float       halfBand;
      float       lagFrac;
      float       register_N;
      float       register_P;
      float       staleFrac;
      float       warn;   //"warn" = (units-of-x)-minutes (an "area") accumulated within a reset period
      float       xObsvdNow;
      float       xGuideNow;
      int         appsSinceReset;   // an "app" is one call of the ApplyChart() method
      int         appsBtwnResets;
      const bool  autoregressive; // "true" = xRef is mean of x, "false" = xRef is a setpoint for x) 
      bool        isFallingNow;
      bool        isHuntingNow;
      bool        isRisingNow;
      bool        wasFallingOnLastValid;
      bool        wasHuntingOnLastValid;
      bool        wasRisingOnLastValid;
      const bool  parametric; // "true" = xRef from ASubject object, "false" = it is acquired data
      bool        trackerOn;
 
   // Methods

      void              ConfigureCycling( void );
      void              AttachOwnKnobs(   CController&,
                                          std::array<float,3>,
                                          std::array<float,3> );
      void              PullValidity( void );
      void              PullTrackability( void );
      void              PullInput( void );
      void              ApplyChart( void );

      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      Cycle( time_t ) override;
      virtual void      LendAntecedentKnobKeysTo( std::vector<NGuiKey>& ) const override;

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Typedef pointers to getter methods of specific classes declared above

typedef bool ( CChartShewhart::*PtrShewGetr_t ) ( void ) const;
typedef bool ( CChartTracking::*PtrTrakGetr_t ) ( void ) const;

#endif

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   ("sayers" return specified field as a string, "getters"/"setters" access the field value)
      Parameter getters & setters pass params of ALL types out & in as floats.



--------------------------------------------------------------------------------
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
