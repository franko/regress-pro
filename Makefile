##############################################################################
# Regress Pro top level Makefile for installation. Requires GNU Make.
#
# Copyright (C) 2005-2015 Francesco Abbate
##############################################################################

PACKAGE_NAME = regress-pro
VERSION = 2.0.2
DEBIAN_BUILD_DIR = debian_build
DEB_ARCH := $(shell dpkg-architecture -qDEB_HOST_ARCH)
DEBIAN_PACKAGE := $(PACKAGE_NAME)_$VERSION-1_$(DEB_ARCH).deb

HOST_RM = rm -f

default all:
	$(MAKE) -C src
	$(MAKE) -C fox-gui

debian: $(DEBIAN_PACKAGE)

$(DEBIAN_PACKAGE): $(EFIT_LIB) fox-gui debian/control
	$(HOST_RM) -r $(DEBIAN_BUILD_DIR)
	mkdir -p $(DEBIAN_BUILD_DIR)/usr/bin $(DEBIAN_BUILD_DIR)/usr/share/applications $(DEBIAN_BUILD_DIR)/usr/share/icons/hicolor/128x128/apps $(DEBIAN_BUILD_DIR)/usr/share/regress-pro
	strip fox-gui/regress$(EXE)
	cp fox-gui/regress$(EXE) $(DEBIAN_BUILD_DIR)/usr/bin
	cp -R examples $(DEBIAN_BUILD_DIR)/usr/share/regress-pro
	cp src/regress-pro-128x128.png $(DEBIAN_BUILD_DIR)/usr/share/icons/hicolor/128x128/apps/regress-pro.png
	fakeroot bash debian/build.sh $(PACKAGE_NAME) $(VERSION)

clean:
	$(MAKE) -C src clean
	$(MAKE) -C fox-gui clean
	$(HOST_RM) -r $(DEBIAN_BUILD_DIR)
	$(HOST_RM) $(DEBIAN_PACKAGE)

.PHONY: all debian clean
