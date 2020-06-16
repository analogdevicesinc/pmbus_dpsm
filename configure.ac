AC_PREREQ(2.61)

# package version number (not shared library version)
# odd micro numbers indicate in-progress development
# even micro numbers indicate released versions
m4_define(LT_PMBusTest_version_major, 0)
m4_define(LT_PMBusTest_version_minor, 1)
m4_define(LT_PMBusTest_version_micro, 0)

m4_define([LT_PMBusTest_version],
          [LT_PMBusTest_version_major.LT_PMBusTest_version_minor.LT_PMBusTest_version_micro])
m4_define([LT_PMBusTest_api_version],
          [LT_PMBusTest_version_major.LT_PMBusTest_version_minor])

AC_INIT(LT_PMBusTest, LT_PMBusTest_version)
AM_INIT_AUTOMAKE(LT_PMBusTest, LT_PMBusTest_version)
AC_CONFIG_HEADERS(config.h)

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_INSTALL

# enable pkg-config
PKG_PROG_PKG_CONFIG

# Checks for library functions.
#AC_CHECK_FUNCS([memset])

AC_SUBST(LT_PMBusApp_CFLAGS)
AC_SUBST(LT_PMBusApp_LIBS)


#icondir=${datadir}/icons/hicolor/32x32/apps
#AC_SUBST(icondir)

AC_OUTPUT([
Makefile
src/Makefile
])

echo ""
echo "   LT_PMBusTest $VERSION"
echo "   ====================="
echo ""
echo "   To build the project, run \"make\""
echo ""
