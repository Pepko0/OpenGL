// main.c - OpenGL + GLFW + GLAD + linmath
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../external/linmath/linmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



static void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/**
 * Definiuje wierzchołek sześcianu:
 * - pozycja (x,y,z)
 * - kolor (r,g,b)
 */
typedef struct { float x,y,z; float r,g,b; } Vertex;

// Rozmiar sześcianu (krawędź 1.0, środek w (0,0,0))
static const float S = 0.5f;

/**
 * Tablica cube_vertices zawiera 24 wierzchołki sześcianu.
 * Trzeba było zduplikować wierzchołki, aby każda ściana miała inny kolor
 */
static const Vertex cube_vertices[] = {
    // +X czerwony
    {+S,-S,-S, 1,0,0}, {+S,+S,-S, 1,0,0}, {+S,+S,+S, 1,0,0}, {+S,-S,+S, 1,0,0},
    // -X zielony
    {-S,-S,+S, 0,1,0}, {-S,+S,+S, 0,1,0}, {-S,+S,-S, 0,1,0}, {-S,-S,-S, 0,1,0},
    // +Y niebieski
    {-S,+S,-S, 0,0,1}, {-S,+S,+S, 0,0,1}, {+S,+S,+S, 0,0,1}, {+S,+S,-S, 0,0,1},
    // -Y żółty
    {-S,-S,+S, 1,1,0}, {-S,-S,-S, 1,1,0}, {+S,-S,-S, 1,1,0}, {+S,-S,+S, 1,1,0},
    // +Z magenta
    {-S,-S,+S, 1,0,1}, {+S,-S,+S, 1,0,1}, {+S,+S,+S, 1,0,1}, {-S,+S,+S, 1,0,1},
    // -Z cyan
    {+S,-S,-S, 0,1,1}, {-S,-S,-S, 0,1,1}, {-S,+S,-S, 0,1,1}, {+S,+S,-S, 0,1,1},
};

/**
 * Tablica definiuje kolejność łączenia wierzchołków w trójkąty,
 * z których zbudowany jest cały sześcian (graniastosłup prosty).
 *
 * ściana składa się z 4 wierzchołków które są połączone w 2 trójkąty.
 *
 */

static const unsigned short cube_indices[] = {
    0,1,2,      2,3,0,        // +X
    4,5,6,      6,7,4,        // -X
    8,9,10,     10,11,8,      // +Y
    12,13,14,   14,15,12,     // -Y
    16,17,18,   18,19,16,     // +Z
    20,21,22,   22,23,20      // -Z
};

/**
 * Kompiluje shader źródłowy (vertex lub fragment)
 * i zwraca jego identyfikator.
 *
 * - type -> rodzaj shadera 
 * - src  -> wskaźnik na kod źródłowy shadera w języku GLSL
 *
 * Funkcja jest potrzebna poniważ OpenGl wymaga aby kod 
 * shadera był najpierw skompilowany, a potem połączony w program.
 */

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[4096]; GLsizei len=0;
        glGetShaderInfoLog(s, sizeof log, &len, log);
        fprintf(stderr,"Shader compile error:\n%.*s\n",(int)len,log);
        exit(EXIT_FAILURE);
    }
    return s;
}

static const char* vertex_shader_text =
    "#version 120\n"
    "uniform mat4 MVP;\n"
    "attribute vec3 vPos;\n"
    "attribute vec3 vCol;\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char* fragment_shader_text =
    "#version 120\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

