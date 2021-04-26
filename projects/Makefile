# дл€ защиты от неполной компил€ции существует тока этот основной makefile, вс€ сборка проектов проходит через него
# он же вызывет остальные

ROOT_DIR:=$(patsubst /cygdrive/c%,c:%, $(shell cd ..; pwd))
COMMON_DIR:=$(ROOT_DIR)/common
PROJECTS:=$(patsubst %/, %, $(wildcard */))
PLATFORM:=all
BUILD_TYPE:=rebuild
DISTRIBUTIV:=zip zip.zip -q -9

export ROOT_DIR
export COMMON_DIR
export DISTRIBUTIV
export DISTRIBUTIV

.PHONY: all $(PROJECTS)

all: $(PROJECTS)

$(PROJECTS):
	@make -s -f selector.mak $(PLATFORM) -C $(addsuffix /platforms, $(ROOT_DIR)) BUILD_TYPE=$(BUILD_TYPE) PROJECT_DIR=$(patsubst /cygdrive/c%,c:%, $(shell cd $@; pwd)) PROJECT_NAME=$@
	@echo Make $@ distributive
	-@rm -f $@/$@.zip 
	@make -s -C $@/ distribute
	-@mv $@/zip.zip $@/$@.zip
	@echo "  Done."

.SUFFIXES: