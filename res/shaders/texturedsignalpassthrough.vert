uniform highp mat4 matrix;
attribute highp vec4 position;
attribute highp vec2 texcoord;
varying highp vec2 vTexcoord;

void main(void)
{
    vTexcoord = texcoord;
    gl_Position = matrix * position;
}
