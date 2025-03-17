# default: build

t1:
	gcc -o exe-tetris tetris/v1.c -lncurses

t2:
	gcc -o exe-tetris lib/utils.c tetris/v2.c

s1:
	gcc -o exe-snake lib/utils.c snake/v1.c

21:
	gcc -o exe-2048 lib/utils.c 2048/v1.c

m1:
	gcc -o exe-mine lib/utils.c mineSweeper/v1.c

clean:
	rm exe-*

