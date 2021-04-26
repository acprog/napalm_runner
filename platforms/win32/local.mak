# сборка файлов под текущую платформу

SRCS_DIR:=ERROR					# каталог исходников, ресурсов и пр.
BINS_DIR:=$(patsubst /cygdrive/c%,c:%, $(shell cd .; pwd))    # текущий каталог (назначени€)
BINS:=$(patsubst %.cpp, %.o, $(notdir $(wildcard $(SRCS_DIR)/*.cpp)))

MSVC:=C:\Program Files\Microsoft Visual Studio .NET\Vc7
TOOLS_DIR:=$(MSVC)\bin
INCLUDES:=/I"$(MSVC)\PlatformSDK\Include" /I"$(MSVC)\include"
RES_INC:=/i"$(MSVC)\atlmfc\include"
LIBS:=/LIBPATH:"$(MSVC)\PlatformSDK\Lib" /LIBPATH:"$(MSVC)\Lib" /LIBPATH:"$(MSVC)\PlatformSDK\Lib\prerelease"

CFLAGS:=/wd4305 /wd4355 /wd4244 /O2 /Ob2 /Oy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /GF /FD /EHsc /ML /Zp4 /GS /Gy /GR /W2 /nologo /c /Wp64 /Zi /TP $(INCLUDES)
LDFLAGS:=dsound.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /INCREMENTAL:NO /NOLOGO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /MACHINE:IX86  $(LIBS)

#----------------------------------------------------------------
.PHONY: compile link clean distribute res_compile

# компил€ци€ файлов из SRCS_DIR
compile: $(BINS)

# получение объектных файлов
%.o: $(SRCS_DIR)/%.cpp
	@"$(TOOLS_DIR)/cl" $(CFLAGS) $<
	@cp $(patsubst %.o, %.obj, $@) $@
	@rm $(patsubst %.o, %.obj, $@)
	@gcc -MM $< -o $(patsubst %.o, %.d, $@) 

-include *.d

%.res: ../resources/%.rc
	"$(TOOLS_DIR)/rc" $(RES_INC) $(INCLUDES) /fo"$@" $<

# сборка всех полученных модулей
link: $(wildcard *.o) $(wildcard common/*.o) $(wildcard common/zip/*.o) $(wildcard common/ogg/*.o) $(wildcard $(PROJECT_NAME)/*.o) win32.res
	@"$(TOOLS_DIR)/link" $(LDFLAGS) $^ /OUT:"$(PROJECT_DIR)/win32.exe"
	
clean:
	-@rm -f $(PROJECT_DIR)/win32.exe

distribute:
	@$(DISTRIBUTIV) win32.exe


.SUFFIXES: