AM_CONDITIONAL(NEED_MYSQL, test x$found_mysql = xyes)
AM_CONDITIONAL(NEED_OPENSSL, test x$found_openssl = xyes)
AM_CONDITIONAL(NEED_SQLITE, test x$WANT_SQLITE3 = xyes)