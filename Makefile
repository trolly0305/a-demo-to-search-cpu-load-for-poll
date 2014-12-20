notify_test: notify.o
	gcc -g -o notify_test notify.o -pthread 

notify.o: notify.c
	gcc -g -c -o notify.o notify.c

clean:
	rm ./notify.o ./notify_test
