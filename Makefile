CC = clang
CXX = clang++

# DEBUG build -g can balloon up the image size
# so for production build, maybe take it out
CXXFLAGS := -std=c++20 -O2 -g \
			-Werror \
			-Wall \
			-Weverything \
			-Wshadow \
			-Wno-c++98-compat \
			-Wno-c++98-compat-pedantic

# FIXME: DBUS support is a must for now, otherwise main doesn't work
# due to the way the main loop works.
DBUS_SUPPORT ?= 1

SRCS := src/macfanpp.cc \
	src/SMCObject.cc \
	src/FileUtils.cc \
	src/FanControl.cc

ifneq (${DBUS_SUPPORT}, 0)
SRCS += src/DBusAppIF.cc
# the C++ side is mostly our code, no longer need that many libraries
CXXFLAGS += $(shell pkg-config --cflags glibmm-2.4 giomm-2.4)
LDFLAGS += $(shell pkg-config --libs glibmm-2.4 giomm-2.4)
endif


OBJS := $(addsuffix .o,$(basename ${SRCS}))

.PHONY: all
all: macfanpp

.PHONY: clean
clean:
	rm -fr macfanpp ${OBJS} src/*.o

macfanpp: ${OBJS}
	$(CXX) $(LDFLAGS) -o $@ $^

