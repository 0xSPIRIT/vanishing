/*
#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main() {
    mat4 bt601 = mat4(
    	1.16438,  0.00000,  1.59603, -0.87079,
    	1.16438, -0.39176, -0.81297,  0.52959,
    	1.16438,  2.01723,  0.00000, -1.08139,
    	0, 0, 0, 1
    );

    gl_FragColor = vec4(y, cb, cr, 1.0) * bt601;
}
*/
