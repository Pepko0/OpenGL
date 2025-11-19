#ifndef RENDERER_H
#define RENDERER_H

#include "../external/linmath/linmath.h"

/**
 * Inicjalizuje renderer oraz zasoby OpenGL.
 *
 * Ładuje program shaderów, przygotowuje bufory VBO/EBO
 * oraz konfiguruje atrybuty wierzchołków.
 *
 * @return 
 */
int  renderer_init();   

/**
 * Renderuje całą scenę przy użyciu podanej macierzy VP.
 *
 * Dla każdego obiektu świata oblicza macierz modelu,
 * wyznacza macierz MVP i rysuje sześcian funkcją glDrawElements().
 *
 * @param VP 
 */
void renderer_draw(const mat4x4 VP);

/**
 * Zwalnia wszystkie zasoby utworzone przez renderer.
 *
 * Usuwa bufory VBO/EBO i program shaderów, a następnie
 * zeruje uchwyty, aby zapobiec dalszemu używaniu.
 */
void renderer_shutdown();

#endif
