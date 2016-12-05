# esshader - simple shadertoy viewer for X & GLES2.
# See LICENSE file for copyright and license details.

include config.mk

SRC = esshader.c
OBJ = ${SRC:.c=.o}

all: options esshader

options:
	@echo esshader build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

esshader: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f esshader ${OBJ} esshader-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p esshader-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk ${SRC} esshader-${VERSION}
	@tar -cf esshader-${VERSION}.tar esshader-${VERSION}
	@gzip esshader-${VERSION}.tar
	@rm -rf esshader-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f esshader ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/esshader
	@chmod u+s ${DESTDIR}${PREFIX}/bin/esshader

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/esshader

.PHONY: all options clean dist install uninstall
