TARGET    = xdkbuild
SRCDIR    = src
CXX       = g++
CXXFLAGS  = -Isrc/include/ -Icryptopp/ -g3 -O3 -DNDEBUG -Wall -Wextra -std=c++2a
LDFLAGS   = -l:libcryptopp.a
SRCS      = $(foreach dir,$(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJS      = $(SRCS:.cpp=.o)

platform_id != uname -s

platform != if [ $(platform_id) = Linux ] || \
    [ $(platform_id) = FreeBSD ] || \
    [ $(platform_id) = OpenBSD ] || \
    [ $(platform_id) = NetBSD ] || \
    [ $(platform_id) = Darwin ]; then \
        echo $(platform_id); \
    else \
        echo Unrecognized; \
    fi

ifeq ($(platform),FreeBSD)
    CXXFLAGS   += -D_FILE_OFFSET_BITS=64
endif
ifeq ($(platform),Darwin)
    CXXFLAGS   += -D_FILE_OFFSET_BITS=64
endif

release: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	rm $(TARGET) $(OBJS)
