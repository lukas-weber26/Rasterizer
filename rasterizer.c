#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define color_max 1000
#define canvas_width 1920
#define canvas_height 1080
#define viewport_width 1.0
#define viewport_height 0.56
#define canvas_depth 1.0

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
	double z;
} canvas_point;

typedef struct rgb_color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_color;

typedef struct int_arena {
	int n_items;
	int max_items;
	int * storage;
} int_arena;

typedef struct scene {
	unsigned char * screen; 
	double *depth_buffer;
	int screen_width; 
	int screen_height; 
	int_arena * scene_arena;

} scene;

canvas_point create_point(int x, int y, double z) {
	canvas_point new = {x,y, z};
	return new;
}

rgb_color create_color(unsigned char r, unsigned char g, unsigned char b) {
	rgb_color new = {r, g, b};
	return new;
}

void put_pixel_on_screen(scene s, int x, int y, rgb_color color, double z) {
	//printf("Drawing.");
	assert((x < s.screen_width) && (x >= 0));
	assert((y < s.screen_height) && (y >= 0));

	//lt may be wrong here
	if (s.depth_buffer[x+s.screen_width*y] < (1/z)) {  
		s.screen[(x+s.screen_width*y)*3] = color.r;
		s.screen[(x+s.screen_width*y)*3 + 1] = color.g;
		s.screen[(x+s.screen_width*y)*3 + 2] = color.b;

		s.depth_buffer[x+s.screen_width*y] = (1/z);  
	}

}

void put_pixels_on_canvas(scene s, canvas_point point, rgb_color color) {
	put_pixel_on_screen(s, point.x + (s.screen_width/2), point.y + (s.screen_height/2), color, point.z);
}

int_arena * int_arena_create (int size) {
	int_arena * arena = calloc(1, sizeof(int_arena));
	arena->storage = calloc(size, sizeof(int));
	arena->max_items = size;
	arena->n_items = 0;
	return arena;
}

void int_arena_free(int_arena * arena) {
	free(arena->storage);
	free(arena);
} 

void int_arena_add(int_arena * arena, int a) {
	if (arena->n_items >= arena->max_items-1) {
		arena->max_items *= 2;
		arena->storage = realloc(arena->storage, arena->max_items*sizeof(int));
		//printf("Realloc occured\n");
	}
	arena->storage[arena->n_items] = a;
	arena->n_items++;
}

typedef struct int_array {
	int n_integers;
	int offset;
	struct int_arena * arena;
} int_array;

int_array int_array_create(int_arena * arena) {
	int_array new_array = {0,arena->n_items, arena};	
	return new_array;
}

void int_array_append(int_array *array, int value) {
	int_arena_add(array->arena, value);
	array->n_integers ++;
}

int int_array_get_index(int_array array, int index) {
	return array.arena->storage[array.offset + index];
}

void int_array_drop_last(int_array * array) {
	array->n_integers -= 1;
}

int_array interpolate(scene s, int i0, int d0, int i1, int d1) {
	int_array new_array = int_array_create(s.scene_arena);
	if (i0 == i1) {
		int_array_append(&new_array, d0);
		return new_array;
	}
	double a = ((double)(d1-d0))/((double)(i1 - i0));
	double d = d0;
	for (int i = i0; i < i1; i ++) {
		int_array_append(&new_array, (int)d);
		d = d+a;
	}
	return new_array;
}

int_array arena_drop_element(scene s, int_array a){
	int_array output = a;
	output.n_integers -= 1;
	s.scene_arena->n_items -= 1;
	return output;
}

void int_array_print(int_array input) {
	for (int i = input.offset; i < input.offset + input.n_integers; i ++) {
		printf("%d,", input.arena->storage[i]);
	}
	printf("\n");
}

