# GNU Convention, SHELL variables must be defined, preferably at first line
SHELL = /bin/bash

# Clean out .SUFFIXES first, then set the supported file type in this build
.SUFFIXES:
.SUFFIXES: .cpp .c .o

# Make the implicit rule explicitly spelled out here
.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(CFLAGS) -fPIC -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -D_POSIX -fPIC -o $@ $<

# Set the conventional program names to be used in this build
CXX = g++
CC = cc
INSTALL = /usr/bin/install
INSTALLPROGRAM := $(INSTALL)
INSTALLDATA := $(INSTALL) -m 644

# Set the conventional directory names to be used in this build
prefix ?= /usr/local
bindir = $(prefix)/bin
libdir = $(prefix)/lib



# The next line assumes "Current Directory" is where this Makefile is
MODULE_ROOT_DIR := $(CURDIR)
# Set the conventional directory name "srcdir", which should be used 
# when referring to a source file at all time 
srcdir := $(MODULE_ROOT_DIR)/src

# Special treatment for unconventional "debug" target
ifeq ($(MAKECMDGOALS), debug)
	CFLAGS += -g
	LDFLAGS += -g 
endif

# Append Macro definitions and Include PATH into CFLAGS
DEFS = 
CFLAGS += -O -Wall
CFLAGS += $(DEFS) -I. 
CFLAGS += -I$(MODULE_ROOT_DIR)/include
CFLAGS += -I$(MODULE_ROOT_DIR)/src


# Append Lib references and Linking PATH into LDFLAGS
LIBS += -lpthread
LDFLAGS += -Wall $(LIBS) 
LDFLAGS += -L. -L/usr/lib/x86_64-linux-gnu/ -L$(MODULE_ROOT_DIR)/lib
DBUSCFLAGS = $(shell pkg-config --cflags dbus-1)
DBUSLIBS = $(shell pkg-config --libs dbus-1)

CFLAGS += $(DBUSCFLAGS) -I/usr/include -I/usr/lib -I/usr/local/include -I$(MODULE_ROOT_DIR)/3rd/include/
LDFLAGS += $(DBUSLIBS) -L$(MODULE_ROOT_DIR)/3rd/lib/dbus

# Following are conventional targets to support, each pointing to a 
# module-specific target, which will be defined in the "include" file.
# Make sure each module-specific target name is unique by adding prefix.
# Make sure "all" is the first (default) target.



# "debug" target is the same as "all", with the DEBUG option already set in CFLAGS
debug: all

help:
	@echo '--------------------------------------------------------'
	@echo '           *** AVAILABLE TARGETS ARE: ***'
	@echo '--------------------------------------------------------'
	@echo 'all        Build everything (by default)'
	@echo 'debug      Build "all" with debug compilation options'
	@echo 'clean      Remove everything built by "all" or "debug"'
	@echo 'help       Print this help message.'
	@echo '--------------------------------------------------------'

.PHONY: all clean debug help

# Following line include/import the 'make' details of DbusAdapter module
include Makefile.include


