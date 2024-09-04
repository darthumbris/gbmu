#include "PixelProcessingUnit.hpp"
#include <GL/gl.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <cstdlib>

// bright color, higlight color, shadow color , unlit color
constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0xC240, 0xA5A0, 0x9540, 0x8900};
constexpr uint16_t GB_COLORS_VIRTUABOY[4] = {0XFC43, 0XEC64, 0XD065, 0X8C63};
constexpr uint16_t GB_COLORS_LIGHT[4] = {0XE75D, 0XCF3D, 0XB2B7, 0X8002};
constexpr uint16_t GB_COLORS_BW[4] = {0XF7BD, 0XDAD6, 0XCA52, 0X8C63};

const GLenum format = GL_BGRA;
const GLint i_format = GL_RGB5;
const GLenum t_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;

bool load_shader(GLuint &shader_id, GLenum shader_type, const GLchar *shader_source) {
	shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1, &shader_source, NULL);
	glCompileShader(shader_id);

	GLint shader_compile;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compile);
	if (shader_compile == GL_FALSE) {
		GLchar shader_log[1024];
		int32_t error_len;
		glGetShaderInfoLog(shader_id, 1024, &error_len, shader_log);
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile shader: %s\n", shader_log);
		glDeleteShader(shader_id);
		return false;
	}
	return true;
}

bool load_shader_file(GLuint &shader_id, GLenum shader_type, const char *shader_file) {
	bool success = false;
	SDL_RWops *shader_source_raw = SDL_RWFromFile(shader_file, "r");
	if (shader_source_raw == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to open shader file: %s\n", shader_file);
		return false;
	}

	size_t len = (size_t)SDL_RWsize(shader_source_raw);
	GLchar *shader_source = (GLchar *)std::calloc(len + 1, sizeof(GLchar));
	SDL_RWread(shader_source_raw, shader_source, len, 1);
	success = load_shader(shader_id, shader_type, shader_source);
	if (!success) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, " Failed shader: %s\n", shader_file);
	}
	SDL_RWclose(shader_source_raw);
	SDL_free(shader_source);
	return success;
}

bool PixelProcessingUnit::init_gl() {
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLint success;

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
		return false;
	}

	if (SDL_GL_SetSwapInterval(1) < 0) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	data.program = glCreateProgram();

	if (!load_shader_file(vertex_shader, GL_VERTEX_SHADER, "Assets/shaders/shader.vert") ||
	    !load_shader_file(fragment_shader, GL_FRAGMENT_SHADER, "Assets/shaders/shader.frag")) {
		return false;
	}

	glAttachShader(data.program, vertex_shader);
	glAttachShader(data.program, fragment_shader);
	glLinkProgram(data.program);
	glGetProgramiv(data.program, GL_LINK_STATUS, &success);
	if (success != GL_TRUE) {
		GLchar log[1024];
		int32_t error_len;
		glGetShaderInfoLog(data.program, 1024, &error_len, log);
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to link shaders: %s\n", log);
		glDeleteProgram(data.program);
		printf("Error linking program %d error_log: %s\n", data.program, log);
		return false;
	} else {
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glUseProgram(data.program);
		data.texcoord = glGetAttribLocation(data.program, "texcoord");
		if (data.texcoord == -1) {
			printf("texcoord is not a valid glsl program variable!\n");
			return false;
		} else {
			GLfloat quad[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
			GLuint indices[] = {0, 1, 2, 3};

			glGenVertexArrays(1, &data.vao);
			glBindVertexArray(data.vao);
			glGenBuffers(1, &data.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
			glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), quad, GL_STATIC_DRAW);
			glGenBuffers(1, &data.ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);
		}
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &data.textere_id);
	glBindTexture(GL_TEXTURE_2D, data.textere_id);
	glTexImage2D(GL_TEXTURE_2D, 0, i_format, SCREEN_WIDTH, SCREEN_HEIGHT, 0, format, t_type,
	             (GLvoid *)rgb555_framebuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return true;
}

bool PixelProcessingUnit::init_window() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	} else {
		printf("data.scale: %u\n", data.scale);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		data.window = SDL_CreateWindow("GBMU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		                               SCREEN_WIDTH * data.scale, SCREEN_HEIGHT * data.scale, SDL_WINDOW_OPENGL);
		if (data.window == nullptr) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		}
	}

	data.context = SDL_GL_CreateContext(data.window);
	if (data.context == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s\n", SDL_GetError());
		SDL_DestroyWindow(data.window);
		SDL_Quit();
		return false;
	}

	SDL_GL_MakeCurrent(data.window, data.context);
	return init_gl();
}

void PixelProcessingUnit::close() {
	glDeleteProgram(data.program);
	SDL_GL_DeleteContext(data.context);
	SDL_DestroyWindow(data.window);
	data.window = nullptr;
	SDL_Quit();
}

void PixelProcessingUnit::render_texture() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, data.textere_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, format, t_type, (GLvoid *)rgb555_framebuffer);

	glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
	glEnableVertexAttribArray(data.texcoord);
	glVertexAttribPointer(data.texcoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
}

void PixelProcessingUnit::render_with_ghosting() {
	constexpr float mix_intensity = 0.5f;
	float alpha = 0.15f + (0.50f * (1.0f - mix_intensity));

	constexpr float round_error_val = 0.03f;
	static bool round_error = false;
	float round_color = 1.0f - (round_error ? round_error_val : 0.0f);
	round_error = !round_error;

	glEnable(GL_BLEND);
	glColor4f(round_color, round_color, round_color, alpha);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	render_texture();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}

void PixelProcessingUnit::render_default() {
	glDisable(GL_BLEND);
	render_texture();
}

void PixelProcessingUnit::render_screen() {
	if (!is_cgb) {
		const uint16_t *palette_used;
		switch (current_palette) {
		case 0:
			palette_used = GB_COLORS_ORIGNAL;
			break;
		case 1:
			palette_used = GB_COLORS_VIRTUABOY;
			break;
		case 2:
			palette_used = GB_COLORS_LIGHT;
			break;
		default:
			palette_used = GB_COLORS_BW;
			break;
		}
		for (int i = 0; i < SCREEN_PIXELS; i++) {
			rgb555_framebuffer[i] = palette_used[mono_framebuffer[i]];
		}
	}

	glUseProgram(data.program);

	// setting the uniform for the shader effects
	glProgramUniform1i(data.program, 0, data.matrix);
	glProgramUniform1i(data.program, 1, data.color_correction && is_cgb);
	glProgramUniform1i(data.program, 2, data.darkening);
	glProgramUniform1i(data.program, 3, data.ghosting);

	if (data.ghosting) {
		render_with_ghosting();
	} else {
		render_default();
	}

	glDisableVertexAttribArray(data.texcoord);
	glUseProgram(NULL);

	SDL_GL_SwapWindow(data.window);
	draw_screen = false;
}

void PixelProcessingUnit::increase_palette() {
	current_palette += 1;
	if (current_palette >= MAX_PALETTES) {
		current_palette = 0;
	}
}
void PixelProcessingUnit::decrease_palette() {
	if (current_palette == 0) {
		current_palette = MAX_PALETTES - 1;
	} else {
		current_palette -= 1;
	}
}