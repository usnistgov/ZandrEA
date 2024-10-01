// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare abstract base and concrete classes for objects (parts) accessed via Controller object
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef CONTROLPARTS_HPP
#define CONTROLPARTS_HPP

#include "guiShadow.hpp"
#include <functional>

class CController;
class ISeqElement;

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C/////
// Abstract for "knobs" allowing external GUI to read/adjust private fields in ISeqElement objects.
// Not all ISeqElements have knobs.

class AKnob : public IGuiShadow {

   public:

      virtual ~AKnob( void );

      GuiPackKnob_t           GetGuiPack( void ) const;
      std::string             SayIdentifyingText( void ) const;
      void                    DefineValuesSelectable( std::vector<Nzint_t> );
      virtual EGuiReply       SetValueTo( GuiFpn_t ) = 0;


   protected:

   // Handles
      CController&            CtrlrRef;   // Needed by subclass d-tor to de-register knob (mortal instances)
      const ISeqElement&      HostRef;

      const EDataLabel              fieldLabel;             // label of field on the hosting object
      const EDataUnit               fieldUnits;             // units of field, not necessarily = host's
      const EDataSuffix             unitsSuffix;
      const std::vector<GuiFpn_t>   rangeMinMaxToGui;       // TBD to include sending "default" to GUI
      std::vector<GuiFpn_t>   valuesSelectable_ifDefined;   // non-const so subclass c-tor can define it
      GuiFpn_t                valueNowToGui;

      // Methods          
      AKnob(   EApiType,
               CController&,
               const ISeqElement&,
               EDataLabel, 
               EDataUnit,
               EDataSuffix,
               std::vector<GuiFpn_t>,
               GuiFpn_t );

/* CLASS NOTES '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

   [1]   Setter with no getter means the fields "knobbed" on an ISeqElement operand using this class can
         only be fields changed using this class (i.e., changeable ONLY by the knob instance).

   [2]   GUI is passed the range of allowed field set values by the following coded convention
         with GUI developer (to avoid GUI code needing test and branch on a separate return element):
         Knobbed field is float: range is vector<GuiFpn_t>( {min allowed, max allowed } )
         Knobbed field is int: range is vector<GuiFpn_t>( {min allowed, max allowed } )
         Knobbed field is bool: range is an empty vector
         Knobbed field is a selection: range is empty vector, vett is done by owning object
*/

};

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
//Individual concrete subclass for each type of knob (similar to how Traces and Panes are done)


class CKnobFloat : public AKnob {

   public:

      CKnobFloat( CController&,
                  const ISeqElement&,
                  EDataLabel,
                  EDataUnit,
                  EDataSuffix,
                  std::function< void (float) >,
                  std::array<float,3>,    // [min, default, max] from customTypes.hpp
                  const float& );   // ref to host's field, so knob MUST be built AFTER field allocated 

      ~CKnobFloat( void );

      virtual EGuiReply                         SetValueTo( GuiFpn_t ) override;

   private:

      const std::function< void ( float ) >     FieldSetter;
      const float&                              fieldRef;
};


//======================================================================================================/

class CKnobSint : public AKnob {

   public:

      CKnobSint(   CController&,
                  const ISeqElement&,
                  EDataLabel,
                  EDataUnit,
                  EDataSuffix,
                  std::function< void (int) >,
                  std::array<int,3>,    // [min, default, max] from customTypes.hpp
                  const int& );   // ref to host's field, so knob MUST be built AFTER field allocated

      ~CKnobSint( void );

      virtual EGuiReply                         SetValueTo( GuiFpn_t ) override;

   private:

      const std::function< void ( int ) >       FieldSetter;
      const int&                                fieldRef;
};


//======================================================================================================/

class CKnobBool : public AKnob {

   public:

      CKnobBool(  CController&,
                  const ISeqElement&,
                  EDataLabel,
                  EDataSuffix,
                  std::function<void (bool)>,
                  const bool& );    // ref to host's field, so knob MUST be built AFTER field allocated

      ~CKnobBool( void );

   virtual EGuiReply                         SetValueTo( GuiFpn_t ) override;

private:

   const std::function< void ( bool ) >      FieldSetter;
   const bool&                               fieldRef;
};


//======================================================================================================/

class CKnobSelectNzint : public AKnob {

   public:

      CKnobSelectNzint( CController&,
                        const ISeqElement&,
                        EDataLabel,
                        EDataUnit,
                        std::function< EGuiReply (Nzint_t) >,
                        const Nzint_t&
      ); 

      ~CKnobSelectNzint( void );

      virtual EGuiReply       SetValueTo( GuiFpn_t ) override;

   private:

      const std::function<EGuiReply( Nzint_t )>       FieldSetter; // Only field setter generating reply
      const Nzint_t&                                  r_field;
};

#endif

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
