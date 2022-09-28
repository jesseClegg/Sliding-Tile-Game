#author Jesse Clegg
slidingpuzzle-v3: slidingpuzzle-v3.o sp-pipe-client.o sp-pipe-server.o
	gcc slidingpuzzle-v3.o sp-pipe-client.o sp-pipe-server.o -o slidingpuzzle-v3
slidingpuzzle-v3.o: slidingpuzzle-v3.c
	gcc -c slidingpuzzle-v3.c
sp-pipe-client.o: sp-pipe-client.c
	gcc -c sp-pipe-client.c
sp-pipe-server.o: sp-pipe-server.c
	gcc -c sp-pipe-server.c
clean:
	rm slidingpuzzle.o sp-pipe-client.o sp-pipe-server.o slidingpuzzle-v3


