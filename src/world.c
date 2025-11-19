#include "world.h"
#include <stdlib.h>
#include <time.h>

#define NUM_CUBES 30
static vec3 cubes[NUM_CUBES];

/**
 * Inicjalizuje losowe pozycje obiektów w świecie.
 *
 * Wypełnia tablicę cubes losowymi współrzędnymi,
 * rozmieszczając sześciany w przestrzeni 3D w określonym zakresie.
 *
 * @return void
 */
void world_init()
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_CUBES; ++i)
    {
        cubes[i][0] = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
        cubes[i][1] = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
        cubes[i][2] = -((float)rand() / RAND_MAX) * 40.0f;
    }
}

/**
 * Zwraca liczbę obiektów w świecie.
 *
 * Umożliwia rendererowi ustalenie, ile sześcianów
 * należy narysować.
 *
 * @return Liczba obiektów w świecie
 */
int world_count()
{
    return NUM_CUBES;
}

/**
 * Pobiera pozycję obiektu o podanym indeksie.
 *
 *
 * @param i   Indeks obiektu
 * @param out Wektor, do którego zostanie zapisana pozycja
 * @return void
 */
void world_get_position(int i, vec3 out)
{
    vec3_dup(out, cubes[i]);
}