void drawline(scene s, canvas_point p0, canvas_point p1, rgb_color color) {
	assert((p0.x >= (-s.screen_width/2)) && (p0.x < (s.screen_width/2)));
	assert((p1.x >= (-s.screen_width/2)) && (p1.x < (s.screen_width/2)));
	assert((p0.y >= (-s.screen_height/2)) && (p0.y < (s.screen_height/2)));
	assert((p1.y >= (-s.screen_height/2)) && (p1.y < (s.screen_height/2)));

	int_array draw_array; 

	if (abs(p1.x - p0.x) > abs(p1.y - p0.y)) {
		if (p0.x > p1.x) {
			canvas_point temp = p0;
			p0 = p1;
			p1 = temp;
		}
		draw_array = interpolate(s, p0.x, p0.y, p1.x, p1.y);
		for (int x = p0.x; x < p1.x; x++) {
			put_pixels_on_canvas(s, create_point(x,int_array_get_index(draw_array,x-p0.x), 0.0), color); //this needs to interpolate z 
		}
	} else {
		if (p0.y > p1.y) {
			canvas_point temp = p0;
			p0 = p1;
			p1 = temp;
		}
		draw_array = interpolate(s,p0.y, p0.x, p1.y, p1.x);
		for (int y = p0.y; y < p1.y; y++) {
			put_pixels_on_canvas(s, create_point(int_array_get_index(draw_array, y-p0.y), y, 0.0), color); //this needs to interpolate z
		}
	}

}

typedef struct triangle {
	canvas_point p1;	
	canvas_point p2;	
	canvas_point p3;	
	rgb_color color;
	rgb_color outline_color;
	int p1l;
	int p2l;
	int p3l;
} triangle; 

typedef struct coord {
	double x;
	double y;
	double z;
	double w;
} coord;

typedef struct matrix {
	coord a;
	coord b;
	coord c;
	coord d;
} matrix;

int coord_equal(coord a, coord b) {
	if (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w) {
		return 1;
	}
	return 0;
}

void coord_print(coord a) {
	printf("coord: %f %f %f %f\n", a.x, a.y, a.z, a.w);
} 

coord coord_create(double x, double y, double z, double w) {
	coord output = {x,y,z, w};
	return output;
} 

coord coord_scale(coord input, double scale) {
	coord output = {input.x*scale,input.y*scale,input.z*scale, input.w*scale};
	return output;
} 

double coord_dot(coord a, coord b) {
	double output = a.x*b.x + a.y*b.y + a.z*b.z + a.w + b.w;
	return output;
} 

coord coord_add(coord a, coord b) {
	coord output = {a.x+b.x,a.y+b.y,a.z+b.z, a.w+b.w};
	return output;
} 

coord coord_sub(coord a, coord b) {
	coord output = {a.x-b.x,a.y-b.y,a.z-b.z, a.w-b.w};
	return output;
}

coord coord_normalize(coord a) {
	if (a.w * a.w < 0.001) {
		return a;
	}
	coord output = {a.x/a.w,a.y/a.w,a.z/a.w,a.w/a.w};
	return output;
}

double coord_length(coord a) {
	double output = sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
	return output;
}

coord coord_to_viewport(coord a) {
	double x = (a.x*canvas_depth)/a.z;
	double y = (a.y*canvas_depth)/a.z;
	coord result = {x, y, a.z, 0.0}; //z used to be canvas_depth
	return result;
}

canvas_point coord_to_canvas(coord coordinate) {
	int x_coord = (int)(coordinate.x*(((double)canvas_width)/viewport_width));
	int y_coord = (int)(coordinate.y*(((double)canvas_height)/viewport_height));
	canvas_point result = {x_coord,y_coord};
	return result;
}

matrix matrix_fip(matrix m) {
	matrix result;
	result.a = coord_create(m.a.x, m.b.x, m.c.x, m.d.x);
	result.b = coord_create(m.a.y, m.b.y, m.c.y, m.d.y);
	result.c = coord_create(m.a.z, m.b.z, m.c.z, m.d.z);
	result.d = coord_create(m.a.w, m.b.w, m.c.w, m.d.w);
	return result;
}

matrix matrix_scale(matrix m, double s) {
	matrix result = {coord_scale(m.a, s), coord_scale(m.b, s), coord_scale(m.c, s), coord_scale(m.d, s)};
	return result;
}

coord matrix_vector_mul(matrix m, coord v) {
	matrix temp = matrix_fip(m);
	coord result = {coord_dot(temp.a, v), coord_dot(temp.b, v), coord_dot(temp.c, v), coord_dot(temp.d, v)};
	return result;
}

matrix matrix_normalize(matrix m) {
	double val = m.d.w;
	if (val*val < 0.001) {
		return m;
	}
	return matrix_scale(m, (1/val));
}

