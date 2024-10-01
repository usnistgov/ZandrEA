// libmain.cpp : Defines the entry point for the DLL application.

//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX


#include "tool.hpp"

// 
// LibMain()
//
// This function is a vestigial remnant of a time when this code was originally written as a Windows DLL.
//
// Its purpose is to instantiate and initialize some dynamic resources at library load time.
//
// See also the code in ../EAd/main.cpp that references this function (forcing the linker to link it in to the executable)
// 
void __attribute__((constructor)) LibMain(void) {

   CApplication* p_eaAppInstance = new CApplication();
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
