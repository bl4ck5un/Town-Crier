doc_examplesdir=$(docdir)/examples
dist_doc_examples_DATA=\
	Makefile.am\
	system.cpp\
	tcp-client.cpp\
	tcp-server.cpp\
	test-rpc.cpp\
	test-rpc.h\
	udp-client.cpp\
	udp-server.cpp

noinst_PROGRAMS=udp-client udp-server tcp-client tcp-server system

udp_client_SOURCES=udp-client.cpp test-rpc.cpp
tcp_client_SOURCES=tcp-client.cpp test-rpc.cpp
udp_server_SOURCES=udp-server.cpp test-rpc.cpp
tcp_server_SOURCES=tcp-server.cpp test-rpc.cpp
system_SOURCES=system.cpp



tcp_client_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp 
tcp_server_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp 
udp_client_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp 
udp_server_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp
system_LDADD=$(top_builddir)/src/libjsonrpc-cpp.la -ljsoncpp 

AM_CXXFLAGS=-std=c++98 -Wall -Wextra -pedantic -Wredundant-decls -Wshadow -O2 -Wno-long-long -Werror -I$(top_srcdir)/include

AM_CPPFLAGS=-I"@JSONCPP_INC_DIR@" -I"$(top_srcdir)/src"
if ENABLE_DEBUG 
   AM_CPPFLAGS+='-DDEBUG'
	 CXXFLAGS+=-g
endif