matrix matrix_matrix_mul(matrix m, matrix b) {
	matrix a = matrix_fip(m);
	matrix result;
	//ugh
	return result;
}

int_array int_array_cat(int_array a, int_array b) {
	assert(a.arena == b.arena);
	assert(a.offset + a.n_integers == b.offset);
	int_array result;
	result.offset = a.offset; 
	result.arena = a.arena;
	result.n_integers =  a.n_integers + b.n_integers;
	return result;
}

void draw_triangle_outline(scene s, triangle t) {
	//outline
	drawline(s, t.p1, t.p2, t.outline_color);
	drawline(s, t.p1, t.p3, t.outline_color);
	drawline(s, t.p2, t.p3, t.outline_color);
}

rgb_color color_scale(rgb_color base_color, int scale) {
	rgb_color result;
	result.r = (unsigned char)(((int)base_color.r * scale)/color_max);
	result.g = (unsigned char)(((int)base_color.g * scale)/color_max);
	result.b = (unsigned char)(((int)base_color.b * scale)/color_max);
	return result;
}

void draw_triangle_interior(scene s, triangle t, int h1, int h2, int h3) {
	assert(h1 < color_max && h2 < color_max && h3 < color_max);
	assert(t.p1.y<= t.p2.y);
	assert(t.p2.y <= t.p3.y);

	//x points
	int_array x01 = interpolate(s, t.p1.y, t.p1.x, t.p2.y, t.p2.x);	
	int_array x12 = interpolate(s, t.p2.y, t.p2.x, t.p3.y, t.p3.x);	
	int_array x012 = int_array_cat(x01, x12);
	int_array x02 = interpolate(s, t.p1.y, t.p1.x, t.p3.y, t.p3.x);	
	
	int_array z01 = interpolate(s, t.p1.y, (int)(t.p1.z*1000), t.p2.y, (int)(t.p2.z*1000));	
	int_array z12 = interpolate(s, t.p2.y, (int)(t.p2.z*1000), t.p3.y, (int)(t.p3.z*1000));	
	int_array z012 = int_array_cat(z01, z12);
	int_array z02 = interpolate(s, t.p1.y, (int)(t.p1.z*1000), t.p3.y, (int)(t.p3.z*1000));	

	//h points note, mixing these in with x point computations will lead to disaster
	int_array h01	= interpolate(s, t.p1.y, h1, t.p2.y, h2);
	int_array h12	= interpolate(s, t.p2.y, h2, t.p3.y, h3);
	int_array h012 = int_array_cat(h01, h12);
	int_array h02 = interpolate(s, t.p1.y, h1, t.p3.y, h3);	

	//determine which item is left 
	int_array x_left = x012;
	int_array x_right = x02;
	
	int_array z_left = z012;
	int_array z_right = z02;

	int_array h_left = h012;
	int_array h_right = h02;

	int m = (int)floor(((double)x012.n_integers)/2.0);
	if (int_array_get_index(x02,m) < int_array_get_index(x012,m)) {
		x_left = x02;
		x_right = x012;

		h_left = h02;
		h_right = h012;

		z_left = z02;
		z_right = z012;
	}

	for (int y = t.p1.y; y < t.p3.y; y++) {
		int xl = int_array_get_index(x_left, y-t.p1.y);
		int xr = int_array_get_index(x_right, y-t.p1.y);

		int hl = int_array_get_index(h_left, y-t.p1.y);
		int hr = int_array_get_index(h_right, y-t.p1.y);
		
		int zl = int_array_get_index(z_left, y-t.p1.y);
		int zr = int_array_get_index(z_right, y-t.p1.y);

		int_array h_segment = interpolate(s, xl, hl, xr, hr);
		int_array z_segment = interpolate(s, xl, zl, xr, zr);

		for (int x = xl; x < xr; x++) {
			rgb_color shaded_color = color_scale(t.color,int_array_get_index(h_segment, x-xl)); 
			double z = (double)(int_array_get_index(z_segment, x-xl));
			put_pixels_on_canvas(s, create_point(x,y,z), shaded_color);
		}
	}	
}

