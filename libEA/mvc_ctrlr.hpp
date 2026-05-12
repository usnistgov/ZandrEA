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
   Declares the concrete class CController for an EA Application's  MVC Controller object. 
   A CController object ultimately implements the "Controller" methods of the EA runtime API.
   Since the API are all pure virtual methods, the implementation first goes through a CPortOmni object.
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifndef MVC_CTRLR_HPP
#define MVC_CTRLR_HPP

#include "customTypes.hpp"

// Forward declares (to avoid unnecessary #includes)
class ADataChannel;
class AKnob;
class ASubject;

class CClockPerPort;
class CCaseKit;
class CDomain;
class CView;

typedef std::unordered_map<NGuiKey, AKnob* const>     KnobPtrTable_t; // non-const due to setters
typedef std::unordered_map<NGuiKey, ASubject* const>  SubjPtrTable_t; // non-const due to setters
typedef std::unordered_map< NGuiKey, std::vector<EPointName> >       SubjPointNameTable_t;
typedef std::unordered_map< NGuiKey, std::vector<ADataChannel*> >    SubjPointObjectTable_t;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////

class CController { 

   public:

   // Methods
      explicit CController( CClockPerPort& );   // c-tor of Controller updating Agent via Clock "bells"

      ~CController( void );

      EGuiReply                        PrepareApplicationForShutdown( void );

      EGuiReply                        SetTimeStampInDomain( std::tm );

      std::vector<EPointName>          SayInputPointNameOrderExpectedBySubject( NGuiKey ) const;

      EGuiReply                        ReadInDataForSubject(   const std::vector<GuiFpn_t>&,
                                                               NGuiKey );

      EGuiReply                        SingleStepModelOnTimeAndInputs( void );

      GuiPackKnob_t                    GetGuiPackFromKnob( NGuiKey ) const;
      EGuiReply                        SetKnobToValue( NGuiKey, GuiFpn_t );
      std::string                      SayTextIdentifyingKnob( NGuiKey ) const;

//vvvvvvv1vvvvvvvvv2vvvvvvvvv3vvvvvvvvv4vvvvvvvvv5vvvvvvvvv6vvvvvvvvv7vvvvvvvvv8vvvvvvvvv9vvvvvvvvvCvvvvv

      void                             Register( CView* const ); 
      void                             Register( AKnob* const );
      void                             RegisterBasPointToSubjectKey( ADataChannel*, NGuiKey );

      void                             DeregisterKnob( const NGuiKey );

   private:

   // Handles
      CClockPerPort&                         ClockRef;
      CView*                                 p_View;

      KnobPtrTable_t                         p_Knobs_byKey;
      SubjPointNameTable_t                   pointNamesZeroToN_bySubjKey;
      SubjPointObjectTable_t                 pointObjectsZeroToN_bySubjKey;
 };   

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
