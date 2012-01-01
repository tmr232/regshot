@ECHO OFF
SETLOCAL

PUSHD %~dp0%

SET "ASTYLE=AStyle.exe"

IF NOT EXIST %ASTYLE% (
  ECHO.
  ECHO ERROR: Astyle wasn't found!
  PAUSE
  EXIT /B
)

%ASTYLE% -s4 --style=kr --indent-switches --indent-namespaces --add-brackets^
 --indent-col1-comments --pad-header --align-pointer=name --align-reference=name^
 --preserve-date --pad-oper --unpad-paren --recursive ..\src\*.c ..\src\*.h

IF %ERRORLEVEL% NEQ 0 (
  ECHO.
  ECHO ERROR: Something went wrong!
  PAUSE
)

POPD
PAUSE
ENDLOCAL
EXIT /B
