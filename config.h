/* See LICENSE file for copyright and license details. */

/*
 * Configuration attributes for EGL context. See eglChooseConfig.
*/
#include <getopt.h>


static const char options_string[] = "?fw:h:s:l0:1:2:3:x:y:";

static struct option long_options[] = {
    {"width", required_argument, 0, 'w'},
    {"height", required_argument, 0, 'h'},
    {"window_x", required_argument, 0, 'x'},
    {"window_y", required_argument, 0, 'y'},
    {"height", required_argument, 0, 'h'},
    {"fullscreen", no_argument, 0, 'f'},
    {"source", required_argument, 0, 's'},
    {"legacy", no_argument, 0, 'l'},
    {"texture0", required_argument, 0, '0'},
    {"texture1", required_argument, 0, '1'},
    {"texture2", required_argument, 0, '2'},
    {"texture3", required_argument, 0, '3'},
    {"help", no_argument, 0, '?'},
    {0, 0, 0, 0}
};

/*
 * Settings for OpenGL ES 2.0 with GLSL 100 (legacy) and OpenGL ES 3.0 with GLSL 300 ES
 */

// OpenGL ES 2.0 / GLSL 100

static const char common_shader_header_gles2[] =
    "#version 100\n"
    "precision highp float;";

static const char vertex_shader_body_gles2[] =
    "attribute vec4 iPosition;"
    "void main(){gl_Position=iPosition;}";

static const char fragment_shader_header_gles2[] =
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;" // legacy
    "uniform float iTime;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;\n";

static const char fragment_shader_footer_gles2[] =
    "\nvoid main(){mainImage(gl_FragColor,gl_FragCoord.xy);}";

// OpenGL ES 3.0 / GLSL 300 es

static const char common_shader_header_gles3[] =
    "#version 300 es\n"
    "precision highp float;\n";

static const char vertex_shader_body_gles3[] =
    "layout(location = 0) in vec4 iPosition;"
    "void main() {"
    "  gl_Position=iPosition;"
    "}\n";

static const char fragment_shader_header_gles3[] =
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;" // legacy
    "uniform float iTime;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;"
    "out vec4 fragColor;\n";

static const char fragment_shader_footer_gles3[] =
    "\nvoid main(){mainImage(fragColor, gl_FragCoord.xy);}";

/*
 * Standard ShaderToy Shader
 * https://www.shadertoy.com/new#
*/

static char *default_fragment_shader =
    "void mainImage( out vec4 fragColor, in vec2 fragCoord )"
    "{"
    "    vec2 uv = fragCoord/iResolution.xy;"
    "    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));"
    "    fragColor = vec4(col,1.0);"
    "}";