void draw_triangle(scene s, triangle t) {
	draw_triangle_interior(s, t, t.p1l, t.p2l, t.p3l);
	//draw_triangle_interior(s, t, 10, 500, 990);
	//draw_triangle_outline(s,t);
}

triangle triangle_create (int x1, int y1, double z1,int x2, int y2, double z2, int x3, int y3, double z3, unsigned char r, unsigned char g, unsigned char b, unsigned char r2, unsigned char g2, unsigned char b2) {
	canvas_point p1 = create_point(x1,y1, z1);
	canvas_point p2 = create_point(x2,y2, z2);
	canvas_point p3 = create_point(x3,y3, z3);
	canvas_point temp;

	if (p1.y > p2.y) {
		temp = p1; 
		p1 = p2;
		p2 = temp;
	}
	
	if (p2.y > p3.y) {
		temp = p2; 
		p2 = p3;
		p3 = temp;
	}
	
	if (p1.y > p2.y) {
		temp = p1; 
		p1 = p2;
		p2 = temp;
	}

	triangle new_triangle = {p1,p2,p3,create_color(r,g,b), create_color(r2,g2,b2)};
	return new_triangle;
}

void triangle_from_3d(scene s) {
	canvas_point one = coord_to_canvas(coord_to_viewport(coord_create(-0.25, -0.25, 2.0, 0.0)));
	canvas_point two = coord_to_canvas(coord_to_viewport(coord_create(0.25, -0.25, 2.0, 0.0)));
	canvas_point three= coord_to_canvas(coord_to_viewport(coord_create(0.0, 0.25, 2.0, 0.0)));
	rgb_color interior = {240, 21, 14};
	rgb_color exterior = {240, 21, 14};
	triangle t = {one, two, three, interior, exterior};	

	draw_triangle_interior(s, t, 10, 500, 990);
	draw_triangle_outline(s,t);
}

void cube_from_3d(scene s) {
	canvas_point one = coord_to_canvas(coord_to_viewport(coord_create(-0.25, -0.25, 2.0, 0.0)));
	canvas_point two = coord_to_canvas(coord_to_viewport(coord_create(0.25, -0.25, 2.0, 0.0)));
	canvas_point three= coord_to_canvas(coord_to_viewport(coord_create(0.25, 0.25, 2.0, 0.0)));
	canvas_point four = coord_to_canvas(coord_to_viewport(coord_create(-0.25, 0.25, 2.0, 0.0)));
	
	canvas_point one_rear = coord_to_canvas(coord_to_viewport(coord_create(-0.25, -0.25, 4.0, 0.0)));
	canvas_point two_rear  = coord_to_canvas(coord_to_viewport(coord_create(0.25, -0.25, 4.0, 0.0)));
	canvas_point three_rear = coord_to_canvas(coord_to_viewport(coord_create(0.25, 0.25, 4.0, 0.0)));
	canvas_point four_rear = coord_to_canvas(coord_to_viewport(coord_create(-0.25, 0.25, 4.0, 0.0)));

	rgb_color color = {240, 21, 14};

	drawline(s, one, two, color);
	drawline(s, two, three, color);
	drawline(s, three, four, color);
	drawline(s, four, one, color);
	
	drawline(s, one_rear, two_rear, color);
	drawline(s, two_rear, three_rear, color);
	drawline(s, three_rear, four_rear, color);
	drawline(s, four_rear, one_rear, color);
	
	drawline(s, one, one_rear, color);
	drawline(s, two, two_rear, color);
	drawline(s, three, three_rear, color);
	drawline(s, four, four_rear, color);
}

typedef struct raw_triangle {
	coord a;
	coord b;
	coord c;
} raw_triangle;

raw_triangle raw_triangle_create(coord a, coord b, coord c) {
	if (!(!coord_equal(a,b) && !coord_equal(b,c) && !coord_equal(a,c))) {
		printf("Triangle has three equal coordd\n");
		coord_print(a);
		coord_print(b);
		coord_print(c);
		exit(0);
	};
	raw_triangle output = {a,b,c};
	return output;
}

coord coord_unit(coord intput_coord) {
	double len = coord_length(intput_coord);
	coord output = coord_scale(intput_coord, 1/len);
	return output;
}

