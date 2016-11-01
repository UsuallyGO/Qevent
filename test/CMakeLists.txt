IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32) 
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)

IF(COMMAND cmake_policy)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF()

PROJECT(Qtest)

SET(QTEST_INCLUDE ${CMAKE_INSTALL_PREFIX}/include)
INCLUDE_DIRECTORIES(${QTEST_INCLUDE})

IF(WIN32)
    IF(MSVC)
	ELSE(MSVC)
	    FIND_LIBRARY(libqevent "qevent.dll")
	ENDIF(MSVC)
ELSE(WIN32)
    FIND_LIBRARY(libqevent "qevent")
ENDIF(WIN32)

SET(Qtimetest time-test.c)

ADD_EXECUTABLE(timetest ${Qtimetest})
IF(NOT libqevent MATCHES libqevent-NOTFOUND)
    TARGET_LINK_LIBRARIES(timetest ${libqevent})
ENDIF()

SET(QIOserver io-test-server.c)

ADD_EXECUTABLE(ioserver ${QIOserver})
IF(NOT libqevent MATCHES libqevent-NOTFOUND)
    TARGET_LINK_LIBRARIES(ioserver ${libqevent})
ENDIF()

SET(QIOclient io-test-client.c)

ADD_EXECUTABLE(ioclient ${QIOclient})
IF(NOT libqevent MATCHES libqevent-NOTFOUND)
    TARGET_LINK_LIBRARIES(ioclient ${libqevent})
ENDIF()

SET(Qsigtest signal-test.c)

ADD_EXECUTABLE(sigtest ${Qsigtest})
IF(NOT libqevent MATCHES libqevent-NOTFOUND)
    TARGET_LINK_LIBRARIES(sigtest ${libqevent})
ENDIF()

INSTALL(TARGETS timetest DESTINATION bin)
INSTALL(TARGETS ioserver DESTINATION bin)
INSTALL(TARGETS ioclient DESTINATION bin)
INSTALL(TARGETS sigtest  DESTINATION bin)

