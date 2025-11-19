#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>

/**
 * Ładuje i tworzy program shaderów z plików źródłowych
 *
 * @param vs_path Ścieżka do pliku vertex shader.
 * @param fs_path Ścieżka do pliku fragment shader.
 * @return
 */
GLuint shader_load_program_from_files(const char* vs_path, const char* fs_path);

#endif
