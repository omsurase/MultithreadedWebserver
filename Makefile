# CC=g++
# CFLAGS= -g -Wall -Werror

# all: proxy

# proxy: proxy.cpp
# 	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c 
# 	$(CC) $(CFLAGS) -o proxy.o -c proxy.cpp
# 	$(CC) $(CFLAGS) -o proxy proxy_parse.o proxy.o 
	

# clean:
# 	rm -f proxy *.o

# tar:
# 	tar -cvzf ass1.tgz proxy README Makefile proxy_parse.c proxy_parse.h

CC=g++
CFLAGS= -g -Wall -Werror

all: proxy2

proxy_parse.o: proxy_parse.c proxy_parse.h
	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c 

proxy2.o: proxy2.cpp proxy_parse.h
	$(CC) $(CFLAGS) -o proxy2.o -c proxy2.cpp

proxy2: proxy_parse.o proxy2.o
	$(CC) $(CFLAGS) -o proxy2 proxy_parse.o proxy2.o 

clean:
	rm -f proxy2 *.o

tar:
	tar -cvzf ass1.tgz proxy2 README Makefile proxy_parse.c proxy_parse.h

# CC=g++
# CFLAGS= -g -Wall -Werror
# all: proxy3

# proxy_parse.o: proxy_parse.c proxy_parse.h
# 	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c

# proxy3.o: proxy3.cpp proxy_parse.h
# 	$(CC) $(CFLAGS) -o proxy3.o -c proxy3.cpp

# proxy3: proxy_parse.o proxy3.o
# 	$(CC) $(CFLAGS) -o proxy3 proxy_parse.o proxy3.o -lpthread

# clean:
# 	rm -f proxy3 *.o

# tar:
# 	tar -cvzf ass1.tgz proxy3 README Makefile proxy_parse.c proxy_parse.h