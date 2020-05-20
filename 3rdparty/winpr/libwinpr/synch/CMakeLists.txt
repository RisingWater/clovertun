# WinPR: Windows Portable Runtime
# libwinpr-synch cmake build script
#

SET(TARGET winpr-synch)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
		../thread
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_PTHREAD_GNU_EXT)
ADD_DEFINITIONS(-DHAVE_UNISTD_H)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -fPIC")

LIST(APPEND SRC_FILES 
	address.c
	barrier.c
	condition.c
	critical.c
	event.c
	init.c
	mutex.c
	semaphore.c
	sleep.c
	srw.c
	synch.c
	synch.h
	timer.c
	wait.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})

