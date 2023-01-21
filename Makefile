CXX                ?= g++
CXX_OPTS           ?= -Wall -Werror -Wextra -g -fPIC -lcurlpp -lcurl -I./inc
SRC                 = $(wildcard src/*.cc)
BIN                 = $(patsubst src/%.cc, bin/%.o, $(SRC))
TARGET             ?= /usr
VERSION             = 1
OUT                 = target/package
all: build
bin/%.o: src/%.cc
	@echo "Compiling $^ into $@"
	@$(CXX) $(CXX_OPTS) -c -o $@ $^
build: $(BIN)
	@echo "Linking $(BIN) into $(OUT)"
	@$(CXX) $(CXX_OPTS) -o $(OUT) $(BIN)
install: build
	cp $(OUT) $(TARGET)/bin
	mkdir -p /etc/Package
	mkdir -p /etc/Package/Cache
	mkdir -p /etc/Package/Cache/Installer
	mkdir -p /etc/Package/Cache/Downloader
	mkdir -p /Apps
	chmod  a+rw /Apps
	chmod -R a+rw /etc/Package
	@echo "To finish installation execute: sh scripts/PostBuild.sh (without SUDO)"
	@sh scripts/PostBuild.sh
packagePrepare: build
	mkdir -p packageData/bin
	mkdir -p packageData/include
	mkdir -p packageData/include/Package
	cp inc/* packageData/include/Package
	cp $(OUT) packageData/bin
	cd packageData && tar -czf data.tgz bin include
	rm -rf packageData/bin packageData/include
clean:
	@echo "Removing $(BIN) $(OUT)"
	@rm -rf $(BIN) $(OUT)