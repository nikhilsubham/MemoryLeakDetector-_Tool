TARGET: exe
LinkedListApi.o: LinkedList/LinkedListApi.c
	gcc -c -I LinkedList LinkedList/LinkedListApi.c -o LinkedList/LinkedListApi.o
libmld.a: mld.o 
	ar rs libmld.a mld.o
mld.o: mld.c
	gcc -g -c -I . mld.c -o mld.o
exe: ass2.o LinkedList/LinkedListApi.o libmld.a
	gcc -g ass2.o LinkedList/LinkedListApi.o -o exe -L . -lmld 
ass2.o: ass2.c
	gcc -g -c -I . ass2.c -o ass2.o
clean:
	rm *.o
	rm LinkedList/*.o
	rm libmld.a
	rm exe

