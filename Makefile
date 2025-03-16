# default: build

t1:
	gcc -o exe-tetris tetris/v1.cpp -lncurses

t2:
	gcc -o exe-tetris lib/utils.c tetris/v2.c

s1:
	gcc -o exe-snake lib/utils.cpp snake/v1.cpp

21:
	gcc -o exe-2048 lib/utils.cpp 2048/v1.cpp

clean:
	rm exe-*

