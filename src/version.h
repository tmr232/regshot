/*
    Copyright 2011 XhmikosR

    This file is part of Regshot.

    Regshot is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Regshot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Regshot; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef REGSHOT_VERSION_H
#define REGSHOT_VERSION_H


// Version numbers
#define DO_STRINGIFY(x) #x
#define STRINGIFY(x) DO_STRINGIFY(x)

#define REGSHOT_VERSION_MAJOR    1
#define REGSHOT_VERSION_MINOR    8
#define REGSHOT_VERSION_PATCH    3
#define REGSHOT_VERSION_REV      0
#define REGSHOT_VERSION_NUM      REGSHOT_VERSION_MAJOR,REGSHOT_VERSION_MINOR,REGSHOT_VERSION_PATCH,REGSHOT_VERSION_REV
#define REGSHOT_VERSION          STRINGIFY(REGSHOT_VERSION_MAJOR) ", " STRINGIFY(REGSHOT_VERSION_MINOR) ", " STRINGIFY(REGSHOT_VERSION_PATCH) ", " STRINGIFY(REGSHOT_VERSION_REV)
#define REGSHOT_VERSION_STRING   STRINGIFY(REGSHOT_VERSION_MAJOR) "." STRINGIFY(REGSHOT_VERSION_MINOR) "." STRINGIFY(REGSHOT_VERSION_PATCH)
#ifdef _WIN64
#define REGSHOT_FILEDESCRIPTION  "Regshot x64"
#else
#define REGSHOT_FILEDESCRIPTION  "Regshot"
#endif

#ifdef _WIN64
    #define PLATFORM_SUFFIX " x64"
#else
  #define PLATFORM_SUFFIX   ""
#endif	// _WIN64


#endif

