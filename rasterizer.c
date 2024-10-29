#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void window_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);
}

void handle_close(GLFWwindow * window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE); //changed 1 to gl_true
	}
}

GLFWwindow * opengl_init(unsigned int * VAO, unsigned int * program, unsigned int * texture) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(1920, 1080, "Canvas", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, window_size_callback);
	glfwPollEvents();

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0,0, 1920, 1080);
	
	float verts [] = {
		-1.0, -1.0,0,0,
		1.0,-1.0,1.0,0,
		-1.0,1.0,0,1.0,
		1.0,-1.0,1.0,0,
		1.0,1.0,1.0,1.0,
		-1.0,1.0,0,1.0
	};

	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);	
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	int success;
	const char * vertex_shader_source = "#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"layout (location = 1) in vec2 aTexCoord;\n"
	"out vec2 TexCoord;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
	"TexCoord = aTexCoord;"
	"}\0";

	const char * fragment_shader_source = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec2 TexCoord;\n"
	"uniform sampler2D ourTexture;\n"
	"void main()\n"
	"{\n"
	"FragColor = texture(ourTexture,TexCoord);\n"
	"}\0";

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	assert(success);

	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	assert(success);

	*program = glCreateProgram();
	glAttachShader(*program, vertex_shader);
	glAttachShader(*program, fragment_shader);
	glLinkProgram(*program);
	glGetProgramiv(*program, GL_LINK_STATUS, &success);
	assert(success);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glUseProgram(*program);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void * )0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void * )(2 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//texture stuff
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return window;
}

typedef struct canvas_point {
	int x; 
	int y;
} canvas_point;

typedef struct rgb_color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_color;

typedef struct scene {
	unsigned char * screen; 
	int screen_width; 
	int screen_height; 
} scene;

canvas_point create_point(int x, int y) {
	canvas_point new = {x,y};
	return new;
}

rgb_color create_color(unsigned char r, unsigned char g, unsigned char b) {
	rgb_color new = {r, g, b};
	return new;
}

void put_pixel_on_screen(scene s, int x, int y, rgb_color color) {
	//printf("Drawing.");
	assert((x < s.screen_width) && (x >= 0));
	assert((y < s.screen_height) && (y >= 0));

	s.screen[(x+s.screen_width*y)*3] = color.r;
	s.screen[(x+s.screen_width*y)*3 + 1] = color.g;
	s.screen[(x+s.screen_width*y)*3 + 2] = color.b;
}

void put_pixels_on_canvas(scene s, canvas_point point, rgb_color color) {
	put_pixel_on_screen(s, point.x + (s.screen_width/2), point.y + (s.screen_height/2), color);
}

void drawline(scene s, canvas_point p0, canvas_point p1, rgb_color color) {
	assert((p0.x >= (-s.screen_width/2)) && (p0.x < (s.screen_width/2)));
	assert((p1.x >= (-s.screen_width/2)) && (p1.x < (s.screen_width/2)));
	assert((p0.y >= (-s.screen_height/2)) && (p0.y < (s.screen_height/2)));
	assert((p1.y >= (-s.screen_height/2)) && (p1.y < (s.screen_height/2)));

	if (p0.x > p1.x) {
		drawline(s, p1, p0, color);
	}
	
	double a = ((double)(p1.y - p0.y))/((double)(p1.x-p0.x));
	double y = p0.y;

	for (int x = p0.x; x < p1.x; x++) {
		canvas_point point = create_point(x,(int)y);
		put_pixels_on_canvas(s, point, color);	
		y = y + a;
	}

}

int main() {	
	unsigned char * screen = calloc(3*1920*1080,sizeof(unsigned char));

	unsigned int VAO;	
	unsigned int program; 
	unsigned int texture;

	scene new_scene;
	new_scene.screen_width = 1920;
	new_scene.screen_height = 1080;
	new_scene.screen = calloc(3*new_scene.screen_height*new_scene.screen_width, sizeof(unsigned char));

	drawline(new_scene, create_point(200,200), create_point(-400,-400), create_color(240,32,15));

	GLFWwindow  * window = opengl_init(&VAO, &program, &texture);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB ,GL_UNSIGNED_BYTE, new_scene.screen);
	glGenerateMipmap(GL_TEXTURE_2D);

	while(!glfwWindowShouldClose(window)) {
		handle_close(window);
		glClearColor(0.3, 0.6, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glBindVertexArray(VAO);
		glUseProgram(program);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	free(new_scene.screen);
}
