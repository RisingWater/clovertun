# WinPR: Windows Portable Runtime
# libwinpr-registry cmake build script
#
SET(TARGET winpr-registry)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	registry_reg.c
	registry_reg.h
	registry.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})

