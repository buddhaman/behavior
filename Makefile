CPP=g++
CPP_WIN=x86_64-w64-mingw32-g++
SRC=$(wildcard src/*)
EXTRA_FLAGS=-Wno-write-strings -DSIM_SLOW=1 

behavior: $(SRC)
	$(CPP) src/main.cpp external.o \
		-Iinclude \
		-I/usr/local/include/SDL2 \
		-Iform_lib \
		-Igl3wsrc \
		-ldl \
		-lSDL2 \
		-lm \
		-L/usr/local/lib \
		-pthread \
		-o behavior \
		-g \
		$(EXTRA_FLAGS) \
		-Wall

external: 
	$(CPP) src/external_libs.cpp -Iinclude -I/usr/local/include/SDL2 \
		 -c -o external.o -g -Wall

