LIBS := -lunicorn -lpthread
INCLUDE := -Iinclude/

all:
	cc -g $(LIBS) $(INCLUDE) source/*.c -o dolos2x
