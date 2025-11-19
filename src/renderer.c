#include <glad/gl.h>
#include "../external/linmath/linmath.h"

#include "renderer.h"
#include "shader.h"
#include "world.h"
#include <stdio.h>

typedef struct
{
    float x, y, z;
    float r, g, b;
} Vertex;

static const float S = 0.5f;

/**
 * Tablica wierzchołków pojedynczego sześcianu.
 *
 * Zawiera 24 wierzchołki opisujące 6 ścian sześcianu (po 4 wierzchołki
 * na ścianę). Każdy wierzchołek posiada pozycję (x, y, z) oraz kolor (r, g, b).
 * Tablica jest używana przez renderer do rysowania sześcianów w scenie.
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
 * Indeksy trójkątów definiujących sześcian.
 *
 * Tablica zawiera 36 indeksów,
 * opisujących kolejność rysowania wierzchołków dla każdej z 6 ścian sześcianu.
 * Tablica jest wykorzystywana przez glDrawElements() do renderowania sześcianów.
 */
static const unsigned short cube_indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20};

static GLuint vbo = 0;
static GLuint ebo = 0;
static GLuint shaderProgram = 0;
static GLint u_MVP = -1;

/**
 * Inicjalizuje moduł renderera oraz zasoby OpenGL.
 *
 * Ładuje program shaderów z plików, pobiera lokalizację uniformu MVP,
 * tworzy i wypełnia bufory VBO oraz EBO danymi sześcianu, a także
 * konfiguruje atrybuty wierzchołków (pozycję i kolor).
 *
 * @return
 */
int renderer_init()
{
    shaderProgram = shader_load_program_from_files("shaders/basic.vert", "shaders/basic.frag");
    if (!shaderProgram)
    {
        fprintf(stderr, "renderer_init: nie udalo sie zaladowac shaderow.\n");
        return 0;
    }

    u_MVP = glGetUniformLocation(shaderProgram, "MVP");

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *)(3 * sizeof(float)));

    return 1;
}

/**
 * Renderuje wszystkie obiekty przy użyciu macierzy VP.
 *
 * Ustawia program shaderów, wiąże bufory VBO/EBO, a następnie
 * dla każdej bryły w świecie tworzy macierz modelu, oblicza
 * macierz MVP = VP * M i przekazuje ją do shadera. Na końcu
 * wykonuje rysowanie elementów sześcianu funkcją glDrawElements().
 *
 * @param VP  Macierz widokowo-projekcyjna (View * Projection).
 * @return void
 */
void renderer_draw(const mat4x4 VP)
{
    glUseProgram(shaderProgram);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    int count = world_count();

    for (int i = 0; i < count; ++i)
    {
        vec3 pos;
        world_get_position(i, pos);

        mat4x4 M;
        mat4x4_identity(M);
        mat4x4_translate_in_place(M, pos[0], pos[1], pos[2]);

        mat4x4 MVP;
        mat4x4_mul(MVP, VP, M);

        glUniformMatrix4fv(u_MVP, 1, GL_FALSE, (const float *)MVP);

        glDrawElements(GL_TRIANGLES,
                       sizeof(cube_indices) / sizeof(cube_indices[0]),
                       GL_UNSIGNED_SHORT, 0);
    }
}

/**
 * Zwalnia zasoby używane przez renderer.
 *
 * Usuwa bufory VBO i EBO, a także program shaderów jeśli zostały
 * wcześniej utworzone. Po zwolnieniu zasobów zeruje uchwyty,
 * aby zapobiec ponownemu użyciu nieprawidłowych wartości.
 *
 * @return void
 */
void renderer_shutdown()
{
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (ebo)
        glDeleteBuffers(1, &ebo);
    if (shaderProgram)
        glDeleteProgram(shaderProgram);
    vbo = ebo = shaderProgram = 0;
}
