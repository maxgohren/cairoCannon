make:
	gcc -o cannon cairoCannon.c `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	./cannon

noop:
	gcc -O0 cairoCannon.c -o cannon `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	./cannon

debug:
	gcc -g cairoCannon.c `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	

