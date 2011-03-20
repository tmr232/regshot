@ECHO OFF
rem ******************************************************************************
rem *
rem * Regshot
rem *
rem * make_packages.bat
rem *   Batch file for building Regshot with WDK and creating the zip packages
rem *
rem * See gpl.txt for details about distribution and modification.
rem *
rem *                                       (c) XhmikosR 2010-2011
rem *                                       http://code.google.com/p/regshot/
rem *
rem ******************************************************************************


SETLOCAL
CD /D %~dp0


CALL :SubGetVersion

CALL "build_wdk.cmd"

CALL :SubZipFiles Release_x86 x86
CALL :SubZipFiles Release_x64 x64


:END
TITLE Finished!
ECHO.
ENDLOCAL
PAUSE
EXIT /B


:SubZipFiles
TITLE Creating the %2 ZIP file...
CALL :SUBMSG "INFO" "Creating the %2 ZIP file..."

MD "temp_zip" >NUL 2>&1
COPY /Y /V "..\gpl.txt" "temp_zip\GPL.txt"
COPY /Y /V "..\files\history.txt" "temp_zip\History.txt"
COPY /Y /V "..\files\language.ini" "temp_zip\"
COPY /Y /V "..\files\readme.txt" "temp_zip\Readme.txt"
COPY /Y /V "..\files\regshot.ini" "temp_zip\"
COPY /Y /V "..\bin\WDK\%1\Regshot.exe" "temp_zip\"

PUSHD "temp_zip"
START "" /B /WAIT "..\7za.exe" a -tzip -mx=9 "Regshot_%REGSHOTVER%_%2_WDK.zip" >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

CALL :SUBMSG "INFO" "Regshot_%REGSHOTVER%_%2_WDK.zip created successfully!"

MOVE /Y "Regshot_%REGSHOTVER%_%2_WDK.zip" "..\" >NUL 2>&1
POPD
RD /S /Q "temp_zip" >NUL 2>&1
EXIT /B


:SubGetVersion
rem Get the version
FOR /F "tokens=3,4 delims= " %%K IN (
  'FINDSTR /I /L /C:"define REGSHOT_VERSION_MAJOR" "..\src\version.h"') DO (
  SET "VerMajor=%%K"&Call :SubVerMajor %%VerMajor:*Z=%%)
FOR /F "tokens=3,4 delims= " %%L IN (
  'FINDSTR /I /L /C:"define REGSHOT_VERSION_MINOR" "..\src\version.h"') DO (
  SET "VerMinor=%%L"&Call :SubVerMinor %%VerMinor:*Z=%%)
FOR /F "tokens=3,4 delims= " %%M IN (
  'FINDSTR /I /L /C:"define REGSHOT_VERSION_PATCH" "..\src\version.h"') DO (
  SET "VerBuild=%%M"&Call :SubVerBuild %%VerBuild:*Z=%%)
FOR /F "tokens=3,4 delims= " %%N IN (
  'FINDSTR /I /L /C:"define REGSHOT_VERSION_REV" "..\src\version.h"') DO (
  SET "VerRev=%%N"&Call :SubVerRev %%VerRev:*Z=%%)

SET REGSHOTVER=%VerMajor%.%VerMinor%.%VerBuild%
EXIT /B


:SubVerMajor
SET VerMajor=%*
EXIT /B


:SubVerMinor
SET VerMinor=%*
EXIT /B


:SubVerBuild
SET VerBuild=%*
EXIT /B


:SubVerRev
SET VerRev=%*
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
