Makefile: 

PROJECT_DIR = /d/Git/Image_Processing
OBJ_DIR = $(PROJECT_DIR)/objs

CC = g++

CC_FLAGS = -O2 #-Dgtest_disable_pthreads=ON 
LIBS = -lposix

#INCLUDES =
#LIB_INCLUDES = 

#INC = $(INCLUDES) $(LIBINCLUDES)  

OBJS = main.o

main.o: main.cpp 
	$(CC) $(CC_FLAGS)  -c main.cpp
	mv main.o $(OBJ_DIR)/main.o

exe: clean $(OBJS)
	$(CC) $(CC_FLAGS) $(LIBS) $(PROJECT_DIR)/a.exe  $(OBJ_DIR)/*.o
 
clean: 
	rm -f $(OBJ_DIR)/*.o