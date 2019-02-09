CC = gcc
CFLAGS = -Wall
LIBS = -lm

SRC_DIR = src/
OBJ_DIR = build/
BIN_DIR = bin/

APP_NAME = server

$(BIN_DIR)$(APP_NAME): $(OBJ_DIR)$(APP_NAME).o
	$(CC) $(CFLAGS) $(LIBS) -o $(BIN_DIR)$(APP_NAME) $(OBJ_DIR)$(APP_NAME).o

$(OBJ_DIR)$(APP_NAME).o: $(SRC_DIR)$(APP_NAME).c
	$(CC) $(CFLAGS) -c $(SRC_DIR)$(APP_NAME).c -o $(OBJ_DIR)$(APP_NAME).o

clean: 
	rm -f $(OBJ_DIR)*.o

install:
	cp $(BIN_DIR)$(APP_NAME) /usr/bin

