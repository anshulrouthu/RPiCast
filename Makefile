
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
LDLIBS:= $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS)

CC:=g++
CFLAGS:=$(CFLAGS) -Wall -g -O2
TARGET:=rpicast-server rpicast-client
SRC:=./source
SAMPLES:=./samples/*.cpp
BIN:=bin
INCLUDE=
LDLIBS:=$(LDLIBS) -lcurl -lUnitTest++ -lpthread
INC:=-Itarget/include/ -Isource/components/ -Isource/framework/ -Isource/osapi/
LDPATH:=-Ltarget/lib/

#list of files containing main() function, to prevent conflicts while linking
MAINFILES:=source/main/console_command.cpp    
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/components/*.cpp) $(wildcard source/framework/*.cpp) $(wildcard source/osapi/*.cpp)))

############ ----- build main application ----- ##############

.PHONY: all
all: bin $(OBJS) $(TARGET) sample tests
	@echo "Build successful"

bin: 
	@mkdir -p $@
	
$(TARGET):source/main/console_command.o $(OBJS) 
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

############ ----- build samples ----- ##############

.PHONY: sample
sample: screencapture

screencapture: samples/screencapture.o $(OBJS)
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)	   	
						   	
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
			
############ ----- build tests ----- ##############

.PHONY: tests
tests: unittests test_osapi 
	   	
unittests: source/tests/unittests.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

test_osapi: source/tests/test_osapi.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LDLIBS)

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/components/*.o           \
	        source/framework/*.o            \
	        source/main/*.o                 \
	        samples/*.o                     \
	        source/tests/*.o  
