# default: build

t1:
	gcc -o exe-tetris tetris/v1.cpp -lncurses

t2:
	gcc -o exe-tetris tetris/v2.cpp

clean:
	rm exe-tetris

