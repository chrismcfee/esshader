/* See LICENSE file for copyright and license details. */

/*
 * Configuration attributes for EGL context. See eglChooseConfig.
*/

char title_string[] = "ESShader - Version %s";

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

static const char default_fragment_shader[] =
    "#define iterations 17\n"
    "#define formuparam 0.53\n"
    "#define volsteps 20\n"
    "#define stepsize 0.1\n"
    "#define zoom   0.800\n"
    "#define tile   0.850\n"
    "#define speed  0.010\n"
    "#define brightness 0.0015\n"
    "#define darkmatter 0.300\n"
    "#define distfading 0.730\n"
    "#define saturation 0.850\n"
    "void mainImage(out vec4 fragColor, in vec2 fragCoord)"
    "{"
        "vec2 uv=fragCoord.xy/iResolution.xy-.5;"
        "uv.y*=iResolution.y/iResolution.x;"
        "vec3 dir=vec3(uv*zoom,1.);"
        "float time=iGlobalTime*speed+.25;"
        "float a1=.5+iMouse.x/iResolution.x*2.;"
        "float a2=.8+iMouse.y/iResolution.y*2.;"
        "mat2 rot1=mat2(cos(a1),sin(a1),-sin(a1),cos(a1));"
        "mat2 rot2=mat2(cos(a2),sin(a2),-sin(a2),cos(a2));"
        "dir.xz*=rot1;"
        "dir.xy*=rot2;"
        "vec3 from=vec3(1.,.5,0.5);"
        "from+=vec3(time*2.,time,-2.);"
        "from.xz*=rot1;"
        "from.xy*=rot2;"
        "float s=0.1,fade=1.;"
        "vec3 v=vec3(0.);"
        "for (int r=0; r<volsteps; r++) {"
            "vec3 p=from+s*dir*.5;"
            "p = abs(vec3(tile)-mod(p,vec3(tile*2.)));"
            "float pa,a=pa=0.;"
            "for (int i=0; i<iterations; i++) {"
                "p=abs(p)/dot(p,p)-formuparam;"
                "a+=abs(length(p)-pa);"
                "pa=length(p);"
            "}"
            "float dm=max(0.,darkmatter-a*a*.001);"
            "a*=a*a;"
            "if (r>6) fade*=1.-dm;"
            "v+=vec3(dm,dm*.5,0.);"
            "v+=fade;"
            "v+=vec3(s,s*s,s*s*s*s)*a*brightness*fade;"
            "fade*=distfading;"
            "s+=stepsize;"
        "}"
        "v=mix(vec3(length(v)),v,saturation);"
        "fragColor = vec4(v*.01,1.);"
    "}";

