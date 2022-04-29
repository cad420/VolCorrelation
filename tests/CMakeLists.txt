# find_package(GTest MODULE REQUIRED)

add_executable(tests)
target_sources(tests
  PRIVATE
    main.cpp
)
target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(tests PRIVATE GTest::GTest GTest::Main)

# set by user
# set(CMAKE_PREFIX_PATH C:\\Qt\\6.1.0\\msvc2019_64\\lib\\cmake)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(gui)
target_sources(gui
        PRIVATE
        gui.cpp
        )
target_include_directories(gui PRIVATE ${PROJECT_SOURCE_DIR}/include)
 target_link_libraries(gui PRIVATE
         Qt6::Core
         Qt6::Gui
         Qt6::Widgets
         Qt6::OpenGL
         Qt6::OpenGLWidgets
         )
