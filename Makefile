
# DEBUG build -g can baloon up the image size
# so for production build, maybe take it out
CXXFLAGS := -std=c++20 -O2 -g \
			-Werror \
			-Wall \
			-Weverything \
			-Wshadow \
			-Wno-c++98-compat \
			-Wno-c++98-compat-pedantic

SRCS := src/macfanpp.cc
OBJS := $(addsuffix .o,$(basename ${SRCS}))

.PHONY: all
all: macfanpp

.PHONY: clean
clean:
	rm -fr macfanpp ${OBJS}

macfanpp: ${OBJS}
	$(CXX) $(LDFLAGS) -o $@ $^

