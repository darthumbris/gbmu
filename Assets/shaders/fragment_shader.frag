#version 430

#define GBC_CC_LUM 0.94f
#define GBC_CC_R   0.82f
#define GBC_CC_G   0.665f
#define GBC_CC_B   0.73f
#define GBC_CC_RG  0.125f
#define GBC_CC_RB  0.195f
#define GBC_CC_GR  0.24f
#define GBC_CC_GB  0.075f
#define GBC_CC_BR  -0.06f
#define GBC_CC_BG  0.21f

out vec4 LFragment;

in vec2 uv;

layout(binding = 0) uniform sampler2D texture_gb;

// Matrix stuff
layout(location = 0) uniform int matrix_enabled;
const float pixSizeX = 4.0; // width of LED in pixel
const float pixSizeY = 4.0; // height of LED in pixel

// Color Correction stuff
const float targetGamma = 2.2;
const float displayGammaInv = 1.0 / targetGamma;
const float rgbMax = 31.0;
const float rgbMaxInv = 1.0 / rgbMax;

const int color_correction_enabled = 1;
const float colorCorrectionBrightness = 0.5;
// layout(location = 1) uniform int color_correction_enabled;
// layout(location = 2) uniform float colorCorrectionBrightness;

void main() {
    vec3 tex_col = texture(texture_gb, uv).rgb;
    vec3 matrix_col;
    if (matrix_enabled == 1) {
        float indexX = gl_FragCoord.x - .5;
        float indexY = gl_FragCoord.y - .5;
        
        // Apply black parts of matrix
        vec2 uvDots = vec2(fract(indexX / pixSizeX), fract(indexY / pixSizeY));
        float circle = 1.0 - step(0.5, length(uvDots - .5));
        float inv_circle = step(0.5, length(uvDots - .5)) - 1.0;
        matrix_col = tex_col * circle;
    } else {
        matrix_col = tex_col;
    }
    
    // TODO add darkening filter
    // TODO add color correction
    vec3 color_corrected = matrix_col;
    
    if (color_correction_enabled == 1) {
        float adjustedGamma = targetGamma - colorCorrectionBrightness;
        
        float rFloat = pow(matrix_col.r, adjustedGamma);
        float gFloat = pow(matrix_col.g, adjustedGamma);
        float bFloat = pow(matrix_col.b, adjustedGamma);
        // // Perform colour mangling
        float rCorrect = GBC_CC_LUM * ((GBC_CC_R * rFloat) + (GBC_CC_GR * gFloat) + (GBC_CC_BR * bFloat));
        float gCorrect = GBC_CC_LUM * ((GBC_CC_RG * rFloat) + (GBC_CC_G * gFloat) + (GBC_CC_BG * bFloat));
        float bCorrect = GBC_CC_LUM * ((GBC_CC_RB * rFloat) + (GBC_CC_GB * gFloat) + (GBC_CC_B * bFloat));
        // // Range check...
        rCorrect = rCorrect > 0.0f ? rCorrect : 0.0f;
        gCorrect = gCorrect > 0.0f ? gCorrect : 0.0f;
        bCorrect = bCorrect > 0.0f ? bCorrect : 0.0f;
        // // Perform gamma compression
        rCorrect = pow(rCorrect, displayGammaInv);
        gCorrect = pow(gCorrect, displayGammaInv);
        bCorrect = pow(bCorrect, displayGammaInv);
        // // Range check...
        rCorrect = rCorrect > 1.0f ? 1.0f : rCorrect;
        gCorrect = gCorrect > 1.0f ? 1.0f : gCorrect;
        bCorrect = bCorrect > 1.0f ? 1.0f : bCorrect;
        // // Perform image darkening, if required
        // if (darkFilterLevel > 0) {
        //     darkenRgb(rCorrect, gCorrect, bCorrect);
        //     isDark = true;
        // }
        // // Convert back to 5bit unsigned
        float rFinal = uint((rCorrect * rgbMax) + 0.5) % 31;
        float gFinal = uint((gCorrect * rgbMax) + 0.5) % 31;
        float bFinal = uint((bCorrect * rgbMax) + 0.5) % 31;
        
        // color_corrected = vec3(rFinal * rgbMaxInv, gFinal * rgbMaxInv, bFinal * rgbMaxInv);
        color_corrected = vec3(rCorrect, gCorrect, bCorrect);
        
        // color_corrected = matrix_col;
    }
    LFragment = vec4(color_corrected, 1.0);
}