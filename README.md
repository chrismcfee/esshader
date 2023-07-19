esshader - offline ShaderToy viewer
===========================================
Simple offline ShaderToy-compatible GLSL shader viewer.


Requirements:
-------------
In order to build esshader you will need the GLFW, OpenGL ES 2.0 development headers and SOIL installed on your system.  
The program can also be demanding on GPU hardware, so the more powerful GPU you have, the better.  


Installation:
-------------
Edit config.mk to match your local setup (the /usr/local prefix
is used by default).

Afterwards enter the following command to build and install
esshader (if necessary as root):

```
sudo make clean install
```

Running esshader
------------------
Simply invoke the 'esshader' command. To quit, press either
the Escape or Q key.
