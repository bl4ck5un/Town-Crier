lib_LTLIBRARIES = libjsonrpc-cpp.la
libjsonrpc_cpp_la_SOURCES = \
	jsonrpc_handler.cpp\
	jsonrpc_server.cpp\
	jsonrpc_client.cpp\
	jsonrpc_udpserver.cpp\
	jsonrpc_tcpserver.cpp\
	jsonrpc_udpclient.cpp\
	jsonrpc_tcpclient.cpp\
	netstring.cpp\
	system.cpp\
	networking.cpp

libjsonrpc_cpp_la_INCLUDES=\
	../include/jsonrpc.h\
	../include/jsonrpc_handler.h\
	../include/jsonrpc_server.h\
	../include/jsonrpc_client.h\
	../include/jsonrpc_udpserver.h\
	../include/jsonrpc_tcpserver.h\
	../include/jsonrpc_udpclient.h\
	../include/jsonrpc_tcpclient.h\
	../include/jsonrpc_common.h\
	../include/jsonrpc_httpclient.h\
	../include/netstring.h\
	../include/system.h\
	../include/networking.h

include_jsonrpc_cppdir=$(includedir)/jsonrpc-cpp
include_jsonrpc_cpp_HEADERS=$(libjsonrpc_cpp_la_INCLUDES)

libjsonrpc_cpp_la_LDFLAGS = -release @PACKAGE_VERSION@ -ljsoncpp -lpthread

AM_CXXFLAGS=-std=c++98 -Wall -Wextra -pedantic -Wredundant-decls -Wshadow -O2 -Wno-long-long -Werror -I$(top_srcdir)/include

AM_CPPFLAGS=-I"@JSONCPP_INC_DIR@"
if ENABLE_DEBUG 
	AM_CPPFLAGS+='-DDEBUG'
endif
