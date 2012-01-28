@ECHO OFF

rem Copyright 2011-2012 Regshot Team
rem
rem This file is part of Regshot.
rem
rem Regshot is free software: you can redistribute it and/or modify
rem it under the terms of the GNU Lesser General Public License as published by
rem the Free Software Foundation, either version 2.1 of the License, or
rem (at your option) any later version.
rem
rem Regshot is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU Lesser General Public License for more details.
rem
rem You should have received a copy of the GNU Lesser General Public License
rem along with Regshot.  If not, see <http://www.gnu.org/licenses/>.

SETLOCAL

PUSHD %~dp0

SubWCRev.exe .. "version_in.h" "VersionRev.h" -f
IF %ERRORLEVEL% NEQ 0 GOTO NoSubWCRev
GOTO END

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

:END
POPD
ENDLOCAL
EXIT /B
