# WinPR: Windows Portable Runtime
# libwinpr-smartcard cmake build script
#
SET(TARGET winpr-smartcard)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	smartcard.c
	smartcard.h
	smartcard_link.c
	smartcard_pcsc.c
	smartcard_pcsc.h
	smartcard_winscard.c
	smartcard_winscard.h)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
