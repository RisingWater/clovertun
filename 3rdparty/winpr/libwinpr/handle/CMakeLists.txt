# WinPR: Windows Portable Runtime
# libwinpr-handle cmake build script
#

SET(TARGET winpr-handle)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -fPIC")

LIST(APPEND SRC_FILES 
	handle.c
	handle.h
	table.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
