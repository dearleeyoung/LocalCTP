DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wno-format-security -Wno-deprecated-declarations -Wno-unused-value -c -fPIC -std=c++14
CFLAGS2 = $(DEBUG) -Wall -Wno-deprecated-declarations -Wunused-value -c -fPIC 
RM = rm -rf

AUTO_GEN_PATH = ./auto_generated_code
SRCS = $(wildcard ./*.cpp  $(AUTO_GEN_PATH)/*.cpp )
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
OBJS2 = $(patsubst %.c, %.o, $(wildcard ./sqlite/*.c))

HEADER_PATH = -I./ctp_file/current/ -I$(AUTO_GEN_PATH) 
LIB_PATH =

# 检测操作系统
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS 设置
    LIBS = -pthread
    TARGET = ../bin/mac/libthosttraderapi_se.dylib
    LINK_FLAGS = -dynamiclib -install_name @rpath/libthosttraderapi_se.dylib
else
    # Linux 设置
    LIBS = -pthread -ldl
    TARGET = ../bin/linux/libthosttraderapi_se.so
    LINK_FLAGS = -shared
endif

$(TARGET): $(OBJS) $(OBJS2)
	g++ $^ -o $@ $(LIB_PATH) $(LIBS) $(LINK_FLAGS)

$(OBJS):%.o :  %.cpp
	g++ $(CFLAGS) $< -o $@ $(HEADER_PATH)
 
$(OBJS2):%.o :  %.c
	gcc $(CFLAGS2) $< -o $@ 

clean:
	$(RM) $(TARGET) *.o ./sqlite/*.o $(AUTO_GEN_PATH)/*.o

