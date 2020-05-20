SET(TARGET synch-test)

#包含头文件路径
LIST(APPEND DIR_PATH
                ../../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

LIST(APPEND SRC_FILES 
	TestSynchEvent.c
	TestSynchMutex.c
	TestSynchCritical.c
	TestSynchSemaphore.c
	TestSynchThread.c
	TestSynchTimerQueue.c
	TestSynchWaitableTimer.c
	TestSynchWaitableTimerAPC.c
	../../dummy.c)

ADD_EXECUTABLE(${TARGET} ${SRC_FILES})

#链接库
TARGET_LINK_LIBRARIES(${TARGET} ${LIB_FILES})
