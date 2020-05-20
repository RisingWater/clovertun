# WinPR: Windows Portable Runtime
# winpr cmake build script
#

LIST(APPEND LIB_FILES 
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/asn1/libwinpr-asn1.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/bcrypt/libwinpr-bcrypt.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/com/libwinpr-com.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/credentials/libwinpr-credentials.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/credui/libwinpr-credui.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/crt/libwinpr-crt.a
    #${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/crypto/libwinpr-crypto.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/dsparse/libwinpr-dsparse.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/environment/libwinpr-environment.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/error/libwinpr-error.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/file/libwinpr-file.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/handle/libwinpr-handle.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/heap/libwinpr-heap.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/input/libwinpr-input.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/interlocked/libwinpr-interlocked.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/io/libwinpr-io.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/library/libwinpr-library.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/locale/libwinpr-locale.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/memory/libwinpr-memory.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/nt/libwinpr-nt.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/path/libwinpr-path.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pipe/libwinpr-pipe.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pool/libwinpr-pool.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/registry/libwinpr-registry.a
    #${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/rpc/libwinpr-rpc.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/security/libwinpr-security.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/smartcard/libwinpr-smartcard.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/sspi/libwinpr-sspi.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/sspicli/libwinpr-sspicli.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/synch/libwinpr-synch.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/sysinfo/libwinpr-sysinfo.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/thread/libwinpr-thread.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/timezone/libwinpr-timezone.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/utils/libwinpr-utils.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/winhttp/libwinpr-winhttp.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/winsock/libwinpr-winsock.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wnd/libwinpr-wnd.a
	#${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wtsapi/libwinpr-wtsapi.a
        pthread
        rt)


ADD_SUBDIRECTORY(asn1)
ADD_SUBDIRECTORY(bcrypt)
ADD_SUBDIRECTORY(com)
ADD_SUBDIRECTORY(credentials)
ADD_SUBDIRECTORY(credui)
ADD_SUBDIRECTORY(crt)
#ADD_SUBDIRECTORY(crypto)
ADD_SUBDIRECTORY(dsparse)
ADD_SUBDIRECTORY(environment)
ADD_SUBDIRECTORY(error)
ADD_SUBDIRECTORY(file)
ADD_SUBDIRECTORY(handle)
ADD_SUBDIRECTORY(heap)
ADD_SUBDIRECTORY(input)
ADD_SUBDIRECTORY(interlocked)
ADD_SUBDIRECTORY(io)
ADD_SUBDIRECTORY(library)
ADD_SUBDIRECTORY(locale)
ADD_SUBDIRECTORY(memory)
ADD_SUBDIRECTORY(nt)
ADD_SUBDIRECTORY(path)
ADD_SUBDIRECTORY(pipe)
ADD_SUBDIRECTORY(pool)
ADD_SUBDIRECTORY(registry)
#ADD_SUBDIRECTORY(rpc)
ADD_SUBDIRECTORY(security)
ADD_SUBDIRECTORY(smartcard)
#ADD_SUBDIRECTORY(sspi)
#ADD_SUBDIRECTORY(sspicli)
ADD_SUBDIRECTORY(synch)
ADD_SUBDIRECTORY(sysinfo)
ADD_SUBDIRECTORY(thread)
ADD_SUBDIRECTORY(timezone)
#ADD_SUBDIRECTORY(utils)
#ADD_SUBDIRECTORY(winhttp)
#ADD_SUBDIRECTORY(winsock)
#ADD_SUBDIRECTORY(wnd)
#ADD_SUBDIRECTORY(wtsapi)



