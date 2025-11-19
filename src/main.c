// main.c - OpenGL + GLFW + GLAD + linmath - kamera FPS
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../external/linmath/linmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ================== STRUKTURY I DANE GEOMETRII ==================

typedef struct
{
    float x, y, z;
    float r, g, b;
} Vertex;

// Rozmiar sześcianu (krawędź 1.0, środek w (0,0,0))
static const float S = 0.5f;

/**
 * 24 wierzchołki – każdy bok ma swój kolor
 */
static const Vertex cube_vertices[] = {
    // +X czerwony
    {+S, -S, -S, 1, 0, 0},
    {+S, +S, -S, 1, 0, 0},
    {+S, +S, +S, 1, 0, 0},
    {+S, -S, +S, 1, 0, 0},
    // -X zielony
    {-S, -S, +S, 0, 1, 0},
    {-S, +S, +S, 0, 1, 0},
    {-S, +S, -S, 0, 1, 0},
    {-S, -S, -S, 0, 1, 0},
    // +Y niebieski
    {-S, +S, -S, 0, 0, 1},
    {+S, +S, -S, 0, 0, 1},
    {+S, +S, +S, 0, 0, 1},
    {-S, +S, +S, 0, 0, 1},
    // -Y żółty
    {-S, -S, +S, 1, 1, 0},
    {+S, -S, +S, 1, 1, 0},
    {+S, -S, -S, 1, 1, 0},
    {-S, -S, -S, 1, 1, 0},
    // +Z magenta
    {-S, -S, +S, 1, 0, 1},
    {-S, +S, +S, 1, 0, 1},
    {+S, +S, +S, 1, 0, 1},
    {+S, -S, +S, 1, 0, 1},
    // -Z cyjan
    {+S, -S, -S, 0, 1, 1},
    {+S, +S, -S, 0, 1, 1},
    {-S, +S, -S, 0, 1, 1},
    {-S, -S, -S, 0, 1, 1},
};

/**
 * Indeksy trójkątów
 */
static const unsigned short cube_indices[] = {
    0, 1, 2, 2, 3, 0,       // +X
    4, 5, 6, 6, 7, 4,       // -X
    8, 9, 10, 10, 11, 8,    // +Y
    12, 13, 14, 14, 15, 12, // -Y
    16, 17, 18, 18, 19, 16, // +Z
    20, 21, 22, 22, 23, 20  // -Z
};

// ================== SHADERY ==================

static const char *vertex_shader_text =
    "#version 120\n"
    "attribute vec3 vPos;\n"
    "attribute vec3 vCol;\n"
    "varying vec3 fCol;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    fCol = vCol;\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 120\n"
    "varying vec3 fCol;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(fCol, 1.0);\n"
    "}\n";

// ================== KAMERA FPS ==================

static vec3 cameraPos = {0.0f, 1.0f, 5.0f};
static float cameraYaw = -90.0f * (float)M_PI / 180.0f; // patrzymy w -Z
static float cameraPitch = 0.0f;

static float cameraSpeed = 3.0f; // jednostek / sekundę
static float mouseSensitivity = 0.0025f;
static float fov_deg = 60.0f; // pole widzenia w stopniach (10–120)

static int keys[1024] = {0};
static int firstMouse = 1;
static double lastMouseX = 0.0, lastMouseY = 0.0;
static int invertedY = 0; // 0 = normalne, 1 = inverted Y

// ================== LOSOWE KOSTKI ==================

#define NUM_CUBES 30
static vec3 cubePositions[NUM_CUBES];

static void init_cubes(void)
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_CUBES; ++i)
    {
        // rozrzut w zakresie ok. [-15,15] w X/Z, trochę w górę/dół w Y
        cubePositions[i][0] = ((float)rand() / (float)RAND_MAX - 0.5f) * 30.0f;
        cubePositions[i][1] = ((float)rand() / (float)RAND_MAX - 0.5f) * 4.0f;
        cubePositions[i][2] = -((float)rand() / (float)RAND_MAX) * 40.0f; // bardziej "przed" kamerą
    }
}

// ================== POMOCNICZE ==================

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}

static GLuint compile_shader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        char log[4096];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, sizeof log, &len, log);
        fprintf(stderr, "Shader compile error:\n%.*s\n", (int)len, log);
    }
    return shader;
}

// ================== WEJŚCIE: KLAWIATURA + MYSZ ==================

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = 1;
        else if (action == GLFW_RELEASE)
            keys[key] = 0;
    }

    // Zmiana FOV (PLUS/MINUS) – co ~5 stopni
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL)
        { // + i Enter numpad
            fov_deg -= 5.0f;
            if (fov_deg < 10.0f)
                fov_deg = 10.0f;
        }
        else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
        {
            fov_deg += 5.0f;
            if (fov_deg > 120.0f)
                fov_deg = 120.0f;
        }
        else if (key == GLFW_KEY_I && action == GLFW_PRESS)
        {
            invertedY = !invertedY; // przełącz inverted Y
        }
    }
}

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = 0;
        return;
    }

    double dx = xpos - lastMouseX;
    double dy = ypos - lastMouseY;
    lastMouseX = xpos;
    lastMouseY = ypos;

    cameraYaw -= (float)(dx * mouseSensitivity);
    cameraPitch += (float)((invertedY ? dy : -dy) * mouseSensitivity);

    // ograniczenie pitch do ~[-89°,89°]
    const float pitchLimit = 89.0f * (float)M_PI / 180.0f;
    if (cameraPitch > pitchLimit)
        cameraPitch = pitchLimit;
    if (cameraPitch < -pitchLimit)
        cameraPitch = -pitchLimit;
}

