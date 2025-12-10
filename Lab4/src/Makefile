all: libd1.so libd2.so program1 program2

libd1.so: libraries_first_impl.c
	gcc -fPIC -shared libraries_first_impl.c -o libd1.so -lm

libd2.so: libraries_second_impl.c
	gcc -fPIC -shared libraries_second_impl.c -o libd2.so -lm

program1: program1.c
	gcc program1.c -L. -ld1 -o program1 -Wl,-rpath,.

program2: program2.c
	gcc program2.c -o program2 -ldl

clean:
	rm -f *.so program1 program2