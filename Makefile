TEST_SRC = $(wildcard test_*.c)
TEST_OBJ = $(TEST_SRC:%.c=%.o)
TEST_EXE = $(TEST_SRC:%.c=%)

SRC = $(subst $(TEST_SRC),,$(wildcard *.c))
OBJS = $(SRC:%.c=%.o)
LIB = libmroutine.a

CC = gcc 
AR = ar

CFLAGS = -g -Wall -fPIC
INCLUDE =  
LDFLAGS = 

.PHONY: lib test clean

lib: $(LIB)


test: lib $(TEST_EXE)


$(LIB): $(OBJS)
	$(AR) rcv $@ $^

$(TEST_EXE):%:%.c  
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIB) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^

clean:
	rm -f $(LIB) $(OBJS) $(TEST_EXE) $(TEST_OBJ) 

