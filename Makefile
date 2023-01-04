all:
	cd src ; make
	mv src/mtcmd .
	strip ./mtcmd

clean:
	cd src ; make clean
	/bin/rm -f mtcmd

