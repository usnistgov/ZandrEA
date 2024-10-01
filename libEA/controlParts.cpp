// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Implement abstract base and concrete classes for objects (parts) accessed via Controller object
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "controlParts.hpp"

#include "mvc_ctrlr.hpp"
#include "seqElement.hpp"
#include <algorithm>

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// AKnob implementation

AKnob::AKnob(  EApiType bArg,
               CController& arg0,
               const ISeqElement& arg1,              // Only needed for SayName() call
               EDataLabel arg2,                      // Knob sub-label
               EDataUnit arg3,
               EDataSuffix arg4, 
               std::vector<GuiFpn_t> arg5,
               GuiFpn_t arg6 )
               :  IGuiShadow( bArg ),
                  CtrlrRef (arg0),
                  HostRef (arg1),
                  fieldLabel (arg2),
                  fieldUnits (arg3),
                  unitsSuffix (arg4),
                  rangeMinMaxToGui (arg5),
                  valuesSelectable_ifDefined(0),
                  valueNowToGui (arg6)  {

   // Empty base c-tor
}


AKnob::~AKnob( void ) {  }


GuiPackKnob_t   AKnob::GetGuiPack( void ) const {

    return SGuiPackKnob( LookUpGuiType( ownApiType),
                        ownGuiKey,
                        ( LookUpTag( HostRef.SayLabel() ) + " : " + LookUpTag( fieldLabel ) ),
                        ( LookUpText( fieldUnits ) + LookUpText( unitsSuffix ) ),
                        rangeMinMaxToGui,
                        valuesSelectable_ifDefined,
                        valueNowToGui
   );

/* Method Notes:   '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''/

[1]   If GUI not providing context for tags, revise loading of tag to:
      (  RenderGuiTag( HostRef.SayLabel(),  HostRef.SaySuffix() ) +
                      ":" + LookUpTag( fieldLabel )
      ),
*/

}


std::string AKnob::SayIdentifyingText( void ) const {

   return ( LookUpTag( fieldLabel ) );
}


