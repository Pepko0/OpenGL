#include "camera.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14
#endif

/**
 *Inicjalizuje strukturę kamery domyślnymi wartościami.
 *
 * Funkcja ustawia wszystkie pola struktury Camera na wartości startowe,
 * definiujące początkową pozycję obserwatora, kierunek patrzenia,
 * szybkość ruchu, czułość myszy oraz początkowe pole widzenia (FOV).
 *
 * @param cam
 */
void camera_init(Camera *cam)
{
    cam->position[0] = 0.0f;
    cam->position[1] = 1.0f;
    cam->position[2] = 5.0f;

    cam->yaw = -90.0f * (float)M_PI / 180.0f;
    cam->pitch = 0.0f;

    cam->speed = 3.0f;
    cam->mouseSensitivity = 0.0025f;
    cam->fov_deg = 60.0f;
    cam->invertedY = 0;
}

static int firstMouse = 1;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;

/**
 * Aktualizuje kierunek patrzenia kamery na podstawie ruchu myszy.
 *
 * Na podstawie przesunięcia kursora oblicza zmianę yaw i pitch,
 * uwzględniając czułość myszy oraz opcję odwrócenia osi Y.
 * Pierwsze poruszenie myszy służy jedynie do ustalenia pozycji startowej.
 *
 * @param cam   Wskaźnik na strukturę Camera.
 * @param xpos  Aktualna pozycja kursora w osi X.
 * @param ypos  Aktualna pozycja kursora w osi Y.
 * @return void
 */
void camera_on_mouse_move(Camera *cam, double xpos, double ypos)
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

    // yaw ma minus, żeby lewo/prawo było zgodne z ruchem myszy
    cam->yaw -= (float)(dx * cam->mouseSensitivity);
    cam->pitch += (float)((cam->invertedY ? dy : -dy) * cam->mouseSensitivity);

    const float pitchLimit = 89.0f * (float)M_PI / 180.0f;
    if (cam->pitch > pitchLimit)
        cam->pitch = pitchLimit;
    if (cam->pitch < -pitchLimit)
        cam->pitch = -pitchLimit;
}

/**
 * Aktualizuje pozycję kamery na podstawie wciśniętych klawiszy.
 *
 * Wylicza wektory ruchu (forward i right) na podstawie bieżącego yaw/pitch,
 * a następnie zmienia pozycję kamery proporcjonalnie do prędkości oraz dt.
 *
 * @param cam   Wskaźnik na strukturę Camera.
 * @param dt    Upływ czasu w sekundach od ostatniej aktualizacji.
 * @param keyW  1 jeśli W jest wciśnięty - ruch do przodu.
 * @param keyS  1 jeśli S jest wciśnięty - ruch do tyłu.
 * @param keyA  1 jeśli A jest wciśnięty - strafe w lewo.
 * @param keyD  1 jeśli D jest wciśnięty - strafe w prawo.
 * @return void
 */
void camera_update_movement(Camera *cam, float dt,
                            int keyW, int keyS, int keyA, int keyD)
{
    mat4x4 R;
    mat4x4_identity(R);
    mat4x4_rotate_Y(R, R, cam->yaw);
    mat4x4_rotate_X(R, R, cam->pitch);

    vec4 forwardLocal = {0.f, 0.f, -1.f, 0.f};
    vec4 rightLocal = {1.f, 0.f, 0.f, 0.f};

    vec4 forward4, right4;
    mat4x4_mul_vec4(forward4, R, forwardLocal);
    mat4x4_mul_vec4(right4, R, rightLocal);

    vec3 forward = {forward4[0], forward4[1], forward4[2]};
    vec3 right = {right4[0], right4[1], right4[2]};

    vec3_norm(forward, forward);
    vec3_norm(right, right);

    float velocity = cam->speed * dt;

    // Ruch do przodu (W)
    if (keyW)
    {
        cam->position[0] += forward[0] * velocity;
        cam->position[1] += forward[1] * velocity;
        cam->position[2] += forward[2] * velocity;
    }

    // Ruch do tyłu (S)
    if (keyS)
    {
        cam->position[0] -= forward[0] * velocity;
        cam->position[1] -= forward[1] * velocity;
        cam->position[2] -= forward[2] * velocity;
    }

    // Ruch w lewo (A)
    if (keyA)
    {
        cam->position[0] -= right[0] * velocity;
        cam->position[1] -= right[1] * velocity;
        cam->position[2] -= right[2] * velocity;
    }

    // Ruch w lewo (D)
    if (keyD)
    {
        cam->position[0] += right[0] * velocity;
        cam->position[1] += right[1] * velocity;
        cam->position[2] += right[2] * velocity;
    }
}

/**
 * Zmienia pole widzenia (FOV) kamery o podaną wartość.
 *
 * Dodaje do aktualnego FOV wartość delta, a następnie ogranicza wynik
 * do przedziału 10°–120°, aby zapobiec ekstremalnym wartościom pola widzenia.
 *
 * @param cam   Wskaźnik na strukturę Camera.
 * @param delta Zmiana FOV w stopniach (może być dodatnia lub ujemna).
 * @return void
 */
void camera_change_fov(Camera *cam, float delta)
{
    cam->fov_deg += delta;
    if (cam->fov_deg < 10.0f)
        cam->fov_deg = 10.0f;
    if (cam->fov_deg > 120.0f)
        cam->fov_deg = 120.0f;
}

/**
 * Przełącza tryb odwróconej osi Y dla ruchu myszy.
 *
 * Zmienia wartość invertedY na przeciwną, włączając lub wyłączając
 * odwrócone sterowanie ruchem kamery w pionie.
 *
 * @param cam Wskaźnik na strukturę Camera.
 * @return void
 */
void camera_toggle_inverted_y(Camera *cam)
{
    cam->invertedY = !cam->invertedY;
}

/**
 * Oblicza macierz widoku kamery (V).
 *
 * Tworzy macierz świata kamery Wc poprzez złożenie translacji i rotacji
 * wynikających z pozycji oraz kierunku patrzenia, a następnie odwraca ją
 * aby uzyskać macierz widoku V = Wc-1.
 *
 * @param cam     Wskaźnik na strukturę Camera.
 * @param V_out   Macierz wyjściowa 4x4, w której zapisany zostanie wynik.
 * @return void
 */
void camera_get_view_matrix(const Camera *cam, mat4x4 V_out)
{
    mat4x4 Rc;
    mat4x4_identity(Rc);
    mat4x4_rotate_Y(Rc, Rc, cam->yaw);
    mat4x4_rotate_X(Rc, Rc, cam->pitch);

    mat4x4 Tc;
    mat4x4_translate(Tc,
                     cam->position[0],
                     cam->position[1],
                     cam->position[2]);

    mat4x4 Wc;
    mat4x4_mul(Wc, Tc, Rc);

    mat4x4_invert(V_out, Wc);
}

/**
 * Oblicza macierz rzutowania perspektywicznego kamery.
 *
 * Zamienia FOV zapisany w stopniach na radiany i tworzy macierz
 * projekcji perspektywicznej z wykorzystaniem funkcji mat4x4_perspective.
 *
 * @param cam     Wskaźnik na strukturę Camera.
 * @param aspect  Współczynnik proporcji szerokości do wysokości.
 * @param P_out   Macierz wyjściowa 4x4, w której zapisany zostanie wynik.
 * @return void
 */
void camera_get_projection_matrix(const Camera *cam, float aspect, mat4x4 P_out)
{
    float fov_rad = cam->fov_deg * (float)M_PI / 180.0f;
    mat4x4_perspective(P_out, fov_rad, aspect, 0.1f, 100.0f);
}
