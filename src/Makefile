TARGET = mtcmd
CFLAGS = -Wall -O2 # -g -DDEBUG
CC = cc
FRAMES = -framework IOKit -framework CoreFoundation

SRC = main.c
OBJ = main.o
HDR = Ophiuchus/Ophiuchus.h
LIB = -lophiuchus
LIBDIR = Ophiuchus

all:	lib $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -L$(LIBDIR) $(LIB) $(FRAMES)

lib:
	cd Ophiuchus ; make

clean:
	/bin/rm -f *.o mtcmd
	cd Ophiuchus ; make clean

main.o: Ophiuchus/Ophiuchus.h Ophiuchus/libophiuchus.a

