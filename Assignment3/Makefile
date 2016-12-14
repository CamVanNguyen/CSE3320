CC=gcc
CXX=g++
OBJ = main.o
 
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cxx
	$(CXX) -c -o $@ $< $(CFLAGS)

all: assignment3 


assignment3: $(OBJ)
	gcc -o assignment3 main.o $(CFLAGS) -lpthread


clean:
	rm -f *.o *.tgz assignment3 

submission:
	make clean
	tar -c * | gzip -n > $(USER).tgz

submit:
	make submission
	sudo /usr/local/share/CSE3320/scripts/submit.py -a $(assignment) -i $(USER).tgz -u $(USER)

verify:
	make submission
	sudo /usr/local/share/CSE3320/scripts/verify.py -a $(assignment) -i $(USER).tgz -u $(USER)
