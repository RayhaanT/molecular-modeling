CC = gcc
CXX = g++

LIBS = -L$(lib_dir)
CFLAGS = -g $(INCLUDES)
CXXFLAGS = -g $(INCLUDES)
INCLUDES = -iquote $(inc_dir)
LDFLAGS = $(LIBS) -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl

BIN = bin
SRC_DIR = src
inc_dir = include
lib_dir = lib

TARGET = model
CPP_SRC = $(shell find src -type f -name "*.cpp")
C_SRC = $(shell find src -type f -name "*.c")
SRC_OBJS = $(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)
OBJS = $(patsubst $(SRC_DIR)/%,$(BIN)/%,$(SRC_OBJS))

all: $(TARGET)

$(BIN)/%.o: $(SRC_DIR)/%.cpp 
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BIN)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean :
	rm $(SRC_OBJS)
	rm $(TARGET)
