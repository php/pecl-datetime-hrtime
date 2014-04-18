dnl $Id$
dnl config.m4 for extension hrtime

dnl Otherwise use enable:

PHP_ARG_ENABLE(hrtime, whether to enable hrtime support,
[  --enable-hrtime           Enable hrtime support])

if test "$PHP_HRTIME" != "no"; then
  PHP_ADD_LIBRARY(rt)
  PHP_NEW_EXTENSION(hrtime, hrtime.c timer.c, $ext_shared)
fi
