LIBS := -L../unicorn-install/usr/lib/ -lunicorn -lpthread `sdl2-config --libs`
CFLAGS := -I../unicorn-install/usr/include/ -Iinclude/ `sdl2-config --cflags`

all:
	cc -g $(LIBS) $(CFLAGS) source/*.c -o dolos2x -Wl,-rpath,/home/adan/dev/gp2x/unicorn-install/usr/lib/

debug:
	cc -g $(LIBS) $(CFLAGS) -DDEBUG source/*.c -o dolos2x
