# WinPR: Windows Portable Runtime
# libwinpr-asn1 cmake build script
#
SET(TARGET winpr-asn1)

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
	asn1.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
