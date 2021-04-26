# сборка проекта под текущей платформой
# вызыватся тока извне: make build
#==============================================================================================
PLATFORM_DIR:=$(patsubst /cygdrive/c%,c:%, $(shell cd .; pwd))
PLATFORM_NAME:=ERROR

PLATFORM_BINS:=$(PLATFORM_DIR)/bins
ENGINE_BINS:=$(PLATFORM_DIR)/bins/engine
PROJECT_BINS:=$(PLATFORM_DIR)/bins/$(PROJECT_NAME)

.PHONY: build rebuild clean

rebuild: clean build
	
build:
	@echo --------------------------------------------------------------------------------------- Build $(PROJECT_NAME) for $(PLATFORM_NAME)
	@-mkdir -p bins
	@-mkdir -p bins/engine
	@-mkdir -p bins/engine/zip
	@-mkdir -p bins/engine/ogg
	@-mkdir -p bins/$(PROJECT_NAME)
	@make compile -f $(PLATFORM_DIR)/local.mak -C $(PLATFORM_BINS) 		SRCS_DIR=$(PLATFORM_DIR)/sources
	@make compile -f $(PLATFORM_DIR)/local.mak -C $(ENGINE_BINS)   		SRCS_DIR=$(ENGINE_DIR)/sources
	@make compile -f $(PLATFORM_DIR)/local.mak -C $(ENGINE_BINS)/zip	SRCS_DIR=$(ENGINE_DIR)/sources/zip
	@make compile -f $(PLATFORM_DIR)/local.mak -C $(ENGINE_BINS)/ogg	SRCS_DIR=$(ENGINE_DIR)/sources/ogg
	@make compile -f $(PLATFORM_DIR)/local.mak -C $(PROJECT_BINS)		SRCS_DIR=$(PROJECT_DIR)/sources
	@make link    -f $(PLATFORM_DIR)/local.mak -C $(PLATFORM_BINS)
	@make distribute -f $(PLATFORM_DIR)/local.mak -C $(PROJECT_DIR)
	@echo "  Done."

clean:
	@-rm -fr $(PLATFORM_DIR)/bins
	@make clean -f $(PLATFORM_DIR)/local.mak

.SUFFIXES: