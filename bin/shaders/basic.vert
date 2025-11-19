#version 120
attribute vec3 vPos;
attribute vec3 vCol;
varying vec3 fCol;
uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    fCol = vCol;
}
