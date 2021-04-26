# разборка проекта по платформам и сборка платформ

BUILD_TYPE:=ERROR
PROJECT_DIR:=ERROR
PROJECT_NAME:=ERROR
PLATFORMS:=$(patsubst %/, %, $(wildcard */))
PLATFORM_NAME=ERROR

export PROJECT_DIR
export PLATFORM_NAME
export PROJECT_NAME

.PHONY: all $(PLATFORMS)

all: $(PLATFORMS)

$(PLATFORMS):
	@make -f $(ROOT_DIR)/platforms/builder.mak $(BUILD_TYPE) -C $@ PLATFORM_NAME=$@

.SUFFIXES: