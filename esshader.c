/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>

#include "config.h"


// Active GLES/GLSL version (default to 300 ES for GLES3)

static const char* common_shader_header = common_shader_header_gles3;
static const char* vertex_shader_body = vertex_shader_body_gles3;
static const char* fragment_shader_header = fragment_shader_header_gles3;
static const char* fragment_shader_footer = fragment_shader_footer_gles3;
static int gles_major = 3;
static int gles_minor = 0;

static GLFWwindow *window;
static GLuint shader_program;
static GLint attrib_position;
static GLint sampler_channel[4];
static GLint uniform_cres;
static GLint uniform_ctime;
static GLint uniform_date;
static GLint uniform_gtime;
static GLint uniform_time;
static GLint uniform_mouse;
static GLint uniform_res;
static GLint uniform_srate;

static GLfloat viewportSizeX = 0.0;
static GLfloat viewportSizeY = 0.0;
static GLfloat mouseX = 0.0;
static GLfloat mouseY = 0.0;
static GLfloat mouseLPressed = 0.0;
static GLfloat mouseRPressed = 0.0;

static int mouseUpdating = 0;
static int maximized = 0;

// what iChannel to bind the virtual keyboard to (argument -k)
static int bindKeyboard = -1;
// for storing the keyboard state in a ST-style texture
static unsigned char keyStateTextureData[256 * 3];
static GLuint keyStateTextureID;
// for translating to JS key codes
static int keyLookup[GLFW_KEY_LAST + 1];

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

static void select_gles2() {
    info("Selected OpenGL ES 2.0 / GLSL 100\n");
    common_shader_header = common_shader_header_gles2;
    vertex_shader_body = vertex_shader_body_gles2;
    fragment_shader_header = fragment_shader_header_gles2;
    fragment_shader_footer = fragment_shader_footer_gles2;
    gles_major = 2;
    gles_minor = 0;
}

static void select_gles3() {
    info("Selected OpenGL ES 3.0 / GLSL 300 es\n");
    common_shader_header = common_shader_header_gles3;
    vertex_shader_body = vertex_shader_body_gles3;
    fragment_shader_header = fragment_shader_header_gles3;
    fragment_shader_footer = fragment_shader_footer_gles3;
    gles_major = 3;
    gles_minor = 0;
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
    viewportSizeX = w;
    viewportSizeY = h;
}


static void populate_key_lookup() {
    // Convert GLFW key codes to Shadertoy / JS
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        keyLookup[i] = -1;
    }
    for (int i = GLFW_KEY_0; i <= GLFW_KEY_9; ++i) {
        keyLookup[i] = i;
    }
    for (int i = GLFW_KEY_A; i <= GLFW_KEY_Z; ++i) {
        keyLookup[i] = i;
    }
    for (int i = GLFW_KEY_F1; i <= GLFW_KEY_F12; ++i) {
        keyLookup[i] = 112 + i - GLFW_KEY_F1;
    }
    for (int i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; ++i) {
        keyLookup[i] = 96 + i - GLFW_KEY_KP_0;
    }
    keyLookup[GLFW_KEY_SPACE]         = 32;
    keyLookup[GLFW_KEY_APOSTROPHE]    = 222;
    keyLookup[GLFW_KEY_COMMA]         = 188;
    keyLookup[GLFW_KEY_MINUS]         = 173;
    keyLookup[GLFW_KEY_PERIOD]        = 190;
    keyLookup[GLFW_KEY_SLASH]         = 191;
    keyLookup[GLFW_KEY_SEMICOLON]     = 59;
    keyLookup[GLFW_KEY_EQUAL]         = 61;
    keyLookup[GLFW_KEY_LEFT_BRACKET]  = 219;
    keyLookup[GLFW_KEY_BACKSLASH]     = 220;
    keyLookup[GLFW_KEY_RIGHT_BRACKET] = 221;
    keyLookup[GLFW_KEY_GRAVE_ACCENT]  = 192;
    keyLookup[GLFW_KEY_ESCAPE]        = 27;
    keyLookup[GLFW_KEY_ENTER]         = 13;
    keyLookup[GLFW_KEY_TAB]           = 9;
    keyLookup[GLFW_KEY_BACKSPACE]     = 8;
    keyLookup[GLFW_KEY_INSERT]        = 45;
    keyLookup[GLFW_KEY_DELETE]        = 46;
    keyLookup[GLFW_KEY_RIGHT]         = 39;
    keyLookup[GLFW_KEY_LEFT]          = 37;
    keyLookup[GLFW_KEY_DOWN]          = 40;
    keyLookup[GLFW_KEY_UP]            = 38;
    keyLookup[GLFW_KEY_PAGE_UP]       = 33;
    keyLookup[GLFW_KEY_PAGE_DOWN]     = 34;
    keyLookup[GLFW_KEY_HOME]          = 36;
    keyLookup[GLFW_KEY_END]           = 35;
    keyLookup[GLFW_KEY_CAPS_LOCK]     = 20;
    keyLookup[GLFW_KEY_SCROLL_LOCK]   = 145;
    keyLookup[GLFW_KEY_NUM_LOCK]      = 144;
    keyLookup[GLFW_KEY_PRINT_SCREEN]  = 42;
    keyLookup[GLFW_KEY_PAUSE]         = 19;
    keyLookup[GLFW_KEY_KP_DECIMAL]    = 110;
    keyLookup[GLFW_KEY_KP_DIVIDE]     = 111;
    keyLookup[GLFW_KEY_KP_MULTIPLY]   = 106;
    keyLookup[GLFW_KEY_KP_SUBTRACT]   = 109;
    keyLookup[GLFW_KEY_KP_ADD]        = 107;
    keyLookup[GLFW_KEY_KP_ENTER]      = 13;
    keyLookup[GLFW_KEY_LEFT_SHIFT]    = 16;
    keyLookup[GLFW_KEY_LEFT_CONTROL]  = 17;
    keyLookup[GLFW_KEY_LEFT_ALT]      = 18;
    keyLookup[GLFW_KEY_LEFT_SUPER]    = 91;
    keyLookup[GLFW_KEY_RIGHT_SHIFT]   = 16;
    keyLookup[GLFW_KEY_RIGHT_CONTROL] = 17;
    keyLookup[GLFW_KEY_RIGHT_ALT]     = 225;
    keyLookup[GLFW_KEY_RIGHT_SUPER]   = 91;
    keyLookup[GLFW_KEY_MENU]          = 93;
}

static void update_keystate() {
    if (bindKeyboard >= 0) {
        glActiveTexture(GL_TEXTURE0 + bindKeyboard);
        glBindTexture(GL_TEXTURE_2D, keyStateTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 3, 0, GL_RED, GL_UNSIGNED_BYTE, keyStateTextureData);
        glUniform1i(sampler_channel[bindKeyboard], bindKeyboard);
    }
}

static void reset_keystate() {
    memset(keyStateTextureData, 0, sizeof(keyStateTextureData));
}

