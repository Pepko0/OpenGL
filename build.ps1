Write-Host "Kompilowanie..." -ForegroundColor Cyan

g++ `
  src/main.c `
  src/camera.c `
  src/shader.c `
  src/world.c `
  src/renderer.c `
  external/glad/src/glad.c `
  -Iexternal/glfw/include `
  -Iexternal/glad/include `
  -Iexternal/linmath `
  -Lexternal/glfw/lib-mingw-w64 `
  -o bin/OpenGL.exe `
  -lglfw3 -lopengl32 -lgdi32

Write-Host "Gotowe!" -ForegroundColor Green
