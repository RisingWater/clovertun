# WinPR: Windows Portable Runtime
# libwinpr-crt cmake build script
#

SET(TARGET winpr-crt)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -fPIC")

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	alignment.c
	conversion.c
	buffer.c
	memory.c
	string.c
	unicode.c
	utf.c
	utf.h)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
