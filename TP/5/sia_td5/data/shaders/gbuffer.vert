#version 410 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

in vec3 vtx_position;
in vec3 vtx_color;
in vec3 vtx_normal;

out vec3 vert_normal_view;
out vec3 vert_color;

void main()
{
    // World to view matrix
    mat4 MV  = view_matrix * model_matrix;

    vert_normal_view = normalize(normal_matrix * vtx_normal);

    vert_color = vtx_color;

    gl_Position = projection_matrix * MV * vec4(vtx_position, 1.0);
}
