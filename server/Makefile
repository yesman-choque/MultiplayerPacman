CXX := g++
CFLAGS := -Wall -Wextra

SRCDIR := src
INCDIR := interfaces
BUILDDIR := build
TARGET := bin/program

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	@mkdir -p bin
	@$(CXX) $^ -o $(TARGET)
	@rm -rf $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	@$(CXX) $(CFLAGS) -I$(INCDIR) -c $< -o $@

.PHONY: clean run

clean:
	@rm -rf $(BUILDDIR) bin

run:
	./$(TARGET)
