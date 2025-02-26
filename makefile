hw2: hw2.c

build:
	gcc -Wall -o hw2 hw2.c


rebuild:
	clean
	gcc -Wall -o hw2 hw2.c

clean:
	rm -f hw2.c


run:
	./hw2
