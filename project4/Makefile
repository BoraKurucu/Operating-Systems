all: libvsimplefs.a create_format app readf createf appendf deletef

libvsimplefs.a: 	vsimplefs.c
	gcc -g -Wall -c vsimplefs.c
	ar -cvq libvsimplefs.a vsimplefs.o
	ranlib libvsimplefs.a

create_format: create_format.c
	gcc -g -Wall -o create_format  create_format.c   -L. -lvsimplefs

app: 	app.c
	gcc -g -Wall -o app app.c  -L. -lvsimplefs

readf: 	read_file.c
	gcc -g -Wall -o readf read_file.c  -L. -lvsimplefs

createf: 	create_file.c
	gcc -g -Wall -o createf create_file.c  -L. -lvsimplefs

appendf: 	append_file.c
	gcc -g -Wall -o appendf append_file.c  -L. -lvsimplefs

deletef: 	delete_file.c
	gcc -g -Wall -o deletef delete_file.c  -L. -lvsimplefs

clean: 
	rm -fr *.o *.a *~ a.out app  vdisk create_format readf createf appendf deletef
