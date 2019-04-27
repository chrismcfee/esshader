/* See LICENSE file for copyright and license details. */

/*
 * Configuration attributes for EGL context. See eglChooseConfig.
*/
#include <getopt.h>


static const char options_string[] = "?fw:h:s:";

static struct option long_options[] = {
    {"width", required_argument, 0, 'w'},
    {"height", required_argument, 0, 'h'},
    {"fullscreen", no_argument, 0, 'f'},
    {"source", required_argument, 0, 's'},
    {"help", no_argument, 0, '?'},
    {0, 0, 0, 0}
};

/*
 * Standard ShaderToy Shader
 * https://www.shadertoy.com/new#
*/

static char *default_fragment_shader =
    "void mainImage(out vec4 fragColor, in vec2 fragCoord)"
    "{"
        "vec2 uv=fragCoord.xy/iResolution.xy;"
        "fragColor = vec4(uv, 0.5+0.5*sin(iGlobalTime), 1.0);"
    "}";

