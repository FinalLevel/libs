AM_CONDITIONAL(NEED_MYSQL, test x$found_mysql = xyes)
AM_CONDITIONAL(NEED_OPENSSL, test x$found_openssl = xyes)
AM_CONDITIONAL(NEED_SQLITE, test x$WANT_SQLITE3 = xyes)
AM_CONDITIONAL(NEED_ICONV, test x$am_cv_func_iconv = xyes -o x$am_cv_lib_iconv = xyes)
AM_CONDITIONAL(NEED_PHONENUMBER, test x$found_phonenumber = xyes)
AC_CHECK_FUNC(lseek64, [], [CXXFLAGS+=' -DNO_LSEEK64'])
AC_CHECK_HEADER(sys/prctl.h, [], [CXXFLAGS+=' -DNO_SYS_PRCTL'])