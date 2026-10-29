CXX ?= c++
PKG_CONFIG ?= pkg-config

TARGET := build/frameless-camera
OBJECTS := build/main.o
QT_PACKAGES := Qt6Widgets Qt6Multimedia Qt6MultimediaWidgets
QT_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(QT_PACKAGES) 2>/dev/null)
QT_LIBS := $(shell $(PKG_CONFIG) --libs $(QT_PACKAGES) 2>/dev/null)

CPPFLAGS += $(QT_CFLAGS)
CXXFLAGS += -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDLIBS += $(QT_LIBS)

.PHONY: all appimage check-deps clean run

all: $(TARGET)

check-deps:
	@$(PKG_CONFIG) --exists $(QT_PACKAGES) || { \
		echo "Missing Qt 6 development packages."; \
		echo "Install them with:"; \
		echo "  sudo apt install build-essential pkg-config qt6-base-dev qt6-multimedia-dev"; \
		exit 1; \
	}

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/main.o: src/main.cpp | build check-deps
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(OBJECTS) $(TARGET)
