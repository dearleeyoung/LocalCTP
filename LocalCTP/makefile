DEBUG = -g
CFLAGS = $(DEBUG) -Wall -c -fPIC -std=c++11 
CFLAGS2 = $(DEBUG) -Wall -c -fPIC 
RM = rm -rf

SRCS = $(wildcard ./*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
OBJS2 = $(patsubst %.c, %.o, $(wildcard ./sqlite/*.c))

HEADER_PATH = -I./ctp_file/current/
LIB_PATH =
LIBS = -pthread -ldl

TARGET = ../bin/linux/libthosttraderapi_se.so

$(TARGET): $(OBJS) $(OBJS2)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) -shared

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)

$(OBJS2):%.o :  %.c
	gcc $(CFLAGS2) $< -o $@ $(HEADER_PATH)

clean:
	$(RM) $(TARGET) *.o ./sqlite/*.o

