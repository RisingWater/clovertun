SET(TARGET asn1-test)

#包含头文件路径
LIST(APPEND DIR_PATH
                ../../../include
		)

INCLUDE_DIRECTORIES(${DIR_PATH})

LIST(APPEND SRC_FILES 
	TestAsn1Module.c
	TestAsn1Encoder.c
	TestAsn1Decoder.c
	TestAsn1Encode.c
	TestAsn1Decode.c
	TestAsn1String.c
	TestAsn1Integer.c
	TestAsn1Compare.c
	TestAsn1BerEnc.c
	TestAsn1BerDec.c
	TestAsn1DerEnc.c
	TestAsn1DerDec.c
        ../../dummy.c)

ADD_EXECUTABLE(${TARGET} ${SRC_FILES})

#链接库
TARGET_LINK_LIBRARIES(${TARGET} ${LIB_FILES})
