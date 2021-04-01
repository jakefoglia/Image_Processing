Makefile: 

PROJECT_DIR = ~/Git/Image_Processing
OBJ_DIR = $(PROJECT_DIR)/objs

CC = g++

FLAGS1 = `Magick++-config --cppflags`
FLAGS2 = `Magick++-config --ldflags --libs`

OBJS = main.o

main.o: main.cpp 
	$(CC) $(FLAGS1) -c main.cpp $(FLAGS2)
	mv main.o $(OBJ_DIR)/main.o

exe: clean $(OBJS)
	$(CC) $(FLAGS1) $(OBJ_DIR)/*.o $(FLAGS2)
 
clean: 
	rm -f $(OBJ_DIR)/*.o