int main(void) {
    glfwSetErrorCallback(error_callback);
    
    /**
     * Inicjalizacja biblioteki GLFW i utworzenie okna renderowania. 
     */
    if (!glfwInit()) return -1; 

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL + GLFW: graniastoslup", NULL, NULL); // tworzy okno 
    if(!window){ glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window); // aktywuje okno 
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1); //VSync 

    if(!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Nie udalo sie zainicjalizowac GLAD\n");
        return -1;
    }

    /**
     * Kompilacja i połączenie shaderów w program.
     *
     * - Vertex shader oblicza pozycję każdego wierzchołka w przestrzeni 3D
     * - Fragment shader odpowiada za kolor każdego piksela na ekranie
     *
     * Po kompilacji oba shadery są łączone w jeden program
     */
    GLuint vs = compile_shader(GL_VERTEX_SHADER,   vertex_shader_text);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_text);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "vPos");
    glBindAttribLocation(prog, 1, "vCol");
    glLinkProgram(prog);
    GLint linked=0; glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if(!linked){
        char log[4096]; GLsizei len=0;
        glGetProgramInfoLog(prog, sizeof log, &len, log);
        fprintf(stderr,"Link error:\n%.*s\n",(int)len,log);
        return -1;
    }

    GLint u_MVP = glGetUniformLocation(prog, "MVP");

    /**
     * Utworzenie buforów na karcie graficznej:
     *
     * - VBO (Vertex Buffer Object) przechowuje współrzędne i kolory wierzchołków
     * - EBO (Element Buffer Object) przechowuje indeksy (kolejność rysowania trójkątów)
     *
     * Dzięki temu dane są przetwarzane przez GPU bardzo szybko,
     */
    GLuint vbo=0, ebo=0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));

    glEnable(GL_DEPTH_TEST);


    /**
     * Główna pętla renderowania:
     *
     * 1. Odczytuje aktualny rozmiar okna i czyści ekran
     * 2. Tworzy macierze transformacji:
     *    - Model (M): obrót i przesunięcie sześcianu
     *    - View  (V): pozycja kamery 
     *    - Projection (P): rzut perspektywiczny 
     * 3. Łączy macierze w jedną (MVP) i przekazuje ją do shaderów
     * 4. Wywołuje glDrawElements(), aby narysować bryłę z trójkątów
     * 5. Wymienia bufory (SwapBuffers) i przetwarza zdarzenia (PollEvents)
     */
    while(!glfwWindowShouldClose(window)) {
        int w,h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0,0,w,h);
        glClearColor(0.08f,0.08f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = (float)glfwGetTime();

        mat4x4 I;      mat4x4_identity(I);
        mat4x4 M;      mat4x4_identity(M); // macierz jednostkowa
        mat4x4 Rx, Ry; mat4x4_identity(Rx); mat4x4_identity(Ry);

        /**
         * Transformacje modelu:
         *
         * - Rotacja wokół osi X i Y tworzy efekt obrotu w czasie rzeczywistym
         * - Translacja (T) przesuwa obiekt po okręgu w płaszczyźnie XY
         * - Wynikowa macierz M opisuje położenie i orientację sześcianu w przestrzeni
         */        
        mat4x4_rotate_X(Rx, I, t * 0.9f);// obrót wokół osi X
        mat4x4_rotate_Y(Ry, I, t * 1.3f);// obrót wokół osi Y
        mat4x4_mul(M, Ry, Rx);

        
        mat4x4 T; mat4x4_identity(T);
        mat4x4_translate_in_place(T, 0.6f*sinf(t*0.7f), 0.4f*cosf(t*0.7f), 0.0f); // przesunięcie
        mat4x4_mul(M, T, M); 

        float aspect = (h>0) ? (float)w/(float)h : 1.0f;
        mat4x4 P; mat4x4_perspective(P, 60.0f*(float)M_PI/180.0f, aspect, 0.1f, 100.0f);  // projekcja perspektywiczna
        mat4x4 V; mat4x4_identity(V);
        mat4x4_translate_in_place(V, 0.0f, 0.0f, -3.0f);

        mat4x4 VP;  mat4x4_mul(VP, P, V);
        mat4x4 MVP; mat4x4_mul(MVP, VP, M); 

        glUseProgram(prog);
        glUniformMatrix4fv(u_MVP, 1, GL_FALSE, (const GLfloat*)MVP);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, sizeof(cube_indices)/sizeof(cube_indices[0]), GL_UNSIGNED_SHORT, 0);

        glfwSwapBuffers(window); // reaguje na zdarzenia 
        glfwPollEvents();
    }

    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
