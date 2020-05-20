# WinPR: Windows Portable Runtime
# libwinpr-credentials cmake build script
#
# Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
#
SET(TARGET winpr-credentials)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
		../thread
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	credentials.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