double coord_project(coord n, coord l) {
	double dot = n.x*l.x + n.y*l.y + n.z*l.z;
	double len = (coord_length(n)* coord_length(l));
	double output = dot/len;
	assert(output*output < 1.01);
	return output;
} 

int get_lighting (coord input_coord, coord normal) {
	int max_intensity = 999;

	//assume a single directional light 
	coord light_vector = {0.4, 0.4, 1.0, 0.0};
	light_vector = coord_unit(light_vector);

	double diffuse = coord_project(normal, light_vector);
	if (diffuse < 0.01) {
		diffuse = 0;
	} 
	assert(diffuse < 1.01);

	coord R = coord_sub(coord_scale(normal, 2*coord_dot(normal,light_vector)), light_vector); 
	coord V = coord_scale(input_coord, -1.0); //from point to camera;

	R.w = 0; 
	V.w = 0;
	
	//coord_print(R);
	//coord_print(V);

	double specular = coord_project(R,V);
	specular = specular > 0 ? specular : 0;
	double specular_power = specular;

	for (int i = 10; i > 0; i --) {
		specular_power *= specular;
	}

	//specular = specular_power*10;

	assert(specular < 1.01);
	printf("Specular: %f\n", specular);
	if (specular < 0.01) {
		specular = 0;
	} 

	double intensity = 0.2 + 0.4*diffuse + 0.4*specular; 
	//printf("Intensity intesity: %f\n", intensity);

	int return_intensity =	(int)(intensity*((double)max_intensity));
	
	return return_intensity;
}

coord coord_cross(coord a,coord b) {
	coord output = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x, 0.0};
	output = coord_unit(output);
	return output;
}

coord get_triangle_normal(raw_triangle t) {
	coord l1 = coord_sub(t.b,t.a);
	coord l2 = coord_sub(t.c,t.a);
	coord output = coord_unit(coord_cross(l1,l2));
	return output;
}

triangle raw_to_processed_triangle(raw_triangle t, rgb_color red, rgb_color blue) {
	triangle processed_triangle;
	coord normal_vec = get_triangle_normal(t);

	processed_triangle.color = red;
	processed_triangle.outline_color = blue;

	processed_triangle.p1 = coord_to_canvas(coord_to_viewport(t.a));
	processed_triangle.p2 = coord_to_canvas(coord_to_viewport(t.b));
	processed_triangle.p3 = coord_to_canvas(coord_to_viewport(t.c));

	//need to calculate lighting for these
	processed_triangle.p1l = get_lighting(t.a,normal_vec);
	processed_triangle.p2l = get_lighting(t.b, normal_vec);
	processed_triangle.p3l = get_lighting(t.c, normal_vec);
	int temp_light;

	canvas_point temp;

	if (processed_triangle.p1.y > processed_triangle.p2.y) {
		temp = processed_triangle.p1; 
		processed_triangle.p1 = processed_triangle.p2;
		processed_triangle.p2 = temp;

		temp_light = processed_triangle.p1l;
		processed_triangle.p1l = processed_triangle.p2l;
		processed_triangle.p2l = temp_light;
	}
	
	if (processed_triangle.p2.y > processed_triangle.p3.y) {
		temp = processed_triangle.p2; 
		processed_triangle.p2 = processed_triangle.p3;
		processed_triangle.p3 = temp;
		
		temp_light = processed_triangle.p2l;
		processed_triangle.p2l = processed_triangle.p3l;
		processed_triangle.p3l = temp_light;
	}
	
	if (processed_triangle.p1.y > processed_triangle.p2.y) {
		temp = processed_triangle.p1; 
		processed_triangle.p1 = processed_triangle.p2;
		processed_triangle.p2 = temp;
		
		temp_light = processed_triangle.p1l;
		processed_triangle.p1l = processed_triangle.p2l;
		processed_triangle.p2l = temp_light;
	}
	return processed_triangle;	
}

