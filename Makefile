# Project Info
PROJECT = SimpleSpeedType
VERSION = 1.0
MAINTAINER = Prathamesh Dhake
DATA_FILE = data.txt

# Compilers
CC=cc
MINGW = x86_64-w64-mingw32-gcc

# Flags
CFLAGS = -Wall -O3
LINUX_FLAGS = -lncurses
WINDOWS_FLAGS = -static

# Directories
BUILD_DIR = build
PACKAGE_DIR = packages
DEB_DIR = $(PACKAGE_DIR)/$(PROJECT)-deb
WIN_DIR = $(PACKAGE_DIR)/$(PROJECT)-win

# Others
AARCH = amd64
DEP = libncurses-dev

.PHONY: all help linux windows deb win-zip clean

all: help

help:
	@echo "Available Targets"
	@echo "	make help: Display this message"
	@echo "	make linux: Creates a Linux Binary"
	@echo "	make windows: Creates a Windows Binary(.exe); Requires MinGW"
	@echo "	make deb: Creates a portable .deb"
	@echo "	make win-zip: Creates a portable zip for Windows"
	@echo "	make clean: Clean build artifacts"
	@echo ""

linux: $(BUILD_DIR)/$(PROJECT)

	$(BUILD_DIR)/$(PROJECT): speedtype.c
	@echo "Building Linux Binary"
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LINUX_FLAGS)
	@echo "Linux Binary created: $(BUILD_DIR)/$(PROJECT)"

windows: $(BUILD_DIR)/$(PROJECT).exe

	$(BUILD_DIR)/$(PROJECT).exe: speedtype.c
	@echo "Building Linux Binary"
	@mkdir -p $(BUILD_DIR)
	$(MINGW) $(CFLAGS) $< -o $@ $(WINDOWS_FLAGS)
	@echo "Windows Executable created: $(BUILD_DIR)/$(PROJECT).exe"

deb: linux

	@echo "Creating .deb"
	@mkdir -p $(DEB_DIR)/DEBIAN
	@mkdir -p $(DEB_DIR)/usr/bin
	@mkdir -p $(DEB_DIR)/etc/$(PROJECT)
	@mkdir -p $(DEB_DIR)/usr/share/doc/$(PROJECT)

	# Copy Binary
	cp $(BUILD_DIR)/$(PROJECT) $(DEB_DIR)/usr/bin/
	chmod 700 $(DEB_DIR)/usr/bin/$(PROJECT)

	# Copy Text File
	@if [ -f $(DATA_FILE) ]; then \
		cp $(DATA_FILE) $(DEB_DIR)/etc/$(PROJECT)/$(DATA_FILE); \
	else \
		echo "The quick brown fox jumps over the lazy dog." > $(DEB_DIR)/etc/$(PROJECT)/$(DATA_FILE); \
	fi

	# Copy README
	cp README.md $(DEB_DIR)/usr/share/doc/$(PROJECT)/README;

	# Create Control File
		@echo "Package: $(PROJECT)" > $(DEB_DIR)/DEBIAN/control
		@echo "Version: $(VERSION)" >> $(DEB_DIR)/DEBIAN/control
		@echo "Architecture: $(AARCH)" >> $(DEB_DIR)/DEBIAN/control
		@echo "Depends: $(DEP)" >> $(DEB_DIR)/DEBIAN/control
		@echo "Maintainer: $(MAINTAINER)" >> $(DEB_DIR)/DEBIAN/control
		@echo "Description: Terminal-based typing speed test" >> $(DEB_DIR)/DEBIAN/control
		@echo "Tests typing speed and accuracy with configurable time limits." >> $(DEB_DIR)/DEBIAN/control
		@echo "Edit /etc/$(PROJECT)/$(DATA_FILE) to customize the typing text." >> $(DEB_DIR)/DEBIAN/control

	# Mark $(DATA_FILE) as conffile
		@echo "/etc/$(PROJECT)/$(DATA_FILE)" > $(DEB_DIR)/DEBIAN/conffiles

	# Build Package
	@if ! commend -v dpkg-deb > /dev/null 2&>1; then \
		echo "ErrorL dpkg-deb not found. Install: sudo apt-get install dpkg-dev"; \
		exit 1; \
	fi

	dpkg-deb--build $(DEB_DIR)
	mv $(PACKAGE_DIR)/$(PROJECT)-deb.deb $(PACKAGE_DIR)/$(PROJECT)_$(VERSION)_$(AARCH).deb

	@echo ".deb created, at $(PACKAGE_DIR)/$(PROJECT)_$(VERSION)_$(AARCH).deb\n"
	@echo "Install: sudo dpkg -i $(PACKAGE_DIR)/$(PROJECT)_$(VERSION)_$(AARCH).deb"
	@echo "Edit text: sudo nano /etc/$(PROJECT)/$(DATA_FILE)"

win-zip: windows

	@echo "Creating Windows zip"
	@mkdir -p $(WIN_DIR)

	# Copy Binary
	cp $(BUILD_DIR)/$(PROJECT).exe $(WIN_DIR)/

	# Copy Text File
	@if [ -f $(DATA_FILE) ]; then \
		cp $(DATA_FILE) $(WIN_DIR)/; \
	else \
		echo "The quick brown fox jumps over the lazy dog." > $(WIN_DIR)/$(DATA_FILE); \
	fi

	# Copy README
	cp README.md $(WIN_DIR)/;

	# Create ZIP
	@cd $(PACKAGE_DIR) && zip -r $(PROJECT)-$(VERSION)-windows.zip $(PROJECT)-win/
	@echo "✓ Windows package created: $(PACKAGE_DIR)/$(PROJECT)-$(VERSION)-windows.zip"
	@echo "To Run, extract the zip and run the .exe file"
	@echo "Edit the contained data.txt file to modfiy the text"


clean:
	@echo "Cleaning Build Artifacts"
	rm -rf $(BUILD_DIR)
	rm -rf $(PACKAGE_DIR)
	@echo "Clean Completed."
