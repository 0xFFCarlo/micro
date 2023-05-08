all:
	clang -Wall -O3 \-F /Library/Frameworks/ -I /usr/local/include/ ./src/*.c -o main -lm -framework OpenGL -framework SDL2 -framework SDL2_ttf -framework SDL2_mixer

