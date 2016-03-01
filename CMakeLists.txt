cmake_minimum_required (VERSION 2.8.11)                                          
                                                                                                                                                               
set(Boost_USE_STATIC_LIBS OFF)                                                  
set(Boost_USE_MULTITHREADED ON)                                                 
set(Boost_USE_STATIC_RUNTIME OFF)                                               
find_package(Boost 1.54.0 COMPONENTS system filesystem)                         

set(SOURCES async_http_server/HttpSession.cpp async_http_server/Source.cpp)     
set(HEADER_FILES async_http_server/HttpSession.h async_http_server/Server.h)    

if(Boost_FOUND)                                                                 
        add_definitions(-std=c++11)                                             
        include_directories(${Boost_INCLUDE_DIRS})                              
        include_directories(async_http_server)                                  
        add_executable (final ${SOURCES} ${HEADER_FILES})                       
        target_link_libraries(final ${Boost_LIBRARIES})                         
endif()     