void AKnob::DefineValuesSelectable( std::vector<Nzint_t> arg ) {

   if ( !arg.empty() && ownApiType == EApiType::Knob_selectNzint ) {

      // avoid seg fault from trying to copy into empty container
      valuesSelectable_ifDefined.assign(arg.size(), 0.0);

      // implicit cast of vector elements from Nzint_t to GuiFpn_t
      std::copy( arg.begin(), arg.end(), valuesSelectable_ifDefined.begin() );
   }  
   return;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CKnobFloat concrete subclass implementation

CKnobFloat::CKnobFloat( CController& bArg0,
                        const ISeqElement& bArg1,
                        EDataLabel bArg2,
                        EDataUnit bArg3,
                        EDataSuffix bArg4,
                        std::function< void (float) > arg0,
                        std::array<float,3> arg1,
                        const float& arg3 )
                        :  AKnob(   EApiType::Knob_float,
                                    bArg0,
                                    bArg1,
                                    bArg2,
                                    bArg3,
                                    bArg4,
                                    std::vector<GuiFpn_t>({ arg1[0], arg1[2] }),
                                    GuiFpn_t(arg3)
                           ),
                           FieldSetter (arg0),
                           fieldRef (arg3) {

   if (  rangeMinMaxToGui[0] > rangeMinMaxToGui[1]  ) {
      throw std::logic_error( "Knob c-tor given invalid range" );
   }

   CtrlrRef.Register( this );
}


CKnobFloat::~CKnobFloat( void ) { CtrlrRef.DeregisterKnob( ownGuiKey ); }


EGuiReply CKnobFloat::SetValueTo( GuiFpn_t valueGiven ) {

   if (  (valueGiven < rangeMinMaxToGui[0]) ||
         (valueGiven > rangeMinMaxToGui[1]) ) {               // The "vett": TRUE rejects valueGiven
      return EGuiReply::FAIL_set_givenValueOutOfRangeAllowed;
   }
   FieldSetter( static_cast<float>(valueGiven) );
   valueNowToGui = GuiFpn_t( fieldRef );
   return EGuiReply::OKAY_allDone;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CKnobInt concrete subclass implementation

CKnobSint::CKnobSint(   CController& bArg0,        // also useful for u-int inputs, given min = 0
                        const ISeqElement& bArg1,
                        EDataLabel bArg2,
                        EDataUnit bArg3,
                        EDataSuffix bArg4,
                        std::function< void(int) > arg0,
                        std::array<int,3> arg1,
                        const int& arg3 )
                        :  AKnob(   EApiType::Knob_sint,
                                    bArg0,
                                    bArg1,
                                    bArg2,
                                    bArg3,
                                    bArg4,
                                    std::vector<GuiFpn_t>( {static_cast<GuiFpn_t>(arg1[0]),
                                                            static_cast<GuiFpn_t>(arg1[2]) }
                                    ), // C++11 cannot implicily "narrow" using an initializer list
                                    GuiFpn_t(arg3)
                           ),
                           FieldSetter (arg0),
                           fieldRef (arg3) {

   if (  rangeMinMaxToGui[0] > rangeMinMaxToGui[1]  ) {
      throw std::logic_error( "Knob c-tor given invalid range" );
   }

   CtrlrRef.Register( this );
}


CKnobSint::~CKnobSint( void ) { CtrlrRef.DeregisterKnob( ownGuiKey ); }


EGuiReply CKnobSint::SetValueTo( GuiFpn_t valueGiven ) {

   if (  (valueGiven < rangeMinMaxToGui[0]) ||
         (valueGiven > rangeMinMaxToGui[1]) ) {               // The "vett": TRUE rejects valueGiven
      return EGuiReply::FAIL_set_givenValueOutOfRangeAllowed;
   }
   FieldSetter( static_cast<int>(valueGiven) );
   valueNowToGui = GuiFpn_t( fieldRef );
   return EGuiReply::OKAY_allDone;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CKnobBool concrete subclass implementation

CKnobBool::CKnobBool(   CController& bArg0,
                        const ISeqElement& bArg1,
                        EDataLabel bArg2,
                        EDataSuffix bArg3,
                        std::function<void(bool)> arg0,
                        const bool& arg1 )
                        :  AKnob(   EApiType::Knob_Boolean,
                                    bArg0,
                                    bArg1,
                                    bArg2,
                                    EDataUnit::Binary_Boolean,
                                    bArg3,
                                    std::vector<GuiFpn_t>(0),  // GUI to "know" empty means range n/a
                                    ( arg1 ? 1.0 : 0.0 )
                        ),
                        FieldSetter (arg0),
                        fieldRef (arg1) {

   CtrlrRef.Register( this );
}


CKnobBool::~CKnobBool( void ) { CtrlrRef.DeregisterKnob( ownGuiKey ); }


EGuiReply CKnobBool::SetValueTo( GuiFpn_t valueGiven ) {

   EGuiReply reply = EGuiReply::FAIL_set_givenValueOutOfRangeAllowed;
   bool didUserSayYES = (valueGiven == 1.0 );

   if ( (valueGiven == 0.0) || didUserSayYES ) {     // The "vett": TRUE accepts valueGiven

      FieldSetter( didUserSayYES );
      valueNowToGui = ( fieldRef ? 1.0 : 0.0 );
      reply = EGuiReply::OKAY_allDone;
   }
   return reply;
}


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// CKnobSelectNzint concrete subclass implementation

CKnobSelectNzint::CKnobSelectNzint( CController& bArg0,
                                    const ISeqElement& bArg1,
                                    EDataLabel bArg2,
                                    EDataUnit bArg3,
                                    std::function< EGuiReply (Nzint_t) > arg0,
                                    const Nzint_t& arg2 )
                                    :  AKnob(   EApiType::Knob_selectNzint,
                                                bArg0,
                                                bArg1,
                                                bArg2,
                                                bArg3,
                                                EDataSuffix::None,
                                                std::vector<GuiFpn_t>(0),
                                                static_cast<GuiFpn_t>(0)
                              ),
                              FieldSetter (arg0),
                              r_field (arg2) {

   CtrlrRef.Register( this );
}


CKnobSelectNzint::~CKnobSelectNzint( void ) { CtrlrRef.DeregisterKnob( ownGuiKey ); }


EGuiReply CKnobSelectNzint::SetValueTo( GuiFpn_t valueGiven ) {

   if (  valuesSelectable_ifDefined.empty() ||
         valuesSelectable_ifDefined.end() == std::find(  valuesSelectable_ifDefined.begin(),
                                                         valuesSelectable_ifDefined.end(),
                                                         valueGiven ) ) {
      return EGuiReply::FAIL_set_givenValueNotInSelectionSet;
   }

   FieldSetter( static_cast<Nzint_t>( valueGiven ) );
   valueNowToGui = GuiFpn_t( r_field );
   return EGuiReply::OKAY_allDone;
}


/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   Typ for protected fields related to cycling, initialized with default values that may be
      overwritten by ConfigureCycling() override call in c-tor of ISeqElement subclasses that can cycle
      at rates below the maximum (i.e., at less the rate the object is triggered to cycle).

      These fields cannot be const, as some ISeqElement subclasses must call ConfigureCycling() to
      calculate triggers/cycle from the particular object(s) from which they obtain data.

[2]   Side-effect of CycleFromTriggerNow() is to decrement counter to next cycle.
       
[3]   Default method is virtual so objects of any subclass taking inputs from other ISeqElement object(s)
      can implement overrides adding call(s) of the LendAllKnobsTo() methods aboard those other objects,
      giving the Feature or Trace "borrower" object all knobs relevant to what it sends to client's GUI.
-
XXX END FILE NOTES */

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
