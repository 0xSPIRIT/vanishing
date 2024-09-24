#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float time;

uniform float scan_intensity;
uniform float noise_intensity;
uniform float abberation_intensity;

out vec4 finalColor;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 tex_coord = fragTexCoord;

    const float pi = 3.14159265f;

    float sine_frequency = 100 + (scan_intensity-1) * 0.01;
    float sine_amplitude = 0.0002 + (scan_intensity-1) * 0.0001;

    // increase amplitude to top and bottom

    float t_off = time * 20 + scan_intensity;

    // sine wave to entire scene
    tex_coord.x += sine_amplitude * sin(t_off + tex_coord.y * sine_frequency);

    if (scan_intensity >= 1) {
        float scanline_frequency = 4 + (scan_intensity-1) * 0.01;
        float scanline_amplitude = 0.0005 + (scan_intensity-1) * 0.001;
        float scanline_speed = 2 + (scan_intensity-1) * 0.1;
        tex_coord.x += scanline_amplitude * tan(time * scanline_speed + scanline_frequency * tex_coord.y);
    }

    if (abberation_intensity > 0) {
        float redOffset   =  0.009;
        float greenOffset =  0.006;
        float blueOffset  = -0.006;

        vec2 dir = vec2(0.32,0.32);
        dir *= abberation_intensity;

        finalColor.r  = texture(texture0, tex_coord + dir * vec2(redOffset)).r;
        finalColor.g  = texture(texture0, tex_coord + dir * vec2(greenOffset)).g;
        finalColor.ba = texture(texture0, tex_coord + dir * vec2(blueOffset)).ba;
    } else {
        finalColor = texture(texture0, tex_coord);
    }

    float x = mod(fragTexCoord.x * 0.005 + time/2, 100);
    float y = mod(fragTexCoord.y * 0.006 + time*2, 100);

    float amp = 0.2 * noise_intensity;

    // noise
    finalColor.r += -amp/2.0 + amp * rand(vec2(x, y));
    finalColor.g += -amp/2.0 + amp * rand(vec2(y, x));
    finalColor.b += -amp/2.0 + amp * rand(vec2(x, y));
}
