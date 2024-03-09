CC       = g++

IFLAGS   =

DEFINES  = -DLINUX

SYS_LIBS = -lrt

DEPS     = 

COMMON_FILES = ../CommonFiles/clientServerData.cpp ../CommonFiles/clientServer.c

it: LinuxSample

LinuxSample:  LinuxSample.cpp
	$(CC) $(DEFINES) -g $(IFLAGS) $(COMMON_FILES) $< -o $@  $(SYS_LIBS)

clean:
	rm -rf cs ghd xpgh1 xpgh2
//
//这段代码是一个简单的Makefile，用于编译一个名为LinuxSample的$C++$程序。以下是该Makefile中定义的几个关键部分：
//CC：定义了编译器为$g++$。
//IFLAGS：定义了编译器的标志。
//DEFINES：定义了一个名为LINUX的宏。
//SYS_LIBS：定义了系统库为lrt。
//DEPS：定义了依赖关系。
//COMMON_FILES：定义了一些共享的cpp文件。
//it：定义了一个名为LinuxSample的目标，依赖于LinuxSample.cpp文件和COMMON_FILES中的文件，通过$g++$编译生成可执行文件LinuxSample。
//clean：定义了一个clean目标，用于删除编译生成的可执行文件和其他临时文件。
//通过运行命令“make”或“make LinuxSample”可以编译生成LinuxSample程序，运行“make clean”可以清理编译生成的文件。