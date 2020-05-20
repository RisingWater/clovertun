# WinPR: Windows Portable Runtime
# libwinpr-path cmake build script
#
# Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
#

SET(TARGET winpr-path)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	path.c
	shell.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
