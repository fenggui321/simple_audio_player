LOCAL_PATH:= $(CURDIR)


#compiler params
LDFLAGS += 
INCLUDE += -I/usr/local/include -I/usr/include/SDL2/ -I/usr/include/json-c
CFLAGS  += -g  -Wall -Wno-deprecated-declarations
CXXFLAGS+= -g

#build test code param
ifndef BUILD_OUT_PATH
	BUILD_OUT_PATH=$(LOCAL_PATH)/out
	export BUILD_OUT_PATH
endif
BUILD_OUT_BIN_PATH:=$(BUILD_OUT_PATH)/bin
TARGET_NAME:= $(BUILD_OUT_BIN_PATH)/miplayer

#collect need compile src
PROJ_SRCS:= $(wildcard $(LOCAL_PATH)/*.c $(LOCAL_PATH)/*.cpp)

DEPEND_LIB:= -lpthread -lavcodec -lavformat -lavutil -lavresample -lasound -lSDL2 -ljson-c -lmosquitto
#DEPEND_LIB:= -lpthread -lavcodec -lavformat -lavutil -lavresample -lswresample -lasound -lSDL2 -ljson-c -lmosquitto


$(warning "################$(DEPEND_LIB)#######$(LDFLAGS)#######")
all: $(TARGET_NAME)

$(TARGET_NAME): $(PROJ_SRCS)
	mkdir -p $(BUILD_OUT_BIN_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -o $@  $(PROJ_SRCS) $(DEPEND_LIB)
clean:
	rm -rf $(BUILD_OUT_PATH)
