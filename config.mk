# esshader version
VERSION = 0.1

# customize below to fit your system

# file paths
PREFIX = /usr/local

# includes and libs
INCS = -I.
LIBS = -lc -lm -lGLESv2 $(shell pkg-config --libs glfw3)

# toolchain flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -O2 ${INCS} ${CPPFLAGS} \
	$(pkg-config --cflags glfw3)
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
