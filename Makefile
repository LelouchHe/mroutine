TEST_EXE = test
TEST_SRC = test.c
TEST_OBJ = $(TEST_SRC:%.c=%.o)

SRC = $(subst test.c,,$(wildcard *.c))
OBJS = $(SRC:%.c=%.o)
LIB = libmroutine.a

CC = gcc 
AR = ar

CFLAGS = -g -Wall -fPIC
INCLUDE = -I. 
LDFLAGS = 

.PHONY: all lib clean

all: $(LIB) $(TEST_EXE)


lib: $(LIB)


$(TEST_EXE): $(TEST_OBJ) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(LIB): $(OBJS)
	$(AR) rcv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^

clean:
	rm -f $(LIB) $(OBJS) $(TEST_EXE) $(TEST_OBJ) 

