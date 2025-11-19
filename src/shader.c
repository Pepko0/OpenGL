#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Wczytuje cały plik tekstowy do jednego ciągu znaków.
 *
 * Funkcja otwiera plik w trybie binarnym, odczytuje jego zawartość
 * i zwraca wskaźnik do zaalokowanego bufora zakończonego '\0'.
 * Wywołujący odpowiedzialny jest za zwolnienie pamięci funkcją free().
 *
 * @param path Ścieżka do pliku shaderowego.
 * @return Wskaźnik na bufor z tekstem pliku lub NULL w przypadku błędu.
 */
static char *read_file_to_string(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "Nie mozna otworzyc pliku shader: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *data = (char *)malloc(size + 1);
    if (!data)
    {
        fprintf(stderr, "Brak pamieci przy wczytywaniu pliku: %s\n", path);
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, 1, size, f);
    fclose(f);

    data[read] = '\0';
    return data;
}

/**
 * Kompiluje shader GLSL określonego typu z podanego źródła.
 *
 * Tworzy obiekt shadera, ładuje źródło i wykonuje kompilację.
 * W przypadku błędu wypisuje log kompilacji na stderr.
 *
 * @param type Typ shadera
 * @param src  Ciąg znaków zawierający kod źródłowy shadera
 * @return Identyfikator skompilowanego shadera lub 0 w razie kompilacji nieudanej
 */
static GLuint compile_shader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        char log[2048];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, sizeof(log), &len, log);
        fprintf(stderr, "Blad kompilacji shadera:\n%.*s\n", (int)len, log);
    }

    return shader;
}

/**
 * Ładuje program shaderów z dwóch plików: vertex i fragment shader
 *
 * Wczytuje pliki źródłowe, kompiluje oba shadery, tworzy program
 * wykonuje linkowanie oraz wiąże atrybuty vPos i vCol
 *
 * W przypadku błędów kompilacji lub linkowania funkcja wypisuje stosowną
 * informację i zwraca 0
 *
 * @param vs_path Ścieżka do pliku vertex shader
 * @param fs_path Ścieżka do pliku fragment shader
 * @return Identyfikator programu shaderskiego lub 0 w razie błędu
 */
GLuint shader_load_program_from_files(const char *vs_path, const char *fs_path)
{
    char *vs_src = read_file_to_string(vs_path);
    char *fs_src = read_file_to_string(fs_path);

    if (!vs_src || !fs_src)
    {
        free(vs_src);
        free(fs_src);
        return 0;
    }

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);

    free(vs_src);
    free(fs_src);

    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return 0;
    }

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
        char log[2048];
        GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof(log), &len, log);
        fprintf(stderr, "Blad linkowania programu:\n%.*s\n", (int)len, log);
        glDeleteProgram(prog);
        prog = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}
