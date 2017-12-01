#*********************************************************************
# RPiCast ( Screencasting application using RaspberryPi )
#
# Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>
#
# All rights reserved.
#
# This file Makefile is part of RPiCast project
#
# RPiCast is a free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************/

PROJECT_ROOT:=$(PWD)

FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

PKG_CONFIG_PATH=$(PROJECT_ROOT)/staging/lib/pkgconfig
#CFLAGS:= $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
FFMPEGLIBS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(FFMPEG_LIBS))

BUILD_PATH:=build
CC:=g++
RPATH:=$(PROJECT_ROOT)/staging/lib/
CFLAGS:=-Wall -Werror -fpic -g -O2 -Wl,-rpath=$(RPATH)
EXT_LDLIBS:=-lUnitTest++ $(FFMPEGLIBS)
EXT_LDPATH:=-L$(PROJECT_ROOT)/staging/lib
LDFLAGS:=-Lbuild/ -lrpicast

############ ----- Project include paths ----- ##############
INC:=-I$(PROJECT_ROOT)/staging/include/                                \
     -I$(PROJECT_ROOT)/source/osapi/                                   \
     -I$(PROJECT_ROOT)/source/framework/                               \
     -I$(PROJECT_ROOT)/source/porting_layers/components/               \
     -I$(PROJECT_ROOT)/source/porting_layers/av_pipe/

#list of files containing main() function, to prevent conflicts while linking
MAINFILES:=$(PROJECT_ROOT)/source/main/rpicast.cpp         \
           $(PROJECT_ROOT)/source/main/rpicast-server.cpp  \
           $(PROJECT_ROOT)/source/porting_layers/components/video_tunnel.cpp
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard $(PROJECT_ROOT)/source/porting_layers/components/*.cpp) \
                                                       $(wildcard $(PROJECT_ROOT)/source/framework/*.cpp)                 \
                                                       $(wildcard $(PROJECT_ROOT)/source/osapi/*.cpp)                     \
                                                       $(wildcard $(PROJECT_ROOT)/source/porting_layers/av_pipe/*.cpp)))

############ ----- build main application ----- ##############

TARGET:=$(BUILD_PATH)/rpicast
TARGET_SERVER:=$(BUILD_PATH)/rpicast-server
TARGET_LIB:=$(BUILD_PATH)/librpicast.so

.PHONY: all
all: $(BUILD_PATH) libs $(TARGET) tests

$(BUILD_PATH):
	          @mkdir -p $@
	
.PHONY:libs
libs: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	       @echo "Linking... $@"
	       @$(CC) $(CFLAGS) -fpic -shared $(EXT_LDPATH) $^ -o $@ $(EXT_LDLIBS)

$(TARGET): $(PROJECT_ROOT)/source/main/rpicast.o $(TARGET_LIB)
	   @echo "Linking... $@"
	   @$(CXX) $(CFLAGS) $(LDPATH) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): $(PROJECT_ROOT)/source/main/rpicast-server.o $(TARGET_LIB)
		  @echo "Linking... $@"
	          @$(CC) $(CFLAGS) $(LDPATH) $^ -o $@ $(LDFLAGS)
%.o: %.cpp
	  @echo "[CXX] $@"
	  @$(CXX) $(CFLAGS) $(INC) -c $< -o $@

%.o: %.c
	 @echo "[CXX] $@"
	 @$(CC) $(CFLAGS) $(INC) -c $< -o $@

############ ----- build samples ----- ##############

SAMPLES:= $(BUILD_PATH)/screencapture     \
          $(BUILD_PATH)/socket_server     \
          $(BUILD_PATH)/socket_client     \
          $(BUILD_PATH)/hello_world       \
          $(BUILD_PATH)/muxing

SAMPLE_SRC_DIR:=$(PROJECT_ROOT)/samples

.PHONY: sample
sample: $(TARGET_LIB) $(SAMPLES)

$(BUILD_PATH)/%: $(SAMPLE_SRC_DIR)/%.o
		 @echo "Linking... $@"
	         @$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(EXT_LDPATH) $(FFMPEGLIBS)
			
############ ----- build tests ----- ##############

TESTS:= $(BUILD_PATH)/unittests            \
        $(BUILD_PATH)/test_osapi           \
        $(BUILD_PATH)/test_socket          \
        $(BUILD_PATH)/test_socket_capture  \
        $(BUILD_PATH)/test_demux           \
        $(BUILD_PATH)/test_ssdp

TEST_SRC_DIR:= $(PROJECT_ROOT)/source/tests

.PHONY: tests
tests: $(TARGET_LIB) $(TESTS)
	   	
$(BUILD_PATH)/%: $(TEST_SRC_DIR)/%.o
		 @echo "Linking... $@"
	         @$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(EXT_LDPATH) -lUnitTest++

############ ----- cleaning ----- ##############

.PHONY: clean
clean:
	 @rm -f $(PROJECT_ROOT)/source/framework/*.o                 \
	        $(PROJECT_ROOT)/source/osapi/*.o                     \
	        $(PROJECT_ROOT)/source/main/*.o                      \
	        $(PROJECT_ROOT)/source/porting_layers/av_pipe/*.o    \
	        $(PROJECT_ROOT)/source/porting_layers/components/*.o \
	        $(PROJECT_ROOT)/source/tests/*.o                     \
	        $(PROJECT_ROOT)/samples/*.o

.PHONY:distclean
distclean:
	 @echo "Cleaning files..."
	 @rm -f $(TARGET)                            \
	        $(TARGET_LIB)                        \
	        $(BUILD_PATH)/*
	 @$(MAKE) -f Makefile.cross distclean         

############ ----- cross compilation ----- ##############

cross-tests: clean
	     @$(MAKE) -f Makefile.cross tests	clean

cross-samples: clean
	       @$(MAKE) -f Makefile.cross samples clean
	        
cross-libs: clean
	       @$(MAKE) -f Makefile.cross libs clean
	       
cross-distclean: clean
	         @$(MAKE) -f Makefile.cross distclean clean

cross-all: clean
	   @$(MAKE) -f Makefile.cross all clean
