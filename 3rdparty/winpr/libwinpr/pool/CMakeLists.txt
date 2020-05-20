# WinPR: Windows Portable Runtime
# libwinpr-pool cmake build script
#
SET(TARGET winpr-pool)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	synch.c
	work.c
	timer.c
	io.c
	cleanup_group.c
	pool.c
	pool.h
	callback_environment.c
	callback.c
	callback_cleanup.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
