## 
# JsonRpc-Cpp build file.
#

import sys;
import os;
import platform as pltfrm;

# Configure compiler arguments
cflags = ['-std=c++98', '-Wall', '-Wextra', '-pedantic', '-Wredundant-decls', '-Wshadow', '-Wno-long-long', '-Werror', '-O2', '-Iinclude'];
cpppath = [];
libpath = [];

# Command line parsing

# Build with debug symbols or not
if ARGUMENTS.get('mode', 0) == 'debug':
  cflags.append('-g');

# Installation directory
if ARGUMENTS.get('prefix', 0) != 0:
  install_dir =  ARGUMENTS.get('prefix', ''); 
else:
  if sys.platform == 'win32':
    install_dir = 'C:\\MinGW\\';
  else:
    install_dir = '/usr/local';

cpppath = [];
libpath = [];
linkflags = [];

platform = "default";

if sys.platform == 'win32':
  platform = "mingw";
  # Remove flags that cause compilation errors
  cflags.remove('-std=c++98'); #::swprintf and ::vswprintf has not been declared
  linkflags.append('-enable-auto-import');
  cpppath.append('-Ic:\\MinGW\\include');
else:
  cflags.append('-I/usr/include/jsoncpp');
  cflags.append('-I/usr/local/include/jsoncpp');

# Create an environment
env = Environment(ENV = os.environ.copy(), tools = [platform, "doxygen"], toolpath = ['.', './doc'], CXXFLAGS = cflags, CPPPATH = cpppath, LIBPATH = libpath, LINKFLAGS = linkflags);

# Sources and name of the JsonRpc-Cpp library
lib_target  = 'jsonrpc';

lib_sources = ['src/jsonrpc_handler.cpp',
               'src/jsonrpc_server.cpp',
               'src/jsonrpc_client.cpp',
               'src/jsonrpc_udpserver.cpp',
               'src/jsonrpc_tcpserver.cpp',
               'src/jsonrpc_udpclient.cpp',
               'src/jsonrpc_tcpclient.cpp',
               'src/netstring.cpp',
               'src/system.cpp',
               'src/networking.cpp'];

lib_includes = ['include/jsonrpc.h',
                'include/jsonrpc_handler.h',
                'include/jsonrpc_server.h',
                'include/jsonrpc_client.h',
                'include/jsonrpc_udpserver.h',
                'include/jsonrpc_tcpserver.h',
                'include/jsonrpc_udpclient.h',
                'include/jsonrpc_tcpclient.h',
                'include/jsonrpc_common.h',
                'include/netstring.h',
                'include/system.h',
                'include/networking.h'];

# Build libjsonrpc
libs = ['json'];

if env.WhereIs('curl') is not None:
  libs.append('curl');
  lib_includes.append('src/jsonrpc_httpclient.h');
  lib_sources.append('src/jsonrpc_httpclient.cpp');
  env['CXXFLAGS'].append('-DCURL_ENABLED')

# Add winsock library for MS Windows
if sys.platform == 'win32':
  libs.append('ws2_32');
else:
  libs.append('pthread');

libjsonrpc = env.SharedLibrary(target = lib_target, source = lib_sources, LIBS = libs);

# Build examples
examples_sources = ['examples/test-rpc.cpp', lib_sources];
udpserver_sources = ['examples/udp-server.cpp'];
tcpserver_sources = ['examples/tcp-server.cpp'];
udpclient_sources = ['examples/udp-client.cpp'];
tcpclient_sources = ['examples/tcp-client.cpp'];
system_sources = ['examples/system.cpp'];

examples_common = env.Object(examples_sources);
tcpserver = env.Program(target = 'examples/tcp-server', source = [tcpserver_sources, examples_common], LIBS = libs);
udpserver = env.Program(target = 'examples/udp-server', source = [udpserver_sources, examples_common], LIBS = libs);
tcpclient = env.Program(target = 'examples/tcp-client', source = [tcpclient_sources, examples_common], LIBS = libs);
udpclient = env.Program(target = 'examples/udp-client', source = [udpclient_sources, examples_common], LIBS = libs);
system_bin = env.Program(target = 'examples/system', source = [system_sources, examples_common], LIBS = libs);

# Build unit tests
test_common = env.Object(lib_sources);
unittest_sources = ['test/test-runner.cpp',
                    'test/test-core.cpp',
                    'test/test-system.cpp',
                    'test/test-netstring.cpp']

unittest = env.Program(target = 'test/test-runner', source = [unittest_sources, test_common], LIBS = [libs, 'cppunit']);

# Run unit tests
#
runtest = env.Command('runtest', None, os.path.join("test", "test-runner"));

# Install script
install = env.Install(dir = install_dir + "/lib/", source = libjsonrpc[0]);
install += env.Install(dir = install_dir + "/include/jsonrpc/", source = lib_includes);

# Doxygen
doxygen = env.Doxygen("Doxyfile");
Clean(doxygen, "doc/doxygen.pyc");
AlwaysBuild(doxygen);
env.Alias('doxygen', doxygen);

# Alias for targets
env.Alias('build', [libjsonrpc]);
env.Alias('examples', ['build', tcpserver, udpserver, tcpclient, udpclient, system_bin]);
env.Alias('install', install);
env.Alias('build-test', ['build', unittest]);
env.Alias('test', ['build-test', runtest]);
env.Alias('all', ['build', 'examples', 'doc', 'test']);

# Help documentation
Help("""
Type: 'scons build' to build JsonRpc-Cpp library,
      'scons install' to install shared library and include files on the system,
      'scons doc' to build documentation (doxygen),
      'scons build-test' to build unit tests,
      'scons test' to run unit tests,
      'scons all' to build everything and run unit tests,
      'scons -c' to cleanup object and shared library files,
      'scons -c install' to uninstall shared library and include files,
      'scons -c doc' to cleanup documentation files,
      'scons -c all' to cleanup everything.
      \n
      Default target when launching scons without arguments is 'scons build'.
""");

# Default target when running scons without arguments
Default('build');

