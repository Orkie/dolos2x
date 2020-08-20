LIBS := -lunicorn -lpthread -lstdc++ -lfmt
INCLUDE := -Iinclude/

all:
	cc -g $(LIBS) $(INCLUDE) source/*.c source/*.cpp -o dolos2x
