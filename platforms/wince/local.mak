# сборка файлов под текущую платформу

SRCS_DIR:=ERROR					# каталог исходников, ресурсов и пр.
BINS_DIR:=$(patsubst /cygdrive/c%,c:%, $(shell cd .; pwd))    # текущий каталог (назначени€)
BINS:=$(patsubst %.cpp, %.o, $(notdir $(wildcard $(SRCS_DIR)/*.cpp)))

WCED:=C:\Program Files\Windows CE Tools\wce420\POCKET PC 2003
TOOLS_DIR:=C:\Program Files\Microsoft eMbedded C++ 4.0\EVC\WCE400\BIN
RC:="c:\Program Files\Microsoft eMbedded C++ 4.0\Common\EVC\Bin\rc.exe"
INCLUDES:=/I"$(WCED)\include\armv4"
RES_INC:=/l 0x409 /d UNDER_CE=420 /d _WIN32_WCE=420 /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "WIN32_PLATFORM_PSPC=400" /d "ARM" /d "_ARM_" /d "ARMV4" /r /I"C:\Program Files\Microsoft eMbedded C++ 4.0\EVC\Include\Win32" $(INCLUDES)
LIBS:=/LIBPATH:"$(WCED)\Lib\Armv4"
CFLAGS:=/nologo /Zp4 /w /W0 /Oxt /Ob2 /D _WIN32_WCE=420 /D "UNICODE" /D "_UNICODE" /D "WIN32_PLATFORM_PSPC=400" /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D "NDEBUG" /MC /c $(INCLUDES)
LDFLAGS:=gx.lib commctrl.lib coredll.lib aygshell.lib "/base:0x00010000" /stack:0x10000,0x1000 "/entry:WinMainCRTStartup" /incremental:no "/pdb:winmobile.pdb" "/nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib" /subsystem:windowsce,4.20 "/align:4096" /MACHINE:ARM $(LIBS)

#----------------------------------------------------------------
.PHONY: compile link clean distribute res_compile

# компил€ци€ файлов из SRCS_DIR
compile: $(BINS)

# получение объектных файлов
%.o: $(SRCS_DIR)/%.cpp
	@"$(TOOLS_DIR)/clarm" $(CFLAGS) $<
	@cp $(patsubst %.o, %.obj, $@) $@
	@rm $(patsubst %.o, %.obj, $@)
	@gcc -MM $< -o $(patsubst %.o, %.d, $@) 

-include *.d

%.res: ../resources/%.rc
	$(RC) $(RES_INC) /fo "$@" $<

# сборка всех полученных модулей
link: $(wildcard *.o) $(wildcard common/*.o) $(wildcard common/zip/*.o) $(wildcard common/ogg/*.o) $(wildcard $(PROJECT_NAME)/*.o) wince.res
	@"$(TOOLS_DIR)/link" $(LDFLAGS) $^ /OUT:"$(PROJECT_DIR)/winmobile.exe"
	
clean:
	-@rm -f $(PROJECT_DIR)/winmobile.exe

distribute:
	@$(DISTRIBUTIV) winmobile.exe

.SUFFIXES: