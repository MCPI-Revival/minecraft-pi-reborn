#version 100
precision highp float;
// Matrices
uniform mat4 u_projection;
uniform mat4 u_model_view;
uniform mat4 u_texture;
// Texture
attribute vec3 a_vertex_coords;
attribute vec2 a_texture_coords;
varying vec4 v_texture_pos;
// Color
attribute vec4 a_color;
varying vec4 v_color;
// Fog
varying vec4 v_fog_eye_position;
// Main
void main(void) {
    v_texture_pos = u_texture * vec4(a_texture_coords.xy, 0.0, 1.0);
    gl_Position = u_projection * u_model_view * vec4(a_vertex_coords.xyz, 1.0);
    v_color = a_color;
    v_fog_eye_position = u_model_view * vec4(a_vertex_coords.xyz, 1.0);
}
