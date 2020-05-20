# WinPR: Windows Portable Runtime
# libwinpr-rpc cmake build script
#
SET(TARGET winpr-rpc)

#ADD_SUBDIRECTORY(test)

#包含头文件路径
LIST(APPEND DIR_PATH
		./
                ../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

ADD_DEFINITIONS(-DHAVE_UNISTD_H)

LIST(APPEND SRC_FILES 
	rpc.c
	ndr.c
	ndr_array.c
	ndr_array.h
	ndr_context.c
	ndr_context.h
	ndr_correlation.c
	ndr_correlation.h
	ndr_pointer.c
	ndr_pointer.h
	ndr_private.c
	ndr_private.h
	ndr_simple.c
	ndr_simple.h
	ndr_string.c
	ndr_string.h
	ndr_structure.c
	ndr_structure.h
	ndr_union.c
	ndr_union.h
	midl.c)

#项目生成目标
ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})