static void update_camera(float dt)
{
    // macierz rotacji kamery (bez translacji)
    mat4x4 R;
    mat4x4_identity(R);
    mat4x4_rotate_Y(R, R, cameraYaw);
    mat4x4_rotate_X(R, R, cameraPitch);

    // wektory bazowe w lokalnym układzie kamery
    vec4 forwardLocal = {0.f, 0.f, -1.f, 0.f};
    vec4 rightLocal = {1.f, 0.f, 0.f, 0.f};

    vec4 forward4, right4;
    mat4x4_mul_vec4(forward4, R, forwardLocal);
    mat4x4_mul_vec4(right4, R, rightLocal);

    vec3 forward = {forward4[0], forward4[1], forward4[2]};
    vec3 right = {right4[0], right4[1], right4[2]};

    vec3_norm(forward, forward);
    vec3_norm(right, right);

    float velocity = cameraSpeed * dt;

    // W/S – ruch przód/tył
    if (keys[GLFW_KEY_W])
    {
        cameraPos[0] += forward[0] * velocity;
        cameraPos[1] += forward[1] * velocity;
        cameraPos[2] += forward[2] * velocity;
    }
    if (keys[GLFW_KEY_S])
    {
        cameraPos[0] -= forward[0] * velocity;
        cameraPos[1] -= forward[1] * velocity;
        cameraPos[2] -= forward[2] * velocity;
    }

    // A/D – w lewo / w prawo (strafe)
    if (keys[GLFW_KEY_A])
    {
        cameraPos[0] -= right[0] * velocity;
        cameraPos[1] -= right[1] * velocity;
        cameraPos[2] -= right[2] * velocity;
    }
    if (keys[GLFW_KEY_D])
    {
        cameraPos[0] += right[0] * velocity;
        cameraPos[1] += right[1] * velocity;
        cameraPos[2] += right[2] * velocity;
    }
}

// ================== MAIN ==================

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow *window = glfwCreateWindow(800, 600,
                                          "OpenGL + GLFW: kamera FPS", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        fprintf(stderr, "Nie udalo sie zainicjalizowac GLAD\n");
        return -1;
    }

    // ukrycie i „zablokowanie” kursora – klasyczne FPS
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // shadery
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_text);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_text);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "vPos");
    glBindAttribLocation(prog, 1, "vCol");
    glLinkProgram(prog);
    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[4096];
        GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof log, &len, log);
        fprintf(stderr, "Link error:\n%.*s\n", (int)len, log);
        return -1;
    }

    GLint u_MVP = glGetUniformLocation(prog, "MVP");

    // bufory
    GLuint vbo = 0, ebo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(3 * sizeof(float)));

    init_cubes();

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // aktualizacja kamery z klawiszy
        update_camera(dt);

        // ====== MACIERZ ŚWIATA KAMERY Wc ======
        mat4x4 Rc;
        mat4x4_identity(Rc);
        mat4x4_rotate_Y(Rc, Rc, cameraYaw);
        mat4x4_rotate_X(Rc, Rc, cameraPitch);

        mat4x4 Tc;
        mat4x4_translate(Tc,
                         cameraPos[0],
                         cameraPos[1],
                         cameraPos[2]);

        mat4x4 Wc;
        mat4x4_mul(Wc, Tc, Rc);

        // ====== MACIERZ WIDOKU V = Wc^-1 (BEZ look_at!) ======
        mat4x4 V;
        mat4x4_invert(V, Wc);

        // ====== MACIERZ PROJEKCJI P (perspektywa z FOV 10–120°) ======
        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
        float fov_rad = fov_deg * (float)M_PI / 180.0f;
        mat4x4 P;
        mat4x4_perspective(P, fov_rad, aspect, 0.1f, 100.0f);

        mat4x4 VP;
        mat4x4_mul(VP, P, V);

        glUseProgram(prog);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        // rysujemy kilka losowo rozmieszczonych kostek
        for (int i = 0; i < NUM_CUBES; ++i)
        {
            mat4x4 M;
            mat4x4_identity(M);
            mat4x4_translate_in_place(M,
                                      cubePositions[i][0],
                                      cubePositions[i][1],
                                      cubePositions[i][2]);

            mat4x4 MVP;
            mat4x4_mul(MVP, VP, M);

            glUniformMatrix4fv(u_MVP, 1, GL_FALSE, (const GLfloat *)MVP);
            glDrawElements(GL_TRIANGLES,
                           sizeof(cube_indices) / sizeof(cube_indices[0]),
                           GL_UNSIGNED_SHORT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
