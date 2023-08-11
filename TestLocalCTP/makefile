DEBUG = -g
CFLAGS = $(DEBUG) -Wall -c -std=c++11 
RM = rm -rf

SRCS = $(wildcard ./*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

HEADER_PATH = -I../LocalCTP/ctp_file/current/
LIB_PATH = -L../bin/linux/
LIBS = -lthosttraderapi_se
LDFLAGS = -Wl,-rpath=./

TARGET = ../bin/linux/TestLocalCTP.out

$(TARGET): $(OBJS)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) $(LDFLAGS)

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)

clean:
	$(RM) $(TARGET) *.o

