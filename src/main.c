// main.c - OpenGL + GLFW + GLAD + modularny FPS
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../external/linmath/linmath.h"

#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "world.h"
#include "renderer.h"

static int keys[1024] = {0};
static Camera camera;

/**
 * @brief Callback obsługujący błędy GLFW
 *
 * Wypisuje komunikat błędu na stderr.
 *
 * @param error       Kod błędu GLFW.
 * @param description Opis błędu.
 */
static void error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}

/**
 * Obsługuje zdarzenia klawiatury
 *
 * Aktualizuje tablicę keys[], umożliwia sterowanie kamerą,
 * a także reaguje na klawisze związane ze zmianą FOV
 * oraz przełączaniem inverted Y.
 *
 * @param window    Okno GLFW.
 * @param key       Kod klawisza.
 * @param scancode  Kod scancode (nieużywany).
 * @param action    Typ akcji (press/release/repeat).
 * @param mods      Modyfikatory klawiszy (nieużywane).
 */
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = 1;
        if (action == GLFW_RELEASE)
            keys[key] = 0;
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL)
        {
            camera_change_fov(&camera, -5.0f);
        }
        else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
        {
            camera_change_fov(&camera, +5.0f);
        }
        else if (key == GLFW_KEY_I && action == GLFW_PRESS)
        {
            camera_toggle_inverted_y(&camera);
        }
    }
}

/**
 * Obsługuje ruch myszy
 *
 * Przekazuje aktualną pozycję kursora do modułu kamery
 * który aktualizuje yaw oraz pitch
 *
 * @param window Okno GLFW.
 * @param xpos   Pozycja kursora w osi X.
 * @param ypos   Pozycja kursora w osi Y.
 */
static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    camera_on_mouse_move(&camera, xpos, ypos);
}

/**
 * Inicjalizuje GLFW i GLAD, tworzy okno, ustawia callbacki,
 * konfiguruje kamerę, świat oraz renderer, następnie
 * uruchamia główną pętlę renderowania. 
 *
 * @return
 */
int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL FPS modularny", NULL, NULL);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    camera_init(&camera);
    world_init();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    if (!renderer_init())
    {
        fprintf(stderr, "renderer_init nie powiodl sie.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

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

        camera_update_movement(&camera, dt,
                               keys[GLFW_KEY_W],
                               keys[GLFW_KEY_S],
                               keys[GLFW_KEY_A],
                               keys[GLFW_KEY_D]);

        mat4x4 V, P, VP;
        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;

        camera_get_view_matrix(&camera, V);
        camera_get_projection_matrix(&camera, aspect, P);
        mat4x4_mul(VP, P, V);

        renderer_draw(VP);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
