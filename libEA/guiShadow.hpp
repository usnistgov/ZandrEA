// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declares an abstract "integrating" class that "shadows" front-end GUI elements in the back end (the
   GUI's information source).  Concrete classes specific to each type of GUI element are derived from it. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef GUISHADOW_HPP
#define GUISHADOW_HPP

#include "customTypes.hpp"
#include <sstream>

class ASubject;

 // typedefs providing lookup tables for all API -> GUI or API -> disk file translations:

typedef std::unordered_map< EAlertMsg, std::string >                             AlertMsgTable_t;
typedef std::unordered_map< EDataRange, std::array<float,2> >                    DataRangeTable_t;
typedef std::unordered_map< EDataSuffix, std::string >                           DataSuffixTable_t;
typedef std::unordered_map< EDataLabel, std::string >                            DataTagTable_t;
typedef std::unordered_map< EDataUnit, std::string >                             DataUnitTable_t;
typedef std::unordered_map< ERealName, std::string >                             DiskFileTable_t;
typedef std::unordered_map< EApiType, EGuiType >                                 GuiTypeTable_t;
typedef std::unordered_map< EInfoMode, std::string >                             InfoModeTable_t;
typedef std::unordered_map< ERealName, std::string >                             RealNameTable_t;
typedef std::unordered_map< ESubjParam, std::string >                            SubjParamTable_t;
typedef std::unordered_map< ETimeSpan, std::string >                             TimeSpanTable_t;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
//  Interface class for all classes whose objects 'shadow' objects the Front End has in its GUI

class IGuiShadow {

   public:
   // Methods

      virtual ~IGuiShadow( void );

      NGuiKey                             SayGuiKey( void ) const;
      EApiType                            SayApiType( void ) const;

      // "tag" is abbreviated "text" to be used by plots, etc.
      static std::string                  LookUpDiskFile( ERealName );
      static EGuiType                     LookUpGuiType( EApiType );
      static std::string                  LookUpTag( EDataLabel );
      static std::string                  LookUpText( EAlertMsg );
      static std::array<float,2>          LookUpMinMax( EDataRange );
      static std::string                  LookUpText( EDataSuffix );
      static std::string                  LookUpText( EDataUnit );
      static std::string                  LookUpText( EInfoMode );
      static std::string                  LookUpText( ETimeSpan );
      static std::string                  LookUpText( ERealName );

   protected:
   // Fields

      static unsigned long long           nextFreshKeySeedValue;
      const NGuiKey                       ownGuiKey;
      const EApiType                      ownApiType; // "API" so can use broader internal set of types

#ifdef SWBSWBSWB
      static std::ostringstream           intStreamer;
      static bool                         streamerConfigured;
#endif

   // Methods

   explicit IGuiShadow( EApiType );
   static NGuiKey                         GenerateNewGuiKey( void );

   static AlertMsgTable_t                 InitLookupTable_AlertMsg( void );
   static DiskFileTable_t                 InitLookupTable_DiskFile( void );
   static DataRangeTable_t                InitLookupTable_DataRange( void );
   static DataSuffixTable_t               InitLookupTable_DataSuffix( void );
   static DataTagTable_t                  InitLookupTable_DataTag( void );
   static DataUnitTable_t                 InitLookupTable_DataUnit( void );
   static GuiTypeTable_t                  InitLookupTable_GuiType( void );
   static InfoModeTable_t                 InitLookupTable_InfoMode( void );
   static TimeSpanTable_t                 InitLookupTable_TimeSpan( void );
   static RealNameTable_t                 InitLookupTable_RealName( void );

   static std::string                     SayIdentifierAsThreeDigitText( Nzint_t );
   static void                            WriteTimestampAsTextTo( time_t, std::string& );

   std::string                            RenderGuiTag(  EDataLabel,
                                                         EDataSuffix = EDataSuffix::Undefined ) const;

   std::string                            RenderUnits( EDataUnit ) const;


};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
