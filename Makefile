CXX       	?= g++
TAR		?= bsdtar
PREFIX	  	?= /usr
VARS  	  	?=
CXXSTD		?= c++20

DEBUG 		?= 1

# https://stackoverflow.com/a/1079861
# WAY easier way to build debug and release builds
ifeq ($(DEBUG), 1)
        BUILDDIR  := build/debug
	LTO_FLAGS  = -fno-lto
        CXXFLAGS  := -ggdb3 -Wall -Wextra -pedantic -Wno-unused-parameter \
			-DDEBUG=1 -fno-omit-frame-pointer $(DEBUG_CXXFLAGS) $(CXXFLAGS)
        LDFLAGS	  += -fno-lto -Wl,-rpath,$(BUILDDIR)
else
	# Check if an optimization flag is not already set
	ifneq ($(filter -O%,$(CXXFLAGS)),)
    		$(info Keeping the existing optimization flag in CXXFLAGS)
	else
    		CXXFLAGS := -O3 $(CXXFLAGS)
	endif
	LDFLAGS   += $(LTO_FLAGS)
        BUILDDIR  := build/release
endif

NAME		 = cliboy
TARGET		?= $(NAME)
OLDVERSION	 = 0.0.1
VERSION    	 = 0.0.2-beta
SRC	 	 = $(wildcard src/*.cpp src/*/*.cpp)
OBJ	 	 = $(SRC:.cpp=.o)
LDFLAGS   	+= -L$(BUILDDIR)
LDLIBS		+= $(shell pkg-config --libs notcurses)
CXXFLAGS        += $(LTO_FLAGS) -fvisibility-inlines-hidden -fvisibility=hidden -Iinclude -std=$(CXXSTD) $(shell pkg-config --cflags notcurses) $(VARS) -DVERSION=\"$(VERSION)\"

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BUILDDIR)
	$(CXX) -o $(BUILDDIR)/$(TARGET) $(OBJ) $(LDFLAGS) $(LDLIBS)

dist: $(TARGET)
	zip -j $(NAME)-v$(VERSION).zip LICENSE README.md $(BUILDDIR)/$(TARGET)

clean:
	rm -rf $(BUILDDIR)/$(TARGET) $(OBJ)

distclean:
	rm -rf $(BUILDDIR) $(OBJ)
	find . -type f -name "*.tar.gz" -delete
	find . -type f -name "*.o" -delete
	find . -type f -name "*.a" -delete

updatever:
	sed -i "s#$(OLDVERSION)#$(VERSION)#g" $(wildcard .github/workflows/*.yml) compile_flags.txt
	sed -i "s#Project-Id-Version: $(NAME) $(OLDVERSION)#Project-Id-Version: $(NAME) $(VERSION)#g" po/*

.PHONY: $(TARGET) updatever distclean clean all
