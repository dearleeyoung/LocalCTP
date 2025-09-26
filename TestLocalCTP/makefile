DEBUG = -g
CFLAGS = $(DEBUG) -Wall -c -std=c++14
RM = rm -rf

SRCS = $(wildcard ./*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

HEADER_PATH = -I../LocalCTP/ctp_file/current/ -I../LocalCTP/

# 检测操作系统
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS 设置
    LIB_PATH = -L../bin/mac/
    LIBS = -lthosttraderapi_se -lthostmduserapi_se -lssl -lcrypto -lcomunicationkey
    LDFLAGS = -Wl,-rpath,@executable_path/.
    TARGET = ../bin/mac/TestLocalCTP
    TARGET2 = ../bin/mac/TestLocalCTP2
    # TestLocalCTP2 需要额外的LocalCTP对象文件
else
    # Linux 设置
    LIB_PATH = -L../bin/linux/
    LIBS = -lthosttraderapi_se -lthostmduserapi_se
    LDFLAGS = -Wl,-rpath=./
    TARGET = ../bin/linux/TestLocalCTP.out
    TARGET2 = ../bin/linux/TestLocalCTP2.out
    # TestLocalCTP2 需要额外的LocalCTP对象文件
   
endif

all: $(TARGET) $(TARGET2)

$(TARGET): TestLocalCTP.o
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) $(LDFLAGS)

$(TARGET2): TestLocalCTP2.o $(LOCALCTP_OBJS)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) $(LDFLAGS)

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)

TestLocalCTP2: $(TARGET2)

.PHONY: all clean TestLocalCTP2

clean:
	$(RM) $(TARGET) $(TARGET2) *.o

