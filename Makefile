.DEFAULT_GOAL := all
CXX ?= g++
BUILD ?= build-haiku
RC ?= rc
XRES ?= xres
MIMESET ?= mimeset
HOST_CXX ?= g++
CPPFLAGS += -Isrc -Isrc/model -Isrc/hw -Isrc/ui $(CROSS_CPPFLAGS)
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++17 -Wall -Wextra -Wno-multichar -Wno-unused-parameter
APP_CPPFLAGS ?= -I/boot/system/develop/headers/private/interface
MODEL_SRC = $(wildcard src/model/*.cpp)
APP_SRC = $(wildcard src/*.cpp src/hw/*.cpp src/ui/*.cpp) $(MODEL_SRC)
APP_OBJ = $(APP_SRC:%.cpp=$(BUILD)/%.o)

.PHONY: all check check-host clean package icons

all: $(BUILD)/AirPins

$(BUILD)/AirPins: $(APP_OBJ) resources/AirPins.rdef resources/airpins-icon.hvif
	$(CXX) $(APP_LDFLAGS) -o $@.new $(APP_OBJ) -lbe -ltracker $(APP_LDEND)
	$(RC) -o $(BUILD)/AirPins.rsrc resources/AirPins.rdef
	$(XRES) -o $@.new $(BUILD)/AirPins.rsrc
	$(MIMESET) -f $@.new
	mv $@.new $@

$(BUILD)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(APP_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# the model's tests run on Haiku and on any other host
$(BUILD)/model_tests: tests/ModelTests.cpp $(MODEL_SRC)
	mkdir -p $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

check: $(BUILD)/model_tests
	$(BUILD)/model_tests

check-host:
	mkdir -p build-host
	$(HOST_CXX) -std=c++17 -Wall -Wextra -O1 -g -fsanitize=address,undefined \
		-Isrc/model -o build-host/model_tests tests/ModelTests.cpp $(MODEL_SRC)
	build-host/model_tests

# needs Python with fontTools (toolbar icons) and Pillow (icon preview)
icons:
	python3 tools/make-tool-icons.py
	python3 tools/make-icon.py resources/airpins-icon.hvif

package: all
	bash tools/package-haiku.sh

clean:
	rm -rf $(BUILD) build-host

-include $(APP_OBJ:.o=.d)
