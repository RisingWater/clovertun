# WinPR: Windows Portable Runtime
# libwinpr-credui cmake build script
#
SET(TARGET winpr-credui)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
		../thread
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	credui.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
