CC = gcc
CFLAGS = -Wall
LIBS = -lm

SRC_DIR = 
OBJ_DIR = 
BIN_DIR = 

APP_NAME = mouse_client

$(BIN_DIR)$(APP_NAME): $(OBJ_DIR)$(APP_NAME).o
	$(CC) $(CFLAGS) $(LIBS) -o $(BIN_DIR)$(APP_NAME) $(OBJ_DIR)$(APP_NAME).o

$(OBJ_DIR)$(APP_NAME).o: $(SRC_DIR)$(APP_NAME).c
	$(CC) $(CFLAGS) -c $(SRC_DIR)$(APP_NAME).c -o $(OBJ_DIR)$(APP_NAME).o

clean: 
	rm -f $(OBJ_DIR)*.o

install:
	cp $(BIN_DIR)$(APP_NAME) /usr/bin

