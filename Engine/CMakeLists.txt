set(CMAKE_CXX_STANDARD 20)

add_library(Engine STATIC Engine.cpp Engine.h util/ThreadQueue.h)

if (NOT WIN32)
    target_link_libraries(Engine PRIVATE -ldl -lpthread)
endif()