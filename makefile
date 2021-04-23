Makefile: 


CC = g++

FLAGS1 = `Magick++-config --cppflags`
FLAGS2 = `Magick++-config --ldflags --libs`

INC = /cygdrive/c/cygwin64/usr/include/ImageMagick-7/Magick++.h


OBJS = main.o

main.o: main.cpp 
	$(CC) $(FLAGS1) -c main.cpp $(FLAGS2)
	mv main.o objs/main.o

exe: clean $(OBJS)
	$(CC) $(FLAGS1) objs/main.o $(FLAGS2)
 
clean: 
	rm -f objs/*.o
