set(API_SOURCES
    xtimer_api.h
    xtimer_api.cpp
)

if(WIN32)
    list(APPEND API_SOURCES xtimer_api_win.cpp)
elseif(UNIX)
    list(APPEND API_SOURCES xtimer_api_unix.cpp)
endif()

add_library(XTimerAPI STATIC ${API_SOURCES})

target_include_directories(XTimerAPI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)