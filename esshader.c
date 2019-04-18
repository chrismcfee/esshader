/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include "config.h"

static const char common_shader_header[] =
    "#version 100\n"
    "precision highp float;";

static const char vertex_shader_body[] =
    "attribute vec4 iPosition;"
    "void main(){gl_Position=iPosition;}";

static const char fragment_shader_header[] =
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;\n";

static const char fragment_shader_footer[] =
    "\nvoid main(){mainImage(gl_FragColor,gl_FragCoord.xy);}";

static GLFWwindow *window;
static GLuint shader_program;
static GLint attrib_position;
static GLint sampler_channel[4];
static GLint uniform_cres;
static GLint uniform_ctime;
static GLint uniform_date;
static GLint uniform_gtime;
static GLint uniform_mouse;
static GLint uniform_res;
static GLint uniform_srate;

static void die(const char *format, ...){
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

static void info(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

static GLuint compile_shader(GLenum type, GLsizei nsources, const char **sources){
    GLuint shader;
    GLint success, len;
    GLsizei i, srclens[nsources];
    char *log;

    for (i = 0; i < nsources; ++i)
        srclens[i] = (GLsizei)strlen(sources[i]);

    shader = glCreateShader(type);
    glShaderSource(shader, nsources, sources, srclens);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            log = malloc(len);
            glGetShaderInfoLog(shader, len, NULL, log);
            fprintf(stderr, "%s\n\n", log);
            free(log);
        }
        die("Error compiling shader.\n");
    }

    return shader;
}


static void resize_viewport(GLFWwindow* window, int w, int h){
    glUniform3f(uniform_res, (float)w, (float)h, 0.0f);
    glViewport(0, 0, w, h);
    info("Setting window size to (%d,%d).\n", w, h);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, 1);
}

static void startup(int width, int height, bool fullscreen)
{
    GLuint vtx, frag;
    const char *sources[4];
    char* log;
    GLint success, len;

    if (!glfwInit())
        die("Unable to initialize GLFW.\n");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWmonitor *monitor = NULL;
    if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;
    }

    if (!(window = glfwCreateWindow(width, height, "esshader", monitor, NULL))) {
        glfwTerminate();
        die("Unable to create GLFW window.\n");
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, &key_callback);
    glfwSetFramebufferSizeCallback(window, &resize_viewport);

    sources[0] = common_shader_header;
    sources[1] = vertex_shader_body;
    vtx = compile_shader(GL_VERTEX_SHADER, 2, sources);

    sources[0] = common_shader_header;
    sources[1] = fragment_shader_header;
    sources[2] = default_fragment_shader;
    sources[3] = fragment_shader_footer;
    frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vtx);
    glAttachShader(shader_program, frag);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            log = malloc(len);
            glGetProgramInfoLog(shader_program, len, &len, log);
            fprintf(stderr, "%s\n\n", log);
            free(log);
        }
        die("Error linking shader program.\n");
    }

    glDeleteShader(vtx);
    glDeleteShader(frag);
    glReleaseShaderCompiler();

    glUseProgram(shader_program);
    glValidateProgram(shader_program);

    attrib_position = glGetAttribLocation(shader_program, "iPosition");
    sampler_channel[0] = glGetUniformLocation(shader_program, "iChannel0");
    sampler_channel[1] = glGetUniformLocation(shader_program, "iChannel1");
    sampler_channel[2] = glGetUniformLocation(shader_program, "iChannel2");
    sampler_channel[3] = glGetUniformLocation(shader_program, "iChannel3");
    uniform_cres = glGetUniformLocation(shader_program, "iChannelResolution");
    uniform_ctime = glGetUniformLocation(shader_program, "iChannelTime");
    uniform_date = glGetUniformLocation(shader_program, "iDate");
    uniform_gtime = glGetUniformLocation(shader_program, "iGlobalTime");
    uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
    uniform_res = glGetUniformLocation(shader_program, "iResolution");
    uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");

    resize_viewport(window, width, height);
}

static void shutdown(void){
    glfwTerminate();
}

static void render(float abstime){
    static const GLfloat vertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };

    if(uniform_gtime >= 0)
        glUniform1f(uniform_gtime, abstime);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//Reads a file into a string
//Return string or NULL on failure
static char* read_file_into_str(const char *filename) {
    long length = 0;
    char *result = NULL;
    FILE *file = fopen(filename, "r");
    if(file) {
        int status = fseek(file, 0, SEEK_END);
        if(status != 0) {
            fclose(file);
            return NULL;
        }
        length = ftell(file);
        status = fseek(file, 0, SEEK_SET);
        if(status != 0) {
            fclose(file);
            return NULL;
        }
        result = malloc((length+1) * sizeof(char));
        if(result) {
            size_t actual_length = fread(result, sizeof(char), length , file);
            result[actual_length++] = '\0';
        } 
        fclose(file);
        return result;
    }
    return NULL;
}

int main(int argc, char **argv){
    info("ESShader -  Version: %s\n", VERSION);

    //Default selected_options
    bool fullscreen = false;
    int window_width = 640;
    int window_height = 360;

    int temp_width = 0;
    int temp_height = 0;

    //shader program
    char *program_source = NULL;

    //Parse command line selected_options
    int selected_option = -1;
    int selected_index = 0;
    while((selected_option = getopt_long (argc, argv, options_string, long_options, &selected_index)) != -1) {
    switch(selected_option) {
        case 'f':
            fullscreen = true;
            break;
        case 'w':
            temp_width = atoi(optarg);
            if(temp_width > 0) {
                window_width = temp_width;
            }
            break;
        case 'h':
            temp_height = atoi(optarg);
            if(temp_height > 0) {
                window_height = temp_height;
            }
            break;
        case 's':
            info("Loading shader program: %s\n", optarg);
            program_source = read_file_into_str(optarg);
            if(program_source == NULL) {
                die("Could not read shader program %s\n", optarg);
            }
            default_fragment_shader = program_source;
            break;
        case '?':
            info(   "\nUsage: esshader [OPTIONS]\n"
                    "Example: esshader --width 1280 --height 720\n\n"
                    "Options:\n"
                    " -f, --fullscreen \truns the program in fullscreen mode.\n"
                    " -?, --help \t\tshows this help.\n"
                    " -w, --width [value] \tsets the window width to [value].\n"
                    " -h, --height [value] \tsets the window height to [value].\n"
                    " -s, --source [path] \tpath to shader program\n"
                    );
            return 0;
        }
    }

    info("Press [ESC] or [q] to exit.\n");
    info("Run with --help flag for more information.\n\n");
    startup(window_width, window_height, fullscreen);
    glfwSetTime(0.0);

    while (!glfwWindowShouldClose(window)) {
        render((float)glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shutdown();
    if(program_source != NULL) {
        free(program_source);
        program_source = NULL;
    }
    return 0;
}

