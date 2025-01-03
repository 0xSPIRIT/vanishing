precision mediump float;
precision mediump int;

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float time;

uniform float scan_intensity;
uniform float noise_intensity;
uniform float abberation_intensity;
uniform float mix_factor;
uniform float vignette_intensity;
uniform float vignette_mix;

uniform int do_spin;
uniform int do_wiggle;

out vec4 finalColor;

vec2 clamp_vec2(vec2 a, vec2 min, vec2 max) {
    if (a.x < min.x) a.x = min.x;
    if (a.y < min.y) a.y = min.y;
    if (a.x > max.x) a.x = max.x;
    if (a.y > max.y) a.y = max.y;

    return a;
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 vignette(vec2 tex_coord, vec4 col, float intensity) {
    vec4 unprocessed = col;

    if (intensity == 0.0) return col;

    float dist = 1.0;
    dist = length(tex_coord.xy - vec2(0.5, 0.5));
    dist *= 2.0;

    dist = 1.0 - dist;
    dist *= 1.35f * intensity;

    /*
    if (do_scanline_effect == 1)
        dist *= (0.9 + 0.1 * rand(vec2(time, time)));
        */

    if (intensity == 1.0)
        dist += 0.45;
    else
        dist = max(dist, 0.18);

    col.r = clamp(col.r * dist, 0.0, 1.0);
    col.g = clamp(col.g * dist, 0.0, 1.0);
    col.b = clamp(col.b * dist, 0.0, 1.0);

    col = mix(unprocessed, col, vignette_mix);

    return col;
}

void main() {
    vec2 tex_coord = fragTexCoord;

    if (do_wiggle == 1) {
        float f = mod(time,30.0);
        tex_coord.y += 0.001 * rand(vec2(f, f));
    }

    if (do_spin == 1) {
        float amount = mod(time*3.0, 1.0);
        amount *= amount * amount;

        tex_coord.y += amount;

        float sign = 1.0;
        //if (mod(time, 2) < 1) sign = -1;

        tex_coord.x += sign * 0.25 * tex_coord.y * tex_coord.y;
    }

    vec4 unprocessed_color = texture(texture0, tex_coord);

    const float pi = 3.14159265f;

    float sine_frequency = 100.0 + (scan_intensity-1.0) * 0.01;
    float sine_amplitude = 0.0002 + (scan_intensity-1.0) * 0.0001;

    // increase amplitude to top and bottom

    float t = 900.0 + mod(time, 10.0);

    float t_off = t * 20.0 * scan_intensity;

    // sine wave to entire scene
    tex_coord.x += sine_amplitude * sin(t_off + tex_coord.y * sine_frequency);

    float scanline_frequency = 4.0 * scan_intensity;
    float scanline_amplitude = 0.0005 * scan_intensity;
    float scanline_speed = 2.0 * scan_intensity * sqrt(scan_intensity);
    tex_coord.x += scanline_amplitude * tan(t * scanline_speed + scanline_frequency * tex_coord.y);

    if (abberation_intensity > 0.0) {
        float redOffset   =  0.009;
        float greenOffset =  0.006;
        float blueOffset  = -0.006;

        vec2 dir = vec2(0.32,0.32);
        dir *= abberation_intensity;

        vec2 r_coord = tex_coord + dir * vec2(redOffset);
        vec2 g_coord = tex_coord + dir * vec2(greenOffset);
        vec2 b_coord = tex_coord + dir * vec2(blueOffset);

        /*
        r_coord = clamp_vec2(r_coord, vec2(0.0, 0.0), vec2(1.0, 0.99));
        g_coord = clamp_vec2(g_coord, vec2(0.0, 0.0), vec2(1.0, 1.0));
        b_coord = clamp_vec2(b_coord, vec2(0.0, 0.0), vec2(1.0, 1.0));
        */

        finalColor.r  = texture(texture0, r_coord).r;
        finalColor.g  = texture(texture0, g_coord).g;
        finalColor.ba = texture(texture0, b_coord).ba;
    } else {
        finalColor = texture(texture0, tex_coord);
    }

    float x = mod(fragTexCoord.x * 0.005 + t/2.0, 100.0);
    float y = mod(fragTexCoord.y * 0.006 + t*2.0, 100.0);

    float amp = 0.2 * noise_intensity;

    float x_r = x;
    float y_r = y;

    float x_g = y;
    float y_g = x;

    float x_b = x;
    float y_b = y;

    if (noise_intensity < 0.5) {
        x_r *= 2.0;
        y_r *= 3.0;
        x_g *= 4.0;
        y_g *= 5.0;
        x_b *= 6.0;
        y_b *= 7.0;
    } else if (noise_intensity > 2.0) {
        x = mod(fragTexCoord.x * 0.005 + t/2.0, 200.0);
        y = mod(fragTexCoord.y * 0.009 + t*2.0, 100.0);

        x_r = x;
        y_r = y;

        x_g = y;
        y_g = x;

        x_b = x;
        y_b = y;
    }

    // noise
    amp *= mix_factor;
    finalColor.r += -amp/2.0 + amp * rand(vec2(x_r, y_r));
    finalColor.g += -amp/2.0 + amp * rand(vec2(x_g, y_g));
    finalColor.b += -amp/2.0 + amp * rand(vec2(x_b, y_b));

    float tv = mix_factor;

    finalColor.r = tv * (finalColor.r) + unprocessed_color.r * (1.0-tv);
    finalColor.g = tv * (finalColor.g) + unprocessed_color.g * (1.0-tv);
    finalColor.b = tv * (finalColor.b) + unprocessed_color.b * (1.0-tv);

    // apply the vignette after
    finalColor = vignette(fragTexCoord, finalColor, 1.0);
}


