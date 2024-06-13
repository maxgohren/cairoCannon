make:
	gcc -o cairosimple main.c $(pkg-config --cflags --libs cairo pango pangocairo) -lm

run:
	gcc -o cairosimple main.c -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lcairo -lm
	./cairosimple test
	xdg-open test

clean: 
	if [ -f "test" ]; then rm test; fi

