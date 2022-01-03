
# main makefile

CC ?= gcc
CXX ?= g++
LD := $(CXX)
RM=rm -f

# release flags:
#CFLAGS=-O3 -DNDEBUG
#CXXFLAGS=-O3 -DNDEBUG
#LDFLAGS=-O3
# debug flags:
CFLAGS=-g3 -O0 -Wall -Wextra -Werror -Wno-unused-parameter
CXXFLAGS=-g3 -O0 -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS=

# the rest
CFLAGS := $(CFLAGS) -MMD -MP
CXXFLAGS := -std=c++20 $(CXXFLAGS) -MMD -MP `wx-config --cflags`
LDFLAGS := $(LDFLAGS) `wx-config --libs std,aui`
LDLIBS=-lm
OBJS=
SUBDIRS=common file app ui/dialogs ui/settings ui

TARGET := ../hexbed
IROOT := .

CXXFLAGS := -I$(IROOT) $(CXXFLAGS)

include $(addsuffix /Makefile.inc, $(SUBDIRS))

DEPS := $(OBJS:.o=.d)

default: all

.PHONY: all clean
all: $(TARGET)
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)
