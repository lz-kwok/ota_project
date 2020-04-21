PROJECT_DIR := $(shell pwd)

CC = arm-linux-gnueabihf-g++
# CC = g++

DIR_INC = $(PROJECT_DIR)/inc
DIR_SRC = $(PROJECT_DIR)/src
DIR_BIN = $(PROJECT_DIR)/bin

SRC = $(wildcard ${DIR_SRC}/*.cpp ${DIR_SRC}/*.c)  

INCLUDE  =  -I../extern/include 
INCLUDE  += -I$(DIR_INC)
CPPFLAGS =  -Wall -fopenmp -O2 -fPIC -g -D__LINUX__ -Wl,-rpath=../extern/libs,--disable-new-dtags -L../extern/libs -lcurl -lz
CPPFLAGS += -lrt -ldl -lpthread 


TARGET = ota
BIN_TARGET = ${DIR_BIN}/${TARGET}

all:ota

ota:
	$(CC) -o $(BIN_TARGET) $(SRC) $(INCLUDE) $(CPPFLAGS)

	
.PHONY : install clean  
clean:
	rm -rf ./bin/ota
	@echo "clean done" 
