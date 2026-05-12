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
   Header both declaring and implementing functor classes supporting the fact and rule classes. 
   There is no "stateParts.cpp", class was templated in an earlier version.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef STATEPARTS_HPP
#define STATEPARTS_HPP

#include "customTypes.hpp"

#include <string>
#include <algorithm>
#include <cmath>

#define NOMINMAX  // Per S.O., any hidden include of windows.h will interfere with std::min/std::max (?)

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract Base Class ("ABC") for all functors called by relations embedded in fact objects.

class ARelnFunctor { 

   public:
      virtual           ~ARelnFunctor( void ) { /* empty*/ };

      virtual bool      operator() ( float, float ) { return NaNBOOL; } // See Class Note [1]

      const float&      SayHysterCref( void ) const { return hyster; }  // Used primarily by Knobs
      const float&      SaySlackCref( void ) const { return slack; }  // Used primarily by Knobs
      void              SetHyster( float arg0 ) { hyster = arg0; }
      void              SetSlack( float arg0 ) { slack = arg0; }

      float             SaySpillage( void ) const { return spillage; }

      char              SaySymbol( void ) const { return symbol; }

   protected:

      ARelnFunctor(  float arg0,
                     float arg1,
                     char arg2 )
                     :  hyster (arg0),
                        slack (arg1),
                        spillage (0.0f),
                        symbol (arg2),
                        resultNow (NaNBOOL),
                        resultWas (false) {
      }

      float       hyster;          // hysteresis
      float       slack;
      float       spillage;
      const char  symbol;
      bool        resultNow;
      bool        resultWas;

/* Begin Class Notes '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   Operator() overload is NOP at ABC level.  Taking cheap-to-copy arguments as PBV, but other
      implementations, having more exotic args, may warrant PBR.

'' End Class Notes ''' */  

};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Subclasses of relation functors.  All implementing code is included with these declarations

class Left_LT_Right : public ARelnFunctor {

   public:
         
      Left_LT_Right( float bArg0,
                     float bArg1 )
                     :  ARelnFunctor( bArg0, bArg1, '<' ) {
      }


      ~Left_LT_Right( void ) { }


      virtual bool operator() ( float left, float right ) override {
 

         resultNow = ( left < ( right + slack + ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         spillage = (std::max)( 0.0f, (left - right) );
         resultWas = resultNow; // hyster requires placing this AFTER first computation of resultNow

         return resultNow;
      }
};


class Left_GT_Right : public ARelnFunctor {

   public:
         
      Left_GT_Right( float bArg0,
                     float bArg1 )
                     :  ARelnFunctor(bArg0, bArg1,'>' ) {
      }


      ~Left_GT_Right( void ) { }


      virtual bool operator() ( float left, float right ) override {

         resultNow = ( left > ( right - slack - ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         spillage = (std::max)( 0.0f, (right - left) );
         resultWas = resultNow;

         return resultNow;
      }
};


class Left_EQ_Right : public ARelnFunctor {

   public:
         
      explicit Left_EQ_Right( float bArg0,
                              float bArg1 )
                              : ARelnFunctor( bArg0, bArg1, '=' ) {
      }


      ~Left_EQ_Right( void ) { }


      virtual bool operator() ( float left, float right ) override {


         bool aboveFloor =    ( left >= ( right - slack - ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         bool belowCeiling =  ( left <= ( right + slack + ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         resultNow = ( aboveFloor && belowCeiling );

         if (resultNow) { spillage = 0.0f; }
         else { spillage = std::fabs(right - left); }
         resultWas = resultNow;

         return resultNow;
      }
};


class Left_GTE_Right : public ARelnFunctor {

   public:
         
      explicit Left_GTE_Right(   float bArg0,
                                 float bArg1 )
                                 :  ARelnFunctor(bArg0, bArg1, '>' ) {
      }


      ~Left_GTE_Right( void ) { }


      virtual bool operator() ( float left, float right ) override {

         resultNow = ( left >= ( right - slack - ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         spillage = (std::max)( 0.0f, (right - left) );
         resultWas = resultNow;

         return resultNow;
      }
};


class Left_LTE_Right : public ARelnFunctor {

   public:
         
      explicit Left_LTE_Right(   float bArg0,
                                 float bArg1 )
                              :  ARelnFunctor(bArg0, bArg1, '<' ) {
      }


      ~Left_LTE_Right( void ) { }


      virtual bool operator() ( float left, float right ) override {
 
         resultNow = ( left <= ( right + slack + ( hyster * (resultWas ? 1.0f : 0.0f) ) ) );
         spillage = (std::max)( 0.0f, (left - right) );
         resultWas = resultNow;

         return resultNow;
      }
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
