CC = g++
CFLAGS = -pthread -std=c++11
BIN = ups_server
OBJ = Constants.o ValidationUtils.o StringUtils.o SendUtils.o Player.o Game.o ServerManager.o main.o

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
	$(MAKE) clean

clean:
	-rm -f *.o

# make -f makefile
# make clean
# netstat -tulpn