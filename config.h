/* See LICENSE file for copyright and license details. */

/*
 * Configuration attributes for EGL context. See eglChooseConfig.
*/


static const EGLint egl_config[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
};

/*
 * Star Nest by Pablo Román Andrioli
 * https://www.shadertoy.com/view/XlfGRj
 * This content is under the MIT License.
*/

static char *default_fragment_shader =
    "void mainImage(out vec4 fragColor, in vec2 fragCoord)"
    "{"
        "vec2 uv=fragCoord.xy/iResolution.xy;"
        "fragColor = vec4(uv, 0.5+0.5*sin(iGlobalTime), 1.0);"
    "}";

