PROJECT:=*
PLATFORM:=*
BUILD_TYPE:=rebuild
TO_BACKUP:=	* \
			common/*/* \
			common/*/*/* \
			projects/* \
			projects/$(PROJECT)/sources/* \
			projects/$(PROJECT)/interface/*/*.xml \
			platforms/* \
			platforms/$(PLATFORM)/* \
			platforms/$(PLATFORM)/sources/* \
			tools/makePRC/*.cpp \
			tools/makePRC/*.h \
			"tools/MPP Launcher/sources/*" \
			"tools/MPP Launcher/*"

.PHONY: all build backup

all: build backup

build:
	@make -s -C projects/ BUILD_TYPE=$(BUILD_TYPE)

backup:
	@echo Backup sources $(PROJECT) for $(PLATFORM)
	@zip -q -1 backup.zip $(TO_BACKUP)
	@tools/make_backup backup.zip /ac_lab/backup/mpp
	@rm backup.zip
	@echo "  All Done."
