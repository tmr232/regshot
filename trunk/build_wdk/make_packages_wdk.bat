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
rem *                                       (c) XhmikosR 2010
rem *                                       http://code.google.com/p/regshot/
rem *
rem ******************************************************************************

SETLOCAL
CD /D %~dp0

SET REGSHOTVER=1.8.3

CALL "build_wdk.cmd"

CALL :SubZipFiles Release x86
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
COPY /Y /V "..\history.txt" "temp_zip\History.txt"
COPY /Y /V "..\language.ini" "temp_zip\"
COPY /Y /V "..\readme.txt" "temp_zip\Readme.txt"
COPY /Y /V "..\regshot.ini" "temp_zip\"
COPY /Y /V "..\%1\Regshot.exe" "temp_zip\"

PUSHD "temp_zip"
START "" /B /WAIT "..\7za.exe" a -tzip -mx=9 "Regshot_%REGSHOTVER%_%2.zip" >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

CALL :SUBMSG "INFO" "Regshot_%REGSHOTVER%_%2.zip created successfully!"

MOVE /Y "Regshot_%REGSHOTVER%_%2.zip" "..\" >NUL 2>&1
POPD
RD /S /Q "temp_zip" >NUL 2>&1
EXIT /B


:SUBMSG
ECHO.&&ECHO:______________________________
ECHO:[%~1] %~2
ECHO:______________________________&&ECHO.
IF /I "%~1"=="ERROR" (
  PAUSE
  EXIT
) ELSE (
  EXIT /B
)
