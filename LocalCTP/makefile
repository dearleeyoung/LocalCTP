DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wno-format-security -c -fPIC -std=c++11 
CFLAGS2 = $(DEBUG) -Wall -c -fPIC 
RM = rm -rf

AUTO_GEN_PATH = ./auto_generated_code
SRCS = $(wildcard ./*.cpp  $(AUTO_GEN_PATH)/*.cpp )
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
OBJS2 = $(patsubst %.c, %.o, $(wildcard ./sqlite/*.c))

HEADER_PATH = -I./ctp_file/current/ -I$(AUTO_GEN_PATH) 
LIB_PATH =
LIBS = -pthread -ldl

TARGET = ../bin/linux/libthosttraderapi_se.so

$(TARGET): $(OBJS) $(OBJS2)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) -shared

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)

$(OBJS2):%.o :  %.c
	gcc $(CFLAGS2) $< -o $@ 

clean:
	$(RM) $(TARGET) *.o ./sqlite/*.o $(AUTO_GEN_PATH)/*.o

