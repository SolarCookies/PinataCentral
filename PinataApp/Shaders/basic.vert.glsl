#version 460 core

layout(location = 0) in vec3 a_Position;

layout(location = 0) out vec3 out_color;

layout(push_constant) uniform PushConstants {
	mat4 ViewProjection;
    mat4 Transform;
} push_constants;

vec3 triangle_colors[12] = vec3[](
    //rainbow
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.5, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.5, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.5),
    vec3(0.0, 1.0, 1.0),
    vec3(0.0, 0.5, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.5, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.5)
);

void main()
{
    gl_Position = push_constants.ViewProjection * push_constants.Transform * vec4(a_Position, 1.0);

    out_color = triangle_colors[gl_VertexIndex % 12];
}
