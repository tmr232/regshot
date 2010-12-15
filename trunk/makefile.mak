#******************************************************************************
#*
#* Regshot
#*
#* makefile.mak
#*   makefile for building Regshot with WDK
#*
#* See gpl.txt for details about distribution and modification.
#*
#*                                       (c) XhmikosR 2010
#*                                       http://code.google.com/p/regshot/
#*
#* Use build.cmd and set there your WDK and SDK directories.
#*
#******************************************************************************

CC=cl
RC=rc
LD=link
MT=mt


!IFDEF x64
BINDIR=Release_x64
!ELSE
BINDIR=Release
!ENDIF
OBJDIR=$(BINDIR)\obj
APP=$(BINDIR)\RegShot.exe

DEFINES=/D "_WINDOWS" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS"
CXXFLAGS=/nologo /c /Fo"$(OBJDIR)/" /W3 /EHsc /MD /O2 /GS /GT /GL /MP /wd4133 /wd4819 $(DEFINES)
LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib \
	ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
LDFLAGS=/NOLOGO /INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /LTCG
RFLAGS=/d "_UNICODE" /d "UNICODE"


!IFDEF x64
CXXFLAGS=$(CXXFLAGS) /D "_WIN64" /D "_WIN32_WINNT=0x0502" /wd4244 /wd4267
RFLAGS=$(RFLAGS) /d "_WIN64"
LIBS=$(LIBS) msvcrt_win2003.obj
LDFLAGS=$(LDFLAGS) /SUBSYSTEM:WINDOWS,5.02 /MACHINE:X64 $(LIBS)
!ELSE
CXXFLAGS=$(CXXFLAGS) /D "WIN32" /D "_WIN32_WINNT=0x0501"
RFLAGS=$(RFLAGS) /d "WIN32"
LIBS=$(LIBS) msvcrt_winxp.obj
LDFLAGS=$(LDFLAGS) /SUBSYSTEM:WINDOWS,5.01 /MACHINE:X86 $(LIBS)
!ENDIF


.PHONY:	ALL CHECKDIRS

CHECKDIRS:
		-@ MKDIR "$(OBJDIR)" >NUL 2>&1

ALL:	CHECKDIRS $(APP)

CLEAN:
	-@ DEL "$(APP)" "$(OBJDIR)\*.idb" "$(OBJDIR)\*.obj" "$(BINDIR)\*.pdb" \
	"$(OBJDIR)\*.res" >NUL 2>&1
	-@ RMDIR /Q "$(OBJDIR)" "$(BINDIR)" >NUL 2>&1


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



{src\}.c{$(OBJDIR)}.obj::
	@$(CC) $(CXXFLAGS) /Tc $<


$(APP): $(OBJECTS)
	@$(RC) $(RFLAGS) /fo"$(OBJDIR)\regshot.res" "src\regshot.rc"
	@$(LD) $(LDFLAGS) /OUT:"$(APP)" $(OBJECTS)
#	@$(MT) -nologo -manifest "$(RES)\regshot.exe.manifest" -outputresource:"$(APP);#1"


# Dependencies

# src
$(OBJDIR)\fileshot.obj: src\fileshot.c src\global.h
$(OBJDIR)\language.obj: src\language.c src\global.h
$(OBJDIR)\misc.obj: src\misc.c src\global.h
$(OBJDIR)\output.obj: src\output.c src\global.h
$(OBJDIR)\Regshot.obj: src\Regshot.c src\global.h
$(OBJDIR)\regshot.res: src\regshot.rc src\resource.h
$(OBJDIR)\setup.obj: src\setup.c src\global.h
$(OBJDIR)\ui.obj: src\ui.c src\global.h
$(OBJDIR)\winmain.obj: src\winmain.c src\global.h
