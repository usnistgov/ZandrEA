// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare abstract base and concrete classes for objects receiving sampled data (e.g., "points")
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef DATACHANNEL_HPP
#define DATACHANNEL_HPP

#include "seqElement.hpp"            // Inheiritance requires type completion, brings F.D. of CSequence
#include <vector>
#include <string>

class CController;
class CRainAnalog;
class CFactFromPoint;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract class covering all channels of data input ("points") from Port/GUI/Client

class ADataChannel : public ISeqElement {

   public:

      ADataChannel(  CSequence&, 
                     ASubject&,
                     EApiType,
                     EDataLabel, 
                     EDataUnit,
                     EDataRange,
                     EDataSuffix,
                     EPlotGroup,
                     int,
                     EPointName );

      ~ADataChannel( void );

      // Methods
      EPointName           SayPointName( void ) const;
      void                 ReadFromPortAsNextValue( GuiFpn_t );

   protected:
   // Fields

      double                  xGivenDbl;     // All data arrives as doubles, including binary data 
      double                  xPrevDbl;
      const EPointName        pointName; 

   // Methods
            
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete "point" class for analog (proportional to physical value) data input from Port/GUI/Client

class CPointAnalog : public ADataChannel {

   public:

      CPointAnalog(  CSequence&, 
                     ASubject&,
                     EDataLabel, 
                     EDataUnit,
                     EDataRange,
                     EPlotGroup,
                     EPointName,
                     CController& );    // point registers to Ctrlr (non-const) using ID of its Subject

      ~CPointAnalog( void );

      //CPointAnalog( const CPointAnalog& ) = delete;

      CPointAnalog& operator=( const CPointAnalog& ) { return *this; }

      virtual void      LendHistogramKeysTo( std::vector<NGuiKey>& ) const override;
      virtual void      LendRealtimeAnalogAccessTo( RtTraceAccessTable_t& ) const override;

   // Handles
      const std::unique_ptr<CRainAnalog>     u_Rain;

   private:

   // Handles
      const std::unique_ptr<CTraceRealtime>  u_RealtimeTrace;
 
   // Fields
      float                   xPosted;   
      float                   xValidMin;
      float                   xValidMax; 
      float                   xLastValid;
      bool                    sameDblAsPrev;    // $$$ TBD to exploit.  N/A for CPointBinary (obviously)

   // Methods
      virtual void         Cycle( time_t ) override;
            
};


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Concrete "point" class for binary ( off/on, low/high, false/true, 0.0/1.0 ) info from Port/GUI/Client

class CPointBinary : public ADataChannel {

   public:

      CPointBinary(  CSequence&, 
                     ASubject&,
                     EDataLabel,       // label for CPointBinary object
                     EDataLabel,       // label for linked CStateFromPoint object 
                     EPointName,
                     CController& );    // point registers to Ctrlr (non-const) using ID of its Subject

      ~CPointBinary( void );

      //CPointAnalog( const CPointAnalog& ) = delete;

      CPointBinary& operator=( const CPointBinary& ) { return *this; }

   // Methods, public
      bool               SayBinaryPosted( void ) const;

   // Handles
      const std::unique_ptr<CFactFromPoint>  u_Fact;   // Owned object holding rainfall object, etc.

   private:
   // Fields

      bool              binaryPosted;   

   // Methods

      virtual void      CalcOwnTriggerGroup( void ) override;
      virtual void      Cycle( time_t ) override;
            
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
