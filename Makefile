CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Iinclude -Itests
SANFLAGS = -std=c++20 -g -fsanitize=address,undefined -Iinclude -Itests

SOURCES = $(wildcard src/*.cpp) $(wildcard tests/*.cpp)
HEADERS = $(wildcard include/*.hpp) $(wildcard tests/*.hpp)

all: build/app

build:
	mkdir -p build

build/app: $(SOURCES) $(HEADERS) | build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o build/app

run: build/app
	./build/app

test: build/app
	./build/app tests

benchmark: build/app
	./build/app benchmark

sanitize: | build
	$(CXX) $(SANFLAGS) $(SOURCES) -o build/app_asan
	./build/app_asan tests

clean:
	rm -rf build

.PHONY: all run test benchmark sanitize clean
