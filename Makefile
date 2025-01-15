SRC= cairoCannon.c keyboard.c window.c draw.c physics.c
INCLUDE= 

make: 
	@clear
	gcc -o cannon ${SRC} `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	ctags *
	./cannon

noop:
	gcc -O0 cairoCannon.c -o cannon `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	./cannon

debug:
	gcc -g cairoCannon.c `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	

