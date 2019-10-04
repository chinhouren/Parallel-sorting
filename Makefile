FLAGS = -Wall -g -std=gnu99
DEPENDENCIES = helper.h

all: psort

psort: psort.o helper.o
	gcc ${FLAGS} -o $@ $^

read_sorted: read_sorted.o helper.o
	gcc ${FLAGS} -o $@ $^

mkwords: mkwords.c
	gcc ${FLAGS} -o $@ $^ -lm

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o psort mkwords
