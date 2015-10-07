
all: libperson prog

libperson:
	gcc person.c -fPIC -shared -o libperson.so

prog:
	gcc main.c -L. -lperson -o prog

clean:
	rm -f libperson.so prog



