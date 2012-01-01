@ECHO OFF
SETLOCAL

PUSHD %~dp0

AStyle.exe --version 1>&2 2>NUL

IF %ERRORLEVEL% NEQ 0 (
  ECHO.
  ECHO ERROR: Astyle wasn't found!
  ECHO Visit http://astyle.sourceforge.net/ for download and details.
  PAUSE
  GOTO END
)

AStyle.exe --style=kr^
 -s4 --indent-switches --indent-namespaces --indent-col1-comments^
 --add-brackets^
 --pad-header --pad-oper --unpad-paren^
 --align-pointer=name^
 --preserve-date^
 --recursive ..\src\*.c ..\src\*.h

IF %ERRORLEVEL% NEQ 0 (
  ECHO.
  ECHO ERROR: Something went wrong!
  PAUSE
)

:END
POPD
PAUSE
ENDLOCAL
EXIT /B
