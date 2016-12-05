esshader - ShaderToy viewer for X & GLES2
===========================================
Simple offline ShaderToy compatible GLSL shader viewer for
X using OpenGL ES 2.0, inspired by the Suckless Philosophy.


Requirements:
-------------
In order to build esshader you will need the Xlib, EGL and
OpenGL ES 2.0 development headers installed on your system.
The program can also be demanding on GPU hardware, so the
more powerful GPU you have, the better.


Installation:
-------------
Edit config.mk to match your local setup (the /usr/local prefix
is used by default).

Afterwards enter the following command to build and install
esshader (if necessary as root):

    sudo make clean install


Running esshader
------------------
Simply invoke the 'esshader' command. To quit, press either
the Escape or Q key.
