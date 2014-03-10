
# use pkg-config for getting CFLAGS and LDLIBS

INCLUDE= -I/usr/local/include
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

#CFLAGS:= $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
FFMPEGLIBS:=$(shell pkg-config --libs $(FFMPEG_LIBS))

CC:=g++
CFLAGS:=-Wall -g -O2
TARGET:=rpicast
SRC:=./source
SAMPLES:=./samples/*.cpp
BIN:=bin
INCLUDE=
LDLIBS:=-lcurl -lUnitTest++ $(FFMPEGLIBS)

############ ----- Project include paths ----- ##############
INC:=-Itarget/include/                                 \
     -Isource/osapi/                                   \
     -Isource/framework/                               \
     -Isource/porting_layers/components/               \
     -Isource/porting_layers/av_pipe/


LDPATH:=-Ltarget/lib/

#list of files containing main() function, to prevent conflicts while linking
MAINFILES:=source/main/console_command.cpp
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/porting_layers/components/*.cpp)              \
                                                       $(wildcard source/framework/*.cpp)                              \
                                                       $(wildcard source/osapi/*.cpp)                                  \
                                                       $(wildcard source/porting_layers/av_pipe/*.cpp)))

############ ----- build main application ----- ##############

.PHONY: all
all: bin $(OBJS) $(TARGET) sample tests
	@echo "Build successful"

bin: 
	@mkdir -p $@
	
objs: $(OBJS)

$(TARGET):source/main/console_command.o $(OBJS) 
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

############ ----- build samples ----- ##############

.PHONY: sample
sample: screencapture     \
        socket_server     \
        socket_client

screencapture: samples/screencapture.o $(OBJS)
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

socket_server: samples/socket_server.o $(OBJS)
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

socket_client: samples/socket_client.o $(OBJS)
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)
						   	
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
			
############ ----- build tests ----- ##############

.PHONY: tests
tests: unittests            \
       test_osapi           \
       test_socket          \
       test_socket_capture
	   	
unittests: source/tests/unittests.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

test_osapi: source/tests/test_osapi.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

test_socket: source/tests/test_socket.o $(OBJS)
	    $(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

test_socket_capture: source/tests/test_socket_capture.o $(OBJS)
	    $(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/framework/*.o                 \
	        source/osapi/*.o                     \
	        source/main/*.o                      \
	        source/porting_layers/av_pipe/*.o    \
	        source/porting_layers/components/*.o \
	        source/tests/*.o