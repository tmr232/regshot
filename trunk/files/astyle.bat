@ECHO OFF
SETLOCAL EnableDelayedExpansion

SET "ASTYLE=AStyle.exe"

%ASTYLE% -s4 --indent-switches --indent-namespaces --add-brackets --indent-col1-comments^
 --pad-header --align-pointer=middle --align-reference=middle --preserve-date --pad-oper^
 --unpad-paren ..\src\*.h ..\src\*.c

IF %ERRORLEVEL% NEQ 0 ECHO. & ECHO ERROR: Something went wrong! & PAUSE

ENDLOCAL
EXIT /B