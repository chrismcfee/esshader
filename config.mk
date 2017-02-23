# esshader version
VERSION = 0.1

# customize below to fit your system

# file paths
PREFIX = /usr/local
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC}
LIBS = -L/usr/lib -lc -lm -L${X11LIB} -lX11 -lEGL -lGLESv2

# toolchain flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_POSIX_C_SOURCE=200112L
CFLAGS = -std=c99 -pedantic -Wall -O3 ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
