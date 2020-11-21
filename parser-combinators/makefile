# makefile

OUTPUT      := build/z86

CSRC        += $(shell find source -iname "*.c")
CXXSRC      += $(shell find source -iname "*.cpp")

COBJ        = $(CSRC:.c=.c.o)
CDEPS       = $(COBJ:.o=.d)

CXXOBJ      = $(CXXSRC:.cpp=.cpp.o)
CXXDEPS     = $(CXXOBJ:.o=.d)

CFLAGS      := -std=c11
CXXFLAGS    := -std=c++17 -fno-exceptions

DEFINES     =
WARNINGS    = -Wall -Wextra -Werror=return-type -Wno-unused-parameter -Wno-unused-private-field

PRECOMP_HDR = include/precompile.h
PRECOMP_GCH = $(PRECOMP_HDR:.h=.h.gch)

INCLUDES    = -Iinclude


.DEFAULT_GOAL = all

.PHONY: all, ini, json
.PRECIOUS: $(PRECOMP_GCH)

tests/ini/test_ini: tests/ini/test_ini.cpp $(shell find include -iname "*.h")
	@$(CXX) $(CXXFLAGS) -g $(WARNINGS) $(DEFINES) $(INCLUDES) -o $@ $<

tests/json/test_json: tests/json/test_json.cpp $(shell find include -iname "*.h")
	@$(CXX) $(CXXFLAGS) -g $(WARNINGS) $(DEFINES) $(INCLUDES) -o $@ $<

ini: tests/ini/test_ini
	@tests/ini/test_ini tests/ini/test.ini

json: tests/json/test_json
	@tests/json/test_json tests/json/test.json

all: ini json

clean:
	@find . -name "*.o" -delete
	@find . -name "*.h.d" -delete
	@find . -name "*.cpp.d" -delete
	@find . -name "*.h.gch" -delete
	@rm -f tests/ini/test_ini








