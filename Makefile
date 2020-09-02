

all:
	g++ -std=c++17 -g3 -I /usr/include/ main.cpp -o main -lGL -lGLEW -lglfw
