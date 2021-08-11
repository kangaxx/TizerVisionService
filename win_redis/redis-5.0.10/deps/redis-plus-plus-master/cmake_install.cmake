# Install script for directory: D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/redis++")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/lib/Debug/redis++_static.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/lib/Release/redis++_static.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files/redis++/include/sw/redis++/command.h;C:/Program Files/redis++/include/sw/redis++/command_args.h;C:/Program Files/redis++/include/sw/redis++/command_options.h;C:/Program Files/redis++/include/sw/redis++/connection.h;C:/Program Files/redis++/include/sw/redis++/connection_pool.h;C:/Program Files/redis++/include/sw/redis++/errors.h;C:/Program Files/redis++/include/sw/redis++/tls.h;C:/Program Files/redis++/include/sw/redis++/pipeline.h;C:/Program Files/redis++/include/sw/redis++/queued_redis.h;C:/Program Files/redis++/include/sw/redis++/queued_redis.hpp;C:/Program Files/redis++/include/sw/redis++/redis++.h;C:/Program Files/redis++/include/sw/redis++/redis.h;C:/Program Files/redis++/include/sw/redis++/redis.hpp;C:/Program Files/redis++/include/sw/redis++/redis_cluster.h;C:/Program Files/redis++/include/sw/redis++/redis_cluster.hpp;C:/Program Files/redis++/include/sw/redis++/reply.h;C:/Program Files/redis++/include/sw/redis++/sentinel.h;C:/Program Files/redis++/include/sw/redis++/shards.h;C:/Program Files/redis++/include/sw/redis++/shards_pool.h;C:/Program Files/redis++/include/sw/redis++/subscriber.h;C:/Program Files/redis++/include/sw/redis++/transaction.h;C:/Program Files/redis++/include/sw/redis++/utils.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Program Files/redis++/include/sw/redis++" TYPE FILE FILES
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/command.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/command_args.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/command_options.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/connection.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/connection_pool.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/errors.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/no_tls/tls.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/pipeline.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/queued_redis.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/queued_redis.hpp"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/redis++.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/redis.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/redis.hpp"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/redis_cluster.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/redis_cluster.hpp"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/reply.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/sentinel.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/shards.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/shards_pool.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/subscriber.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/transaction.h"
    "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/src/sw/redis++/utils.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/codes/win_redis/redis-5.0.10/deps/redis-plus-plus-master/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
