#ifndef WORLD_H
#define WORLD_H

#include "../external/linmath/linmath.h"

/**
 * Inicjalizuje świat i generuje losowe pozycje obiektów.
 *
 * Przygotowuje tablicę pozycji sześcianów, rozmieszczając je
 * losowo w przestrzeni 3D
 */
void world_init();

/**
 * Zwraca liczbę obiektów w świecie
 *
 * Wykorzystywane przez renderer do ustalenia,
 * ile sześcianów należy narysować
 *
 * @return Liczba dostępnych obiektów.
 */
int  world_count();

/**
 * Pobiera pozycję obiektu o podanym indeksie
 *
 * @param i   Indeks obiektu w zakresie 0–world_count()-1
 * @param out Wektor wyjściowy, do którego zapisane zostaną współrzędne
 */
void world_get_position(int i, vec3 out);

#endif
