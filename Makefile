visualizer: rasterizer.c
	cc rasterizer.c -g -o test -lm -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -lGLEW 

fast: rasterizer.c
	clang rasterizer.c -Ofast -o test -lm -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -lGLEW 

gcc: rasterizer.c
	gcc rasterizer.c -Ofast -o test -lm -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -lGLEW 
