check_PROGRAMS = test-runner 
TESTS = $(check_PROGRAMS) 

test_runner_SOURCES = \
	test-runner.cpp\
	test-core.cpp\
	test-system.cpp\
	test-netstring.cpp

test_runner_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp -lcppunit

AM_CXXFLAGS=-std=c++03 -Wall -Wextra -pedantic -Wredundant-decls -Wshadow -O2 -Wno-long-long -Werror -I$(top_srcdir)/include

AM_CPPFLAGS=-I"@JSONCPP_INC_DIR@"
if ENABLE_DEBUG 
   AM_CPPFLAGS+='-DDEBUG'
endif
