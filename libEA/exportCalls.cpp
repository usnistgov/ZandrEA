// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Declare "Runtime API" between EA front end (host clock, data acquisition, GUI) and EA back end
   (AFDD engine). The EA front end calls these API methods, the EA back end implements them.    
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#include "customTypes.hpp"
#include "exportCalls.hpp"


IExportOmni* SExportedHandles::p_Port = nullptr;

IExportOmni* SExportedHandles::GetPortPointer( void ) { return p_Port; }

void SExportedHandles::SetPortPointer( IExportOmni* arg ) { p_Port = arg; }


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
