
CC=g++

IDIR=./include
ODIR=./output

SFML_FLAGS=-I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-6 -pthread -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-network -lsfml-system
GLAD_FLAGS=-lassimp -lm -lGL

CFLAGS=-I$(IDIR) -Wall -ggdb $(SFML_FLAGS) $(GLAD_FLAGS)

SFILES=./src/StbImage.cpp ./src/ShaderProgram.cpp ./src/glad.c ./src/Animator.cpp ./src/AssimpImport.cpp ./src/Mesh3D.cpp ./src/Object3D.cpp

all:
	mkdir -p bin
	$(CC) $(CFLAGS) -o ./bin/pj ./src/main.cpp $(SFILES)

clean:
	rm -f ./bin/pj