void pyramid(scene s) {
	rgb_color red = {244, 23, 43};
	rgb_color blue = {23, 43, 243};

	double offset = 0.2;
	coord points [4] = {
		coord_create(0.0, -0.25, 2.0 , 0.0),
		coord_create(-0.25, -0.25, 2.3 , 0.0),
		coord_create(0.25, -0.25, 2.3 , 0.0),
		coord_create(0.0, 0.25, 2.15 , 0.0),
	};

	enum {FRONT = 0, LEFT = 1, RIGHT = 2, TOP = 3};
	raw_triangle triangles [4] = {
		raw_triangle_create(points[LEFT], points[FRONT], points[RIGHT]),
		raw_triangle_create(points[RIGHT], points[FRONT], points[TOP]),
		raw_triangle_create(points[LEFT], points[FRONT], points[TOP]),
		raw_triangle_create(points[TOP], points[RIGHT], points[LEFT]),
	}; 

	triangle final_triangles [4];
	for (int i = 0; i < 4; i++) {
		final_triangles[i] = raw_to_processed_triangle(triangles[i], red, blue);	
	}	
	
	for (int i = 0; i < 4; i++) {
		draw_triangle(s, final_triangles[i]);
	}	

}

void tirangle_cube(scene s) {
	rgb_color red = {244, 23, 43};
	rgb_color blue = {23, 43, 243};

	double offset = 0.2;
	coord points [8] = {
		coord_create(-0.25, -0.25, 2.0 , 0.0),
		coord_create(0.25, -0.25, 2.0 , 0.0),
		coord_create(-0.25, 0.25, 2.0 , 0.0),
		coord_create(0.25, 0.25, 2.0 , 0.0),
		coord_create(-0.25, -0.25, 2.5 , 0.0),
		coord_create(0.25, -0.25, 2.5 , 0.0),
		coord_create(-0.25, 0.25, 2.5 , 0.0),
		coord_create(0.25, 0.25, 2.5 , 0.0),
	};

	enum {BLF = 0, BRF = 1, TLF = 2, TRF = 3, BLB = 4, BRB = 5, TLB = 6, TRB = 7};
	raw_triangle triangles [12] = {
		raw_triangle_create(points[BLF], points[BRF], points[TRF]),
		raw_triangle_create(points[TRF], points[TLF], points[BLF]),
		raw_triangle_create(points[BLB], points[BRB], points[TRB]),
		raw_triangle_create(points[TRB], points[TLB], points[BLB]),
		raw_triangle_create(points[BRF], points[BLF], points[BLB]),
		raw_triangle_create(points[BLB], points[BRB], points[BRF]),
		raw_triangle_create(points[TRF], points[TLF], points[TLB]),
		raw_triangle_create(points[TLB], points[TRB], points[TRF]),
		raw_triangle_create(points[BRF], points[TRB], points[BRB]),
		raw_triangle_create(points[TRF], points[TRB], points[BRF]),
		raw_triangle_create(points[BLF], points[TLB], points[BLB]),
		raw_triangle_create(points[TLF], points[TLB], points[BLF]),
	}; 

	triangle final_triangles [12];
	for (int i = 0; i < 12; i++) {
		final_triangles[i] = raw_to_processed_triangle(triangles[i], red, blue);	
	}	
	
	for (int i = 0; i < 12; i++) {
		draw_triangle(s, final_triangles[i]);
	}	

}

void clear_scene(scene * s, rgb_color background_color) {
	for (int i = 0; i < 1920*1080; i++) {
		s->screen[i] = 0;
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
	new_scene.depth_buffer = calloc(new_scene.screen_width * new_scene.screen_height, sizeof(double));
	new_scene.screen = calloc(3*new_scene.screen_height*new_scene.screen_width, sizeof(unsigned char));
	new_scene.scene_arena = int_arena_create(10000);

	//triangle new_triangle = triangle_create(-300, -300, 300, -300, 0, 300, 20, 160, 20, 0, 0, 0);
	//draw_triangle(new_scene, new_triangle); 
	//triangle_from_3d(new_scene);
	//cube_from_3d(new_scene);
	//tirangle_cube(new_scene); 
	pyramid(new_scene);

	GLFWwindow  * window = opengl_init(&VAO, &program, &texture);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB ,GL_UNSIGNED_BYTE, new_scene.screen);
	glGenerateMipmap(GL_TEXTURE_2D);
	rgb_color background = {0,0,0};

	int i = 0;

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
			
		i++;
		if (i > 1000) {
			exit(0);
		}

	}

	glfwTerminate();
	int_arena_free(new_scene.scene_arena);
	free(new_scene.screen);
	free(new_scene.depth_buffer);
}
