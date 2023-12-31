#version 400

layout(location=0) in vec4 in_Position;
layout(location=1) in vec4 in_TexUVx2;
layout(location=2) in vec4 in_Normal;

out vec4 ex_Position;			
out vec4 ex_PositionWorld;
out vec4 ex_Normal;
out vec4 ex_UV_x2;

uniform mat4 ModelMatrix;	
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main(void)
{
  mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = MVP * in_Position;

    ex_Position = gl_Position;
    ex_PositionWorld = ModelMatrix * in_Position;

    ex_Normal = normalize(ModelMatrix * vec4(in_Normal.xyz, 0.0));

    ex_UV_x2 = in_Position;
}
