CC = gcc
CXX = g++

LIBS = -L$(lib_dir)
CFLAGS = -g $(INCLUDES)
CXXFLAGS = -g $(INCLUDES)
INCLUDES = -iquote $(inc_dir)
LDFLAGS = $(LIBS) -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl

BIN = ./bin
inc_dir = include
lib_dir = lib

TARGET = model
CPP_SRC = $(shell find src -type f -name "*.cpp")
C_SRC = $(shell find src -type f -name "*.c")
OBJS = $(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean :
	rm model $(OBJS)
