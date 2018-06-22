SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(spooler main.cpp)
TARGET_LINK_LIBRARIES(spooler curl opentracing zipkin zipkin_opentracing)
