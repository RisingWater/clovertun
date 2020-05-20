# WinPR: Windows Portable Runtime
# libwinpr-pipe cmake build script
#
SET(TARGET winpr-pipe)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	pipe.c
	pipe.h)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
