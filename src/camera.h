#ifndef CAMERA_H
#define CAMERA_H

#include "../external/linmath/linmath.h"

typedef struct
{
    vec3 position;
    float yaw;
    float pitch;
    float speed;
    float mouseSensitivity;
    float fov_deg;
    int invertedY;
} Camera;

/**
 * Inicjalizuje strukturę Camera domyślnymi wartościami.
 *
 * Ustawia początkową pozycję kamery, kierunek patrzenia,
 * prędkość ruchu, czułość myszy oraz wartość FOV.
 *
 * @param cam Wskaźnik na strukturę Camera do inicjalizacji.
 */
void camera_init(Camera *cam);

/**
 * Aktualizuje kierunek patrzenia kamery na podstawie ruchu myszy.
 *
 * Na podstawie zmiany pozycji kursora oblicza korekty yaw i pitch,
 * uwzględniając czułość myszy oraz tryb inverted Y.
 *
 * @param cam   Wskaźnik na strukturę Camera.
 * @param xpos  Aktualna pozycja kursora w osi X.
 * @param ypos  Aktualna pozycja kursora w osi Y.
 */
void camera_on_mouse_move(Camera *cam, double xpos, double ypos);

/**
 * Aktualizuje pozycję kamery na podstawie wciśniętych klawiszy.
 *
 * Przesuwa kamerę w kierunkach przód/tył oraz lewo/prawo,
 * biorąc pod uwagę aktualny yaw/pitch, prędkość kamery oraz upływ czasu.
 *
 * @param cam
 * @param dt
 * @param keyW
 * @param keyS
 * @param keyA
 * @param keyD
 */
void camera_update_movement(Camera *cam, float dt,
                            int keyW, int keyS, int keyA, int keyD);

/**
 * Zmienia wartość FOV kamery o podaną deltę.
 *
 * Modyfikuje pole widzenia w stopniach, a następnie ogranicza je
 * do zakresu 10°–120°.
 *
 * @param cam
 * @param delta
 */
void camera_change_fov(Camera *cam, float delta);

/**
 * Przełącza tryb odwróconej osi Y kamery.
 *
 * Zmienia wartość invertedY na przeciwną, włączając lub wyłączając
 * odwrócone sterowanie ruchem myszy w pionie.
 *
 * @param cam
 */
void camera_toggle_inverted_y(Camera *cam);

/**
 * Oblicza macierz widoku kamery (V = Wc-1).
 *
 * Tworzy macierz świata kamery na podstawie jej pozycji oraz rotacji,
 * a następnie odwraca ją, zapisując wynik do macierzy wyjściowej.
 *
 * @param cam
 * @param V_out
 */
void camera_get_view_matrix(const Camera *cam, mat4x4 V_out);

/**
 * Oblicza macierz projekcji perspektywicznej kamery.
 *
 * Konwertuje FOV kamery ze stopni na radiany i tworzy macierz
 * perspektywiczną na podstawie współczynnika aspect ratio oraz
 *
 * @param cam    
 * @param aspect  
 * @param P_out   
 */
void camera_get_projection_matrix(const Camera *cam, float aspect, mat4x4 P_out);

#endif
