LIBS := -lunicorn -lpthread `sdl2-config --libs`
CFLAGS := -Iinclude/ `sdl2-config --cflags`

all:
	cc -g $(LIBS) $(CFLAGS) source/*.c -o dolos2x
