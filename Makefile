Target=epoll_key
HomeDir = .
Objects=$(HomeDir)/Epoller.o $(HomeDir)/HotKeyService.o $(HomeDir)/main.o


CC=g++
Compile=$(CC)
Link=$(CC) -lstdc++
INCLUDE = -O3 -D_LINUX -DLINUX -DDEBUG
INCLUDE += -I./
INCLUDE += -I$(ServiceDir)/

LIB=-Wall -lz -lrt -ldl -lpthread

main :  $(Objects)
	$(Link) $(Objects) $(LIB) -o $(Target)

.cpp.o :
	$(Compile) -c -o $@ $< $(INCLUDE)
.c.o :
	$(Compile) -c -o $@ $< $(INCLUDE)
clean:
	rm $(Objects)

