#version 100
precision highp float;
// Texture
uniform bool u_has_texture;
uniform sampler2D u_texture_unit;
// Color
varying vec4 v_color;
varying vec4 v_texture_pos;
// Alpha Test
uniform bool u_alpha_test;
// Fog
uniform bool u_fog;
uniform vec4 u_fog_color;
uniform bool u_fog_is_linear;
uniform float u_fog_start;
uniform float u_fog_end;
varying vec4 v_fog_eye_position;
// Main
void main(void) {
    gl_FragColor = v_color;
    // Texture
    if (u_has_texture) {
        gl_FragColor *= texture2D(u_texture_unit, v_texture_pos.xy);
    }
    // Fog
    if (u_fog) {
        float fog_factor;
        if (u_fog_is_linear) {
            fog_factor = (u_fog_end - length(v_fog_eye_position)) / (u_fog_end - u_fog_start);
        } else {
            fog_factor = exp(-u_fog_start * length(v_fog_eye_position));
        }
        fog_factor = clamp(fog_factor, 0.0, 1.0);
        gl_FragColor.rgb = mix(gl_FragColor, u_fog_color, 1.0 - fog_factor).rgb;
    }
    // Alpha Test
    if (u_alpha_test && gl_FragColor.a <= 0.1) {
        discard;
    }
}
