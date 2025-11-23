# Simple Makefile for BondForge V2.0
# This is a simplified build system for quick compilation

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
QTFLAGS = $(shell pkg-config --cflags Qt6Core Qt6Widgets Qt6Charts Qt6WebEngineCore Qt6WebEngineWidgets)
QTLIBS = $(shell pkg-config --libs Qt6Core Qt6Widgets Qt6Charts Qt6WebEngineCore Qt6WebEngineWidgets)

# Optional dependencies
RDKIT_CFLAGS = $(shell pkg-config --cflags rdkit 2>/dev/null)
RDKIT_LIBS = $(shell pkg-config --libs rdkit 2>/dev/null)
MLPACK_CFLAGS = $(shell pkg-config --cflags mlpack 2>/dev/null)
MLPACK_LIBS = $(shell pkg-config --libs mlpack 2>/dev/null)

# Define optional compilation flags
ifdef ENABLE_RDKIT
    CXXFLAGS += -DRDKIT_ENABLED $(RDKIT_CFLAGS)
    QTLIBS += $(RDKIT_LIBS)
endif

ifdef ENABLE_MLPACK
    CXXFLAGS += -DMLPACK_ENABLED $(MLPACK_CFLAGS)
    QTLIBS += $(MLPACK_LIBS)
endif

# Directories
SRCDIR = .
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
BINDIR = $(BUILDDIR)/bin

# Source files
SOURCES = main.cpp \
          $(wildcard core/*/*.cpp) \
          $(wildcard ui/*.cpp) \
          $(wildcard services/*.cpp) \
          $(wildcard utils/*.cpp)

# Object files
OBJECTS = $(SOURCES:%.cpp=$(OBJDIR)/%.o)

# Target
TARGET = $(BINDIR)/BondForge

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(OBJDIR)/core/data
	@mkdir -p $(OBJDIR)/core/chemistry
	@mkdir -p $(OBJDIR)/core/ml
	@mkdir -p $(OBJDIR)/core/collaboration
	@mkdir -p $(OBJDIR)/core/permissions
	@mkdir -p $(OBJDIR)/core/plugins
	@mkdir -p $(OBJDIR)/ui
	@mkdir -p $(OBJDIR)/services
	@mkdir -p $(OBJDIR)/utils
	@mkdir -p $(BINDIR)

# Build target
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(QTLIBS)

# Compile source files
$(OBJDIR)/%.o: %.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) -c $< -o $@

# Clean build
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILDDIR)

# Install target
install: $(TARGET)
	@echo "Installing BondForge..."
	mkdir -p $(DESTDIR)/usr/local/bin
	cp $(TARGET) $(DESTDIR)/usr/local/bin/
	mkdir -p $(DESTDIR)/usr/local/share/BondForge
	cp -r config $(DESTDIR)/usr/local/share/BondForge/
	cp -r examples $(DESTDIR)/usr/local/share/BondForge/
	cp -r plugins $(DESTDIR)/usr/local/share/BondForge/

# Uninstall target
uninstall:
	@echo "Uninstalling BondForge..."
	rm -f $(DESTDIR)/usr/local/bin/BondForge
	rm -rf $(DESTDIR)/usr/local/share/BondForge

# Run target
run: $(TARGET)
	@echo "Running BondForge..."
	$(TARGET)

# Enable optional dependencies
with-rdkit:
	$(MAKE) ENABLE_RDKIT=1

with-mlpack:
	$(MAKE) ENABLE_MLPACK=1

with-all:
	$(MAKE) ENABLE_RDKIT=1 ENABLE_MLPACK=1

.PHONY: all directories clean install uninstall run with-rdkit with-mlpack with-all