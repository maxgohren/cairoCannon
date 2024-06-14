make:
	gcc -o cannon cairoCannon.c `pkg-config --cflags --libs cairo cairo-svg` -lX11 -lm
	./cannon
