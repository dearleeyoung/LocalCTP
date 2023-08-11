DEBUG = -g
CFLAGS = $(DEBUG) -Wall -c -fPIC -std=c++11 
RM = rm -rf

SRCS = $(wildcard ./*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

HEADER_PATH = -I./ctp_file/current/
LIB_PATH =
LIBS = -pthread

TARGET = ../bin/linux/libthosttraderapi_se.so

$(TARGET): $(OBJS)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) -shared

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)

clean:
	$(RM) $(TARGET) *.o

