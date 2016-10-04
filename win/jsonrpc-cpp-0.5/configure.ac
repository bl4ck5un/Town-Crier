AC_INIT([jsonrpc-cpp], [0.5], [])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc subdir-objects dist-bzip2 dist-zip tar-ustar])

AM_PROG_AR
LT_INIT
#LT_OUTPUT

# Check for C++ compiler and use it to compile the tests.
#
AC_PROG_CXX
AC_PROG_INSTALL
AC_LANG([C++])
AC_CHECK_PROGS([DOXYGEN], [doxygen], [:])

# Configure options: --enable-debug[=no].
AC_ARG_ENABLE(	[debug],
		[AS_HELP_STRING([--enable-debug], [enable additional debug code paths (default is no)])],
		[debug="$withval"], [debug='no'])

AC_ARG_ENABLE(	[examples],
		[AS_HELP_STRING([--enable-examples], [install example code (default is yes)])],
		[examples="$withval"], [examples='yes'])

AC_ARG_ENABLE(	[doc],
		[AS_HELP_STRING([--enable-doc], [install documentation (default is yes)])],
		[doc="$withval"], [doc='yes'])



# Configure options: --with-json-cpp-inc-dir
AC_ARG_WITH(	[jsoncpp-inc-dir],
		[AS_HELP_STRING([--with-jsoncpp-inc-dir], [set jsoncpp include dir (default is /usr/include/jsoncpp)])],
		[JSONCPP_INC_DIR="$withval"], 
		[JSONCPP_INC_DIR='/usr/include/jsoncpp'])

AC_CHECK_LIB([curl], [curl_easy_init])
#AC_CHECK_LIB([jsoncpp], [])
# Allow the user to specify the pkgconfig directory.
#
#PKGCONFIG

# Check for doxygen program.
AM_CONDITIONAL([HAVE_DOXYGEN],[test "${DOXYGEN}" != ':'])
AM_CONDITIONAL([ENABLE_DEBUG],[test ${debug} == 'yes'])
AM_CONDITIONAL([INSTALL_DOCUMENTATION],[test "${doc}" == 'yes'])
AM_CONDITIONAL([INSTALL_EXAMPLES],[test "${examples}" == 'yes'])


# Output.
#
AC_SUBST([JSONCPP_INC_DIR]) 

AC_CONFIG_FILES(Makefile src/Makefile test/Makefile examples/Makefile doc/Makefile)
AC_OUTPUT


echo "
($PACKAGE_NAME) version $PACKAGE_VERSION
Prefix.......................:'${prefix}'
C++ Compiler.................: ${CXX} ${CXXFLAGS} ${CPPFLAGS}
Linker.......................: ${LD} ${LDFLAGS} ${LIBS}
JSON-C++ include directory...:'${JSONCPP_INC_DIR}'
Debug Build..................: ${debug}
Install Examples.............: ${examples}
Install Documentation........: ${doc}
Doxygen......................: ${DOXYGEN:-NONE}
"

