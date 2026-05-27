#pragma once

#include <span>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gl.h"

struct transform {
	float pos_x, pos_y;
	float size;
};

struct renderer {
	float r, g, b, a;
};

struct instance {
	transform transform;
	renderer renderer;
};

class graphics {
public:
	graphics(graphics&) = delete;
	graphics& operator=(graphics&) = delete;
	graphics(graphics&&) = delete;
	graphics& operator=(graphics&&) = delete;

	graphics(const char* name) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		m_window = glfwCreateWindow(1080, 720, name, nullptr, nullptr); // NOLINT (cppcoreguidelines-prefer-member-initializer)
		glfwMakeContextCurrent(m_window);
		gladLoadGL(glfwGetProcAddress);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		constexpr float vertices[] = { -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		glGenBuffers(1, &m_vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &m_instance_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_instance_buffer);
		// NOLINTBEGIN (performance-no-int-to-ptr)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(instance), reinterpret_cast<void*>(offsetof(instance, transform.pos_x)));
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(instance), reinterpret_cast<void*>(offsetof(instance, transform.size)));
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(instance), reinterpret_cast<void*>(offsetof(instance, renderer)));
		// NOLINTEND
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);

		constexpr const char* vertex_src = R"(#version 330 core
		    layout(location = 0) in vec2 vertex;
		    layout(location = 1) in vec2 position;
		    layout(location = 2) in float size;
		    layout(location = 3) in vec4 color;
			out vec4 col;
			out vec2 uv;
		    void main() {
				col = color;
				uv = vertex;
				vec2 p = 2.0 * (vertex * size + position) / vec2(1080.0, 720.0);
				gl_Position = vec4(p, 0.0, 1.0);
			}
		)";
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vertex_src, nullptr);
		glCompileShader(vertex_shader);

		constexpr const char* fragment_src = R"(#version 330 core
		    out vec4 outColor;
			in vec4 col;
			in vec2 uv;
			void main() {
			    if(dot(uv,uv) > 0.25) discard;
			    outColor = col;
			}
		)";
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_src, nullptr);
		glCompileShader(fragment_shader);

		m_program = glCreateProgram(); // NOLINT (cppcoreguidelines-prefer-member-initializer)
		glAttachShader(m_program, vertex_shader);
		glAttachShader(m_program, fragment_shader);
		glLinkProgram(m_program);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glUseProgram(m_program);
	}

	~graphics() {
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vertex_buffer);
		glDeleteBuffers(1, &m_instance_buffer);
		glDeleteProgram(m_program);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	bool shouldClose() { return glfwWindowShouldClose(m_window); }

	void draw(std::span<instance> instances) {
		glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(instances.size() * sizeof(instance)), instances.data(), GL_DYNAMIC_DRAW);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GLsizei(instances.size()));
		glfwSwapBuffers(m_window);
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
	}

private:
	GLFWwindow* m_window;
	GLuint m_vao = 0;
	GLuint m_vertex_buffer = 0;
	GLuint m_instance_buffer = 0;
	GLuint m_program = 0;
};
