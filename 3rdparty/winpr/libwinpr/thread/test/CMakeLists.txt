SET(TARGET thread-test)

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
        dummy.c)

ADD_EXECUTABLE(${TARGET} ${SRC_FILES})

#链接库
TARGET_LINK_LIBRARIES(${TARGET}
		${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/synch/libwinpr-synch.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/thread/libwinpr-thread.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/handle/libwinpr-handle.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/error/libwinpr-error.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/interlocked/libwinpr-interlocked.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/sysinfo/libwinpr-sysinfo.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/nt/libwinpr-nt.a
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/crt/libwinpr-crt.a
                pthread
                rt
		)


set(MODULE_NAME "TestThread")
set(MODULE_PREFIX "TEST_THREAD")

set(${MODULE_PREFIX}_DRIVER ${MODULE_NAME}.c)

set(${MODULE_PREFIX}_TESTS
	TestThreadCommandLineToArgv.c
	TestThreadCreateProcess.c)

create_test_sourcelist(${MODULE_PREFIX}_SRCS
	${${MODULE_PREFIX}_DRIVER}
	${${MODULE_PREFIX}_TESTS})

add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

set_complex_link_libraries(VARIABLE ${MODULE_PREFIX}_LIBS
	MONOLITHIC ${MONOLITHIC_BUILD}
	MODULE winpr
	MODULES winpr-thread winpr-heap winpr-environment winpr-synch)

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})

set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TESTING_OUTPUT_DIRECTORY}")

foreach(test ${${MODULE_PREFIX}_TESTS})
	get_filename_component(TestName ${test} NAME_WE)
	add_test(${TestName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${TestName})
endforeach()

set_property(TARGET ${MODULE_NAME} PROPERTY FOLDER "WinPR/Test")
