#onvention, SHELL variable must be defined, preferably at first line
SHELL = /bin/bash

# Clean out .SUFFIXES first, then set the supported file type in this build
.SUFFIXES:
.SUFFIXES: .cpp .c .o

# Set the implicit rule here instead of the ".include" file, in case of multiple duplicate definition
.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -D_POSIX -o $@ $<

# Set the conventional program names to be used in this build
CXX = g++ -O
CC = cc

# The next line assumes "Current Directory" is where this Makefile is
MODULE_ROOT_DIR := $(CURDIR)

# Append Macro definitions and Include PATH into CFLAGS
DEFS = 
CFLAGS += -O -Wall $(DEFS) -I. 

# Append Lib references and Linking PATH into LDFLAGS
LIBS += -lpthread -ldbus_proxy -ldbus-1
LDFLAGS += -g -Wall $(LIBS) -L. 

# Use CANMORE_SDK/YT3RDPARTY path if defined, but avoid using 'pkg-config'
ifneq ($(CANMORE_SDK), )
        # override CC CXX to utilize Canmore SDK tool-chain
        CC = $(CANMORE_SDK)/bin/i686-cm-linux-gcc
        CXX = $(CANMORE_SDK)/bin/i686-cm-linux-g++
        CFLAGS += -I$(CANMORE_SDK)/include 
        LDFLAGS += -L$(CANMORE_SDK)/lib
endif

ifneq ($(YT3RDPARTY), )
        CFLAGS += -I$(YT3RDPARTY)/include -I$(YT3RDPARTY)/include/nspr -I$(YT3RDPARTY)/include/dbus-1.0 -I$(YT3RDPARTY)/lib/dbus-1.0/include 
        LDFLAGS += -L$(YT3RDPARTY)/lib
endif

ifneq ($(YFI_ROOT), )
        CFLAGS += -I$(YFI_ROOT)/include
        LDFLAGS += -L$(YFI_ROOT)/lib
endif


#CFLAGS += -I/home/lcuser/work/dbus_proxy/include 
#LDFLAGS += -L/home/lcuser/work/dbus_proxy/lib -Wl,-rpath-link=$(YT3RDPARTY)/lib

CFLAGS += -I../include 
LDFLAGS += -L../lib -Wl,-rpath-link=/usr/lib/x86_64-linux-gnu/

# Following are conventional targets to support, each pointing to a 
# module-specific target, which will be defined in the "include" file.
# Make sure each module-specific target name is unique by adding prefix.
# Make sure "all" is the first (default) target.


Dbus_signal_listener =          $(MODULE_ROOT_DIR)/bin/Dbus_signal_listener
Dbus_signal_sender =            $(MODULE_ROOT_DIR)/bin/Dbus_signal_sender
Dbus_method_call =              $(MODULE_ROOT_DIR)/bin/Dbus_method_call
Dbus_method_reg =               $(MODULE_ROOT_DIR)/bin/Dbus_method_reg


all: $(Dbus_signal_listener) $(Dbus_signal_sender) $(Dbus_method_reg) $(Dbus_method_call)


$(Dbus_signal_listener): $(MODULE_ROOT_DIR)/dbus_signal_listener.o
	@mkdir -p $(MODULE_ROOT_DIR)/bin
	$(CXX) -ldl -ldbus_proxy -o $@ $^  $(LDFLAGS) 

$(Dbus_signal_sender): $(MODULE_ROOT_DIR)/dbus_signal_sender.o
	@mkdir -p $(MODULE_ROOT_DIR)/bin
	$(CXX) -ldl -ldbus_proxy -o $@ $^  $(LDFLAGS) 
        
$(Dbus_method_call): $(MODULE_ROOT_DIR)/dbus_method_call.o
	@mkdir -p $(MODULE_ROOT_DIR)/bin 
	$(CXX) -ldl -ldbus_proxy -o $@ $^  $(LDFLAGS) 

$(Dbus_method_reg): $(MODULE_ROOT_DIR)/dbus_method_reg.o
	@mkdir -p $(MODULE_ROOT_DIR)/bin
	$(CXX) -ldl -ldbus_proxy -o $@ $^  $(LDFLAGS) 

clean:
	@rm -f $(MODULE_ROOT_DIR)/*.o
	@rm -rf $(MODULE_ROOT_DIR)/bin
        

.PHONY: all clean

