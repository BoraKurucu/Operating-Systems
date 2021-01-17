all: libsmemlib.a  app test create_memory destroy_memory

libsmemlib.a:  smemlib.c 
	gcc -g -Wall -c smemlib.c -lm -lrt -lpthread
	ar -cvq libsmemlib.a smemlib.o
	ranlib libsmemlib.a

app: app.c
	gcc -g -Wall -o app app.c -L. -lsmemlib -lm -lrt -lpthread

test: test.c
	gcc -g -Wall -o test test.c -L. -lsmemlib -lm -lrt -lpthread

create_memory: create_memory.c
	gcc -g -Wall -o create_memory create_memory.c -L. -lsmemlib -lm -lrt -lpthread

destroy_memory: destroy_memory.c
	gcc -g -Wall -o destroy_memory destroy_memory.c -L. -lsmemlib -lm -lrt -lpthread

clean: 
	rm -fr *.o *.a *~ a.out  app test smemlib.o smemlib.a libsmemlib.a  create_memory destroy_memory
