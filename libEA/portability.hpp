//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* Source code file to an "EA" part of the ZandrEA (tm) project at: https://github.com/usnistgov/ZandrEA
This file last edited in base repo by: SWB, U.S. National Institute of Standards and Technology (NIST).
As a Work of the United States Government, this file is not subject to copyright within the United
States. For other countries, Copyright 2025-2026 National Institute of Standards and Technology.
For countries other than the United States, this file is licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy
of the License at: https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License. */
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXV
/* File summary:
  Definitions used to solve common porting issues go here
  steveb@nist.gov 8/3/2017
*/
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8/////////9/////////C////V

#ifdef _MSC_VER

#else

// Microsoft Visual Studio was the original build platform so things that
// required workarounds outside of that environment are put here.

#include <ctime>
#include <cerrno>
#include <errno.h>
#include <cstring>
//# include <string.h>

//# ifdef __linux
typedef int errno_t;
typedef size_t rsize_t;
//# endif

inline errno_t ctime_s(
   char* buffer,
   size_t numberOfElements,
   const time_t *time
) {
  // This is slightly different than MSVS's ctime_s since I believe that will
  // write a partial result string into the buffer if it's too short.  This
  // one will just abort.
  if (buffer && numberOfElements >= 26) {
    strncpy(buffer, ctime(time), 26);
    return(0);
  } else {
    return(EINVAL);
  }
}

inline errno_t strncat_s(char *dest, rsize_t destsz,
                  const char *src, rsize_t count) {
  // This seems to be another MSVS specific function
  if (count >= destsz) {
    count = destsz - 1;
  }
  if (dest && src && count > 0 && destsz > 0) {
    strncpy(dest, src, count);
    dest[destsz - 1] = '\0';
    return(0);
  } else {
    if (dest && destsz > 0) {
      dest[0] = '\0';
    }
    return(EINVAL);
  }
}

inline errno_t localtime_s(std::tm *tm, const std::time_t *t) {
  // Another MSVS specific function
  if (t && tm) {
    std::tm *buf = localtime(t);
    if (buf) {
      *tm = *buf; // copy out of static buffer to provided buffer
      return(0);
    }
  }
  return(EINVAL);
}
#endif
