# Compiler and flags
CXX = c++
CXXFLAGS = -std=c++98 -Iincludes -Wall -Wextra
LDFLAGS =

# Source files and object files
SRCS = $(wildcard src/*.cpp) main.cpp schema.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = json_validator

# Default rule
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Compilation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
