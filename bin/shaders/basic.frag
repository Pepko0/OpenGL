#version 120
varying vec3 fCol;

void main()
{
    gl_FragColor = vec4(fCol, 1.0);
}
