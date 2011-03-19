#******************************************************************************
#*
#* Regshot
#*
#* makefile.mak
#*   makefile for building Regshot with WDK
#*
#* See gpl.txt for details about distribution and modification.
#*
#*                                       (c) XhmikosR 2010-2011
#*                                       http://code.google.com/p/regshot/
#*
#* Use build.cmd and set there your WDK and SDK directories.
#*
#******************************************************************************


# Remove the .SILENT directive in order to display all the commands
.SILENT:

!IFDEF x64
BINDIR  = ..\bin\WDK\Release_x64
!ELSE
BINDIR  = ..\bin\WDK\Release_x86
!ENDIF
OBJDIR  = $(BINDIR)\obj
EXE     = $(BINDIR)\Regshot.exe
SRC     = ..\src


DEFINES = /D "_WINDOWS" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS"
CFLAGS  = /nologo /c /Fo"$(OBJDIR)/" /W3 /WX /EHsc /MD /O2 /GL /MP $(DEFINES)
LDFLAGS = /NOLOGO /WX /INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /LTCG /MERGE:.rdata=.text \
          /DYNAMICBASE /NXCOMPAT /DEBUG
LIBS    = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib \
          ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
MTFLAGS = -nologo
RFLAGS  = /l 0x0409


!IFDEF x64
CFLAGS  = $(CFLAGS) /D "_WIN64" /D "_WIN32_WINNT=0x0502" /wd4267
LIBS    = $(LIBS) msvcrt_win2003.obj
LDFLAGS = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.02 /MACHINE:X64
RFLAGS  = $(RFLAGS) /d "_WIN64"
!ELSE
CFLAGS  = $(CFLAGS) /D "WIN32" /D "_WIN32_WINNT=0x0500"
LIBS    = $(LIBS) msvcrt_win2000.obj
LDFLAGS = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.0 /MACHINE:X86
RFLAGS  = $(RFLAGS) /d "WIN32"
!ENDIF


###############
##  Targets  ##
###############
BUILD:	CHECKDIRS $(EXE)

CHECKDIRS:
	-MKDIR "$(OBJDIR)" >NUL 2>&1

CLEAN:
	ECHO Cleaning... & ECHO.
	-DEL "$(EXE)" "$(OBJDIR)\regshot.idb" "$(OBJDIR)\*.obj" \
	"$(BINDIR)\regshot.pdb" "$(OBJDIR)\regshot.res" >NUL 2>&1
	-RMDIR /Q "$(OBJDIR)" "$(BINDIR)" >NUL 2>&1

REBUILD:	CLEAN BUILD


####################
##  Object files  ##
####################
OBJECTS= \
    $(OBJDIR)\fileshot.obj \
    $(OBJDIR)\language.obj \
    $(OBJDIR)\misc.obj \
    $(OBJDIR)\output.obj \
    $(OBJDIR)\Regshot.obj \
    $(OBJDIR)\regshot.res \
    $(OBJDIR)\setup.obj \
    $(OBJDIR)\ui.obj \
    $(OBJDIR)\winmain.obj


##################
##  Batch rule  ##
##################
{$(SRC)}.c{$(OBJDIR)}.obj::
    cl $(CFLAGS) /Tc $<


################
##  Commands  ##
################
$(EXE): $(OBJECTS)
	rc $(RFLAGS) /fo"$(OBJDIR)\regshot.res" "$(SRC)\regshot.rc" >NUL
	link $(LDFLAGS) $(LIBS) $(OBJECTS) /OUT:"$(EXE)"
	mt $(MTFLAGS) -manifest "$(SRC)\regshot.exe.manifest" -outputresource:"$(EXE);#1"


####################
##  Dependencies  ##
####################
$(OBJDIR)\fileshot.obj: \
    $(SRC)\fileshot.c \
    $(SRC)\global.h

$(OBJDIR)\language.obj: \
    $(SRC)\language.c \
    $(SRC)\global.h

$(OBJDIR)\misc.obj: \
    $(SRC)\misc.c \
    $(SRC)\global.h

$(OBJDIR)\output.obj: \
    $(SRC)\output.c \
    $(SRC)\global.h

$(OBJDIR)\Regshot.obj: \
    $(SRC)\Regshot.c \
    $(SRC)\global.h

$(OBJDIR)\regshot.res: \
    $(SRC)\regshot.rc \
    $(SRC)\resource.h

$(OBJDIR)\setup.obj: \
    $(SRC)\setup.c \
    $(SRC)\global.h

$(OBJDIR)\ui.obj: \
    $(SRC)\ui.c \
    $(SRC)\global.h

$(OBJDIR)\winmain.obj: \
    $(SRC)\winmain.c \
    $(SRC)\global.h
