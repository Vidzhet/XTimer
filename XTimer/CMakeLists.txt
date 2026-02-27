cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC_RELATIVE_PATHS ON)

if (WIN32)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT XTimer)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

set(SOURCES
    main.cpp
    window.cpp
    settings.cpp
    styles.cpp
    window.h
    settings.h
    styles.h
	gradient_label.h
)

set(SYSTEM_FILES
    config.cpp
    listener.cpp
	timer.cpp
	config.h
    listener.h
    timer.h
)

set(HEADERS
    ConfigCreator.h
)

set(ALL_FILES
    ${SOURCES}
	${SYSTEM_FILES}
	window_glow.h
    ${HEADERS}
)

if(WIN32)
    source_group("Source Files" FILES ${SOURCES})
	source_group("Source Files\\System" FILES ${SYSTEM_FILES})
	source_group("Source Files\\Unused Features" FILES window_glow.h)
    source_group("Header Files" FILES ${HEADERS})
	source_group("_CMake" REGULAR_EXPRESSION "mocs_.*\\.cpp$")
endif()

qt_add_executable(XTimer ${ALL_FILES})

target_link_libraries(XTimer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(XTimer PROPERTIES WIN32_EXECUTABLE ON)
endif()

if(APPLE)
    set_target_properties(XTimer PROPERTIES MACOSX_BUNDLE ON)
endif()