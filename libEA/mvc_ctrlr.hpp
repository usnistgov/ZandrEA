// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares the concrete class CController for an EA Application's  MVC Controller object. 
   A CController object ultimately implements the "Controller" methods of the EA runtime API.
   Since the API are all pure virtual methods, the implementation first goes through a CPortOmni object. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

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
