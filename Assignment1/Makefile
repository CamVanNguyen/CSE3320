CC=gcc
CXX=g++
CFLAGS= -Werror -Wall
OBJ = msh.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cxx
	$(CXX) -c -o $@ $< $(CFLAGS)

all: msh 

msh: $(OBJ)
	gcc -o msh msh.o $(CFLAGS)

clean:
	rm -f *.o msh 

submission:
	make clean
	tar -c *.c Makefile | gzip -n > $(USER).tgz

submit:
	make submission
	sudo /usr/local/share/CSE3320/scripts/submit.py -a $(assignment) -i $(USER).tgz -u $(USER)

verify:
	make submission
	sudo /usr/local/share/CSE3320/scripts/verify.py -a $(assignment) -i $(USER).tgz -u $(USER)
