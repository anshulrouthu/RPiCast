
CC= g++
CFLAGS=-Wall -g -O2
TARGET=rpicast-server rpicast-client
SRC=./source
SAMPLES=./samples/*.cpp
BIN=bin
INCLUDE=
LIBS= -lcurl -lUnitTest++ -lpthread
TMP=tmp
INC=-Itarget/include/ -Isource/components/ -Isource/framework/ -Isource/osapi/
LDPATH= -Ltarget/lib/

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
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

############ ----- build samples ----- ##############

.PHONY: sample
sample:
						   	
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
			
############ ----- build tests ----- ##############

.PHONY: tests
tests: unittests test_osapi 
	   	
unittests: source/tests/unittests.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

test_osapi: source/tests/test_osapi.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/components/*.o           \
	        source/framework/*.o            \
	        source/main/*.o                 \
	        samples/*.o                     \
	        source/tests/*.o  
