#version 430

//https://www.gc-forever.com/forums/viewtopic.php?f=37&t=2782&start=625#p38593

#define display_gamma 2.2
#define target_gamma 1.961
#define sat 1.0
#define contrast 1.0
#define blr 0.0
#define blg 0.0
#define blb 0.0
#define red 0.931954
#define green 0.763927
#define blue 0.939047
#define rg 0.0473918
#define rb 0.0257754
#define gr 0.174893
#define gb 0.0351781
#define br -0.106848
#define bg 0.188681
#define m1 0.3086
#define m2 0.6094
#define m3 0.0820

in vec2 uv;

layout(binding = 0) uniform sampler2D texture_gb;

// Matrix stuff
layout(location = 0) uniform int matrix_enabled;
#define pixSizeX  4.0f // width of LED in pixel
#define pixSizeY  4.0f // height of LED in pixel

// Color Correction stuff
layout(location = 1) uniform int color_correction_enabled;

// Darkening stuff
layout(location = 2) uniform int darkening_enabled;

out vec4 LFragment;

vec3 darkenRgb(vec3 color) {
    return color;
}

void main() {
    vec4 tex_col = texture(texture_gb, uv);
    vec4 matrix_col;
    if (matrix_enabled == 1) {
        float indexX = gl_FragCoord.x - .5;
        float indexY = gl_FragCoord.y - .5;
        
        // Apply black parts of matrix
        vec2 uvDots = vec2(fract(indexX / pixSizeX), fract(indexY / pixSizeY));
        float circle = 1.0 - step(0.5, length(uvDots - .5));
        matrix_col = tex_col * circle;
        if (circle != 1.0) {
            matrix_col = vec4(vec3(0.5), 1.0);
        }
    } else {
        matrix_col = tex_col;
    }
    
    float lum = 1.0f;
    if (darkening_enabled == 1) {
        lum = 0.25f;
    }
    
    if (color_correction_enabled == 1) {
        vec4 screen = pow(matrix_col, vec4(target_gamma));
        vec4 avg_lum = vec4(0.5);
        screen = mix(screen, avg_lum, (1.0 - contrast));
        
        mat4 color = { {
                red, gr, br, blr
            }, {
                rg, green, bg, blg
            }, {
                rb, gb, blue, blb
            }, {
                0.0, 0.0, 0.0, 1.0
            }
        };
        
        mat4 adjust = { {
                (1.0 - sat) * m1 + sat, (1.0 - sat) * m2, (1.0 - sat) * m3, 0.0
            }, {
                (1.0 - sat) * m1, (1.0 - sat) * m2 + sat, (1.0 - sat) * m3, 0.0
            }, {
                (1.0 - sat) * m1, (1.0 - sat) * m2, (1.0 - sat) * m3 + sat, 0.0
            }, {
                1.0, 1.0, 1.0, 1.0
            }
        };
        
        color = color * adjust;
        screen = clamp(screen * lum, 0.0, 1.0);
        screen = screen * color;
        LFragment = pow(screen, vec4(1.0 / display_gamma));
    }
    else {
        if (darkening_enabled == 1) {
            matrix_col = clamp(matrix_col * lum, 0.0, 1.0);
        }
        LFragment = vec4(matrix_col.rgb, 1.0);
    }
}