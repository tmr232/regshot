@ECHO OFF
rem ******************************************************************************
rem *  Copyright 2010-2011 XhmikosR
rem *
rem *  This file is part of Regshot.
rem *
rem *  Regshot is free software; you can redistribute it and/or modify
rem *  it under the terms of the GNU General Public License as published by
rem *  the Free Software Foundation; either version 2 of the License, or
rem *  (at your option) any later version.
rem *
rem *  Regshot is distributed in the hope that it will be useful,
rem *  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem *  GNU General Public License for more details.
rem *
rem *  You should have received a copy of the GNU General Public License
rem *  along with Regshot; if not, write to the Free Software
rem *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
rem *
rem *
rem *  build_wdk.bat
rem *    Batch file "wrapper" for makefile.mak, used to build Regshot with WDK
rem ******************************************************************************


SETLOCAL
CD /D %~dp0

rem Set the WDK directory
SET "WDKBASEDIR=C:\WinDDK\7600.16385.1"

rem Check the building environment
IF NOT EXIST "%WDKBASEDIR%" CALL :SUBMSG "ERROR" "Specify your WDK directory!"

rem check for the help switches
IF /I "%1"=="help"   GOTO SHOWHELP
IF /I "%1"=="/help"  GOTO SHOWHELP
IF /I "%1"=="-help"  GOTO SHOWHELP
IF /I "%1"=="--help" GOTO SHOWHELP
IF /I "%1"=="/?"     GOTO SHOWHELP
GOTO CHECKFIRSTARG


:SHOWHELP
TITLE "%~nx0 %1"
ECHO.
ECHO Usage:  %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|all]
ECHO.
ECHO Note:   You can also prefix the commands with "-", "--" or "/".
ECHO.
ECHO.
ECHO Edit "%~nx0" and set your WDK directory.
ECHO You shouldn't need to make any changes other than that.
ECHO.
ECHO.
ECHO Executing "%~nx0" will use the defaults: "%~nx0 build all"
ECHO If you skip the second argument the default one will be used. Example:
ECHO "%~nx0 rebuild" is equivalent to "%~nx0 rebuild all"
ECHO NOTE: "%~nx0 x86" won't work.
ECHO.
ENDLOCAL
EXIT /B


:CHECKFIRSTARG
rem Check for the first switch
IF "%~1" == "" (
  SET "BUILDTYPE=Build"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:CHECKSECONDARG
rem Check for the second switch
IF "%~2" == "" (
  SET "ARCH=all"
) ELSE (
  IF /I "%~2" == "x86"   SET "ARCH=x86" & GOTO x86
  IF /I "%~2" == "/x86"  SET "ARCH=x86" & GOTO x86
  IF /I "%~2" == "-x86"  SET "ARCH=x86" & GOTO x86
  IF /I "%~2" == "--x86" SET "ARCH=x86" & GOTO x86
  IF /I "%~2" == "x64"   SET "ARCH=x64" & GOTO x86
  IF /I "%~2" == "/x64"  SET "ARCH=x64" & GOTO x86
  IF /I "%~2" == "-x64"  SET "ARCH=x64" & GOTO x86
  IF /I "%~2" == "--x64" SET "ARCH=x64" & GOTO x86
  IF /I "%~2" == "all"   SET "ARCH=all" & GOTO x86
  IF /I "%~2" == "/all"  SET "ARCH=all" & GOTO x86
  IF /I "%~2" == "-all"  SET "ARCH=all" & GOTO x86
  IF /I "%~2" == "--all" SET "ARCH=all" & GOTO x86

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:x86
SET "INCLUDE=%WDKBASEDIR%\inc\crt;%WDKBASEDIR%\inc\api;%WDKBASEDIR%\inc\api\crt\stl60;%WDKBASEDIR%\inc\ddk"
SET "LIB=%WDKBASEDIR%\lib\crt\i386;%WDKBASEDIR%\lib\win7\i386"
SET "PATH=%WDKBASEDIR%\bin\x86;%WDKBASEDIR%\bin\x86\x86"

IF /I "%ARCH%" == "x64" GOTO x64

TITLE Building Regshot x86...
ECHO. & ECHO.

IF /I "%BUILDTYPE%" == "Build" (
  CALL :SUBNMAKE

  IF /I "%ARCH%" == "x86" GOTO END
  IF /I "%ARCH%" == "x64" GOTO x64
  IF /I "%ARCH%" == "all" GOTO x64
)

IF /I "%BUILDTYPE%" == "Rebuild" (
  CALL :SUBNMAKE

  IF /I "%ARCH%" == "x86" GOTO END
  IF /I "%ARCH%" == "x64" GOTO x64
  IF /I "%ARCH%" == "all" GOTO x64
)

IF /I "%BUILDTYPE%" == "Clean" CALL :SUBNMAKE clean
IF /I "%ARCH%" == "x86" GOTO END
IF /I "%ARCH%" == "x64" GOTO x64
IF /I "%ARCH%" == "all" GOTO x64


:x64
SET "LIB=%WDKBASEDIR%\lib\crt\amd64;%WDKBASEDIR%\lib\win7\amd64"
SET "PATH=%WDKBASEDIR%\bin\x86;%WDKBASEDIR%\bin\x86\amd64"

IF /I "%ARCH%" == "x86" GOTO END

TITLE Building Regshot x64...
ECHO. & ECHO.

IF /I "%BUILDTYPE%" == "Build" (
  CALL :SUBNMAKE "x64=1"
  GOTO END
)

IF /I "%BUILDTYPE%" == "Rebuild" (
  CALL :SUBNMAKE "x64=1"
  GOTO END
)

IF /I "%BUILDTYPE%" == "Clean" CALL :SUBNMAKE "x64=1" clean


:END
TITLE Building Regshot - Finished!
ENDLOCAL
EXIT /B


:SUBNMAKE
nmake /NOLOGO /f "makefile.mak" %BUILDTYPE% %1
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"
EXIT /B


:SUBMSG
ECHO. & ECHO ______________________________
ECHO [%~1] %~2
ECHO ______________________________ & ECHO.
IF /I "%~1"=="ERROR" (
  PAUSE
  EXIT
) ELSE (
  EXIT /B
)
