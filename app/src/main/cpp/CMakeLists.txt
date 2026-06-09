cmake_minimum_required(VERSION 3.22.1)
project("StanceBeamCC")

add_library(analytics SHARED analytics.cpp)

# Force 16 KB LOAD-segment alignment at link time
target_link_options(analytics PRIVATE "-Wl,-z,max-page-size=16384")  # required

target_link_libraries(analytics android log)
