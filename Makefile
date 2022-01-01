CXX=llvm-g++
DIR=./src
INCS=-c -std=c++17 \
-I/usr/local/Cellar/glew/2.2.0_1/include \
-I/usr/local/Cellar/glfw/3.3.6/include \
-I/usr/local/Cellar/freeimage/3.18.0/include \
-I/usr/local/Cellar/glm/0.9.9.8/include \
-I/Users/YJ-work/cpp/myGL_glfw/skybox/header

LIBS=-L/usr/local/Cellar/glew/2.2.0_1/lib -lGLEW \
-L/usr/local/Cellar/glfw/3.3.6/lib -lglfw \
-L/usr/local/Cellar/freeimage/3.18.0/lib -lfreeimage \
-framework GLUT -framework OpenGL -framework Cocoa

all: main

main: main.o common.o
	$(CXX) $(LIBS) $^ -o main
	rm -f *.o

main.o: $(DIR)/main.cpp
	$(CXX) -c $(INCS) $^ -o main.o

common.o: $(DIR)/common.cpp
	$(CXX) -c $(INCS) $^ -o common.o

.PHONY: clean

clean:
	rm -vf main
