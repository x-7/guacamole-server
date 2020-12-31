AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4
AM_FLAGS = -Werror -Wall -pedantic

lib_LTLIBRARIES = libguac-client-ball.la

noinst_HEADERS = ball.h

libguac_client_ball_la_SOURCES = \
    ball.c

libguac_client_ball_la_CFLAGS = \
    -Werror -Wall -Iinclude           \
    @LIBGUAC_INCLUDE@

libguac_client_ball_la_LIBADD = \
    @COMMON_LTLIB@                    \
    @LIBGUAC_LTLIB@

libguac_client_ball_la_LDFLAGS = \
    -version-info 0:0:0 
