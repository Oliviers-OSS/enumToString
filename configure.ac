dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(enumToString, 0.1)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_CONFIG_HEADER([config.h])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CXX
AC_C_INLINE
AC_FUNC_MALLOC
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])
AC_CHECK_LIB([expat], [XML_ParserCreate])
AC_CONFIG_FILES(Makefile)
AC_OUTPUT

