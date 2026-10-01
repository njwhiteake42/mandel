COMPILER_FLAGS = -Wall -Wextra -Wpedantic

LINKER_FLAGS = -lSDL2

OBJ_NAME = mandel-demo

all:
	gcc $(OBJ_NAME).c $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
	
run:
	./$(OBJ_NAME)
	
clean:
	rm -f $(OBJ_NAME)
