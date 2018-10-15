ARCH=64
INCLUDE=./include
SRC=./src
CC=gcc
CFLAG=-DDEBUG -DARCH=$(ARCH) -g
TARGET=mm_test rbtree_test
default:$(TARGET)
mm_test:mm_test.o $(SRC)/page.o $(SRC)/memory.o
	$(CC) -o $@ $^ $(CFLAG) -I$(INCLUDE)
rbtree_test:rbtree.o $(SRC)/page.o $(SRC)/memory.o
	$(CC) -o $@ $^ $(CFLAG) -I$(INCLUDE)
%.o:%.c
	$(CC) -c -o $@ $< $(CFLAG) -I$(INCLUDE)
clean:
	$(RM) $(TARGET) *.o $(SRC)/*.o
.PHONY:default clean mm_test rbtree_test
