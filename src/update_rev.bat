@ECHO OFF
SETLOCAL

PUSHD %~dp0%

SET "SUBWCREV=SubWCRev.exe"

"%SUBWCREV%" .. "version_in.h" "VersionRev.h" -f
IF %ERRORLEVEL% NEQ 0 GOTO NoSubWCRev

POPD
ENDLOCAL
EXIT /B


:NoSubWCRev
ECHO. & ECHO SubWCRev, which is part of TortoiseSVN, wasn't found!
ECHO You should (re)install TortoiseSVN.
ECHO I'll use VERSION_REV=0 for now.

ECHO #ifndef REGSHOT_VERSION_REV_H> "VersionRev.h"
ECHO #define REGSHOT_VERSION_REV_H>> "VersionRev.h"
ECHO.>> "VersionRev.h"
ECHO #define REGSHOT_VERSION_REV 0 >> "VersionRev.h"
ECHO.>> "VersionRev.h"
ECHO #endif // REGSHOT_VERSION_REV_H>> "VersionRev.h"

POPD
ENDLOCAL
EXIT /B
