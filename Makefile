
# use pkg-config for getting CFLAGS and LDLIBS

CROSSPREFIX:=arm-linux-gnueabihf-
#INCLUDE= -I/usr/local/include
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

#CFLAGS:= $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
FFMPEGLIBS:=$(shell pkg-config --libs $(FFMPEG_LIBS))

BUILD_PATH:=build
RPATH:=target_cross/usr/lib/
CC:=g++
CFLAGS:=-Wall -g -O2 -Wl,-rpath=$(RPATH)
EXT_LDLIBS:=-lcurl -lUnitTest++ $(FFMPEGLIBS)

EXT_LDPATH:=-Ltarget/lib
LDFLAGS:=-Lbuild/ -lrpicast

############ ----- Project include paths ----- ##############
INC:=-Itarget/include/                                 \
     -Isource/osapi/                                   \
     -Isource/framework/                               \
     -Isource/porting_layers/components/               \
     -Isource/porting_layers/av_pipe/

#list of files containing main() function, to prevent conflicts while linking
MAINFILES:=source/main/rpicast.cpp source/main/rpicast-server.cpp source/porting_layers/components/video_tunnel.cpp
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/porting_layers/components/*.cpp)              \
                                                       $(wildcard source/framework/*.cpp)                              \
                                                       $(wildcard source/osapi/*.cpp)                                  \
                                                       $(wildcard source/porting_layers/av_pipe/*.cpp)))

############ ----- build main application ----- ##############

TARGET:=$(BUILD_PATH)/rpicast
TARGET_SERVER:=$(BUILD_PATH)/rpicast-server
TARGET_LIB:=$(BUILD_PATH)/librpicast.so

.PHONY: all
all: $(BUILD_PATH) libs $(TARGET) $(TARGET_SERVER) sample tests

$(BUILD_PATH):
	          @mkdir -p $@
	
.PHONY:libs
libs: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	           $(CC) $(CFLAGS) -fpic -shared $(EXT_LDPATH) $^ -o $@ $(EXT_LDLIBS)

$(TARGET): source/main/rpicast.o $(TARGET_LIB)
	       $(CC) $(CFLAGS) $(LDPATH) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): source/main/rpicast-server.o $(TARGET_LIB)
	       $(CC) $(CFLAGS) $(LDPATH) $^ -o $@ $(LDFLAGS)
%.o: %.cpp
	 $(CC) $(CFLAGS) $(INC) -c $< -o $@

############ ----- build samples ----- ##############

SAMPLES:= $(BUILD_PATH)/screencapture     \
          $(BUILD_PATH)/socket_server     \
          $(BUILD_PATH)/socket_client     \
          $(BUILD_PATH)/hello_world

SAMPLE_SRC_DIR:=samples

.PHONY: sample
sample: $(TARGET_LIB) $(SAMPLES)

$(BUILD_PATH)/%: $(SAMPLE_SRC_DIR)/%.o
	             $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(EXT_LDPATH) $(FFMPEGLIBS)
			
############ ----- build tests ----- ##############

TESTS:= $(BUILD_PATH)/unittests            \
        $(BUILD_PATH)/test_osapi           \
        $(BUILD_PATH)/test_socket          \
        $(BUILD_PATH)/test_socket_capture  \
        $(BUILD_PATH)/test_demux

TEST_SRC_DIR:= source/tests

.PHONY: tests
tests: $(TARGET_LIB) $(TESTS)
	   	
$(BUILD_PATH)/%: $(TEST_SRC_DIR)/%.o
	             $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(EXT_LDPATH) -lUnitTest++

############ ----- cleaning ----- ##############

.PHONY: clean
clean:
	 @rm -f source/framework/*.o                 \
	        source/osapi/*.o                     \
	        source/main/*.o                      \
	        source/porting_layers/av_pipe/*.o    \
	        source/porting_layers/components/*.o \
	        source/tests/*.o                     \
	        samples/*.o

.PHONY:distclean
distclean:
	 @echo "Cleaning files..."
	 @rm -f source/framework/*.o                 \
	        source/osapi/*.o                     \
	        source/main/*.o                      \
	        source/porting_layers/av_pipe/*.o    \
	        source/porting_layers/components/*.o \
	        source/tests/*.o                     \
	        samples/*.o                          \
	        $(TARGET)                            \
	        $(TARGET_LIB)                        \
	        $(BUILD_PATH)/*

############ ----- cross compilation ----- ##############

cross-tests: clean
	        $(MAKE) -f Makefile-cross tests	clean

cross-samples: clean
	        $(MAKE) -f Makefile-cross samples clean
	        
cross-libs: clean
	        $(MAKE) -f Makefile-cross libs clean
	       
cross-distclean: clean
	        $(MAKE) -f Makefile-cross distclean clean

cross-all: clean
	        $(MAKE) -f Makefile-cross all clean