static void partially_reset_keystate() {
    // Reset 'just pressed' (= rising edge key event) flags
    for (int i=0; i<256; ++i) {
        keyStateTextureData[i + 256] = 0;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        int js = keyLookup[key];
        if (js >= 0) {
            keyStateTextureData[js + 0] = 255;
            keyStateTextureData[js + 256] = 255;
            keyStateTextureData[js + 512] = keyStateTextureData[js + 512] ^ 0xff;
        }
        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, 1);
        } else if (key == GLFW_KEY_F) {
            if (maximized) {
                glfwRestoreWindow(window);
                maximized = 0;
            } else {
                glfwMaximizeWindow(window);
                maximized = 1;
            }
        }
    } else if (action == GLFW_RELEASE) {
        int js = keyLookup[key];
        if (js >= 0) {
            keyStateTextureData[js + 0] = 0;
            keyStateTextureData[js + 256] = 0;
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    mouseX = xpos;
    mouseY = viewportSizeY - ypos;
    if (mouseUpdating) {
        glUniform4f(uniform_mouse, mouseX, mouseY, mouseLPressed, mouseRPressed);
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    printf("button %d action %d\n", button, action);
    if (button == 0) {
        mouseUpdating = action;
        mouseLPressed = action;
    } else if (button == 1) {
        mouseRPressed = action;
    }
    glUniform4f(uniform_mouse, mouseX, mouseY, mouseLPressed, mouseRPressed);
}

static void startup(int width, int height, int window_x, int window_y, bool fullscreen)
{
    GLuint vtx, frag;
    const char *sources[4];
    char* log;
    GLint success, len;

    if (!glfwInit())
        die("Unable to initialize GLFW.\n");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gles_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gles_minor);

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

    if (window_x >= 0 && window_y >= 0) {
        glfwSetWindowPos(window, window_x, window_y);
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, &cursor_position_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
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
    uniform_time = glGetUniformLocation(shader_program, "iTime");
    uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
    uniform_res = glGetUniformLocation(shader_program, "iResolution");
    uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");

    resize_viewport(window, width, height);
}

static void load_images_and_bind(const char* filenames[]) {
    for (int i=0; i<4; ++i) {
        if (filenames[i]) {
            if (sampler_channel[i] < 0) {
                info("Skipping image file for unused iChannel%d: %s\n", i, filenames[i]);
            } else {
                info("Loading image file for iChannel%d: %s\n", i, filenames[i]);
                GLuint tex_2d = SOIL_load_OGL_texture (
                        filenames[i],
                        SOIL_LOAD_AUTO,
                        SOIL_CREATE_NEW_ID,
                        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
                );
                if (!tex_2d) {
                    die("SOIL image library error: %s\n", SOIL_last_result());
                }
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, tex_2d);
                glUniform1i(sampler_channel[i], i);
            }
        }
    }
    glActiveTexture(GL_TEXTURE0);
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
    if(uniform_time >= 0)
        glUniform1f(uniform_time, abstime);

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
    int window_x = -1;
    int window_y = -1;

    int temp_val = 0;

    int legacy = 0;

    const char* texture_files[4];
    for (int i=0; i<4; ++i) {
        texture_files[i] = NULL;
    }

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
            temp_val = atoi(optarg);
            if(temp_val > 0) {
                window_width = temp_val;
            }
            break;
        case 'h':
            temp_val = atoi(optarg);
            if(temp_val > 0) {
                window_height = temp_val;
            }
            break;
        case 'x':
            temp_val = atoi(optarg);
            if(temp_val >= 0) {
                window_x = temp_val;
            }
            break;
        case 'y':
            temp_val = atoi(optarg);
            if(temp_val >= 0) {
                window_y = temp_val;
            }
            break;
        case 'k':
            temp_val = atoi(optarg);
            if(temp_val >= 0 && temp_val < 4) {
                bindKeyboard = temp_val;
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
        case 'l':
            legacy = 1;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
            texture_files[selected_option - '0'] = optarg;
            break;
        case '?':
            info(   "\nUsage: esshader [OPTIONS]\n"
                    "Example: esshader --width 1280 --height 720\n\n"
                    "Options:\n"
                    " -f, --fullscreen \truns the program in fullscreen mode.\n"
                    " -?, --help \t\tshows this help.\n"
                    " -w, --width <value> \tsets the window width to <value>.\n"
                    " -h, --height <value> \tsets the window height to <value>.\n"
                    " -x, --window_x <value> sets the window X position to <value>.\n"
                    " -y, --window_y <value> sets the window Y position to <value>.\n"
                    " -s, --source <path> \tpath to shader program\n"
                    " -l, --legacy \t\tlegacy mode (= GLES2 with GLSL 100)\n"
                    " -0, --texture0 <path> \tload texture for iChannel0\n"
                    " -1, --texture1 <path> \tload texture for iChannel1\n"
                    " -2, --texture2 <path> \tload texture for iChannel2\n"
                    " -3, --texture3 <path> \tload texture for iChannel3\n"
                    " -k, --keyboard <ch> \tbind keyboard to iChannel<ch>\n"
                    );
            return 0;
        }
    }

    if (bindKeyboard >= 0) {
        if (texture_files[bindKeyboard] != NULL) {
            die("Error: cannot bind texture and keyboard to the same channel.\n");
        } else {
            info("Dynamic keyboard texture will be bound to iChannel%d.\n", bindKeyboard);
            glGenTextures(1, &keyStateTextureID);
        }
    }
    if (legacy) {
        select_gles2();
    } else {
        select_gles3();
    }
    populate_key_lookup();
    reset_keystate();
    startup(window_width, window_height, window_x, window_y, fullscreen);
    load_images_and_bind(texture_files);

    info("Press [ESC] or [q] to exit.\n");
    info("Run with --help flag for more information.\n\n");

    glfwSetTime(0.0);
    while (!glfwWindowShouldClose(window)) {
        update_keystate();
        render((float)glfwGetTime());
        glfwSwapBuffers(window);
        partially_reset_keystate();
        glfwPollEvents();
    }

    shutdown();
    if(program_source != NULL) {
        free(program_source);
        program_source = NULL;
    }
    return 0;
}

