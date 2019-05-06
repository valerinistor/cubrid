#
# Copyright (C) 2016 Search Solution Corporation. All rights reserved.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

set(TEST_HEARTBEAT_SOURCES
  ${MASTER_DIR}/heartbeat_config.cpp
  ${MASTER_DIR}/heartbeat_service.cpp
  ${MASTER_DIR}/heartbeat_transport.cpp
  ${MASTER_DIR}/master.c
  ${MASTER_DIR}/master_request.c
  ${MASTER_DIR}/master_util.c
  ${MASTER_DIR}/master_heartbeat.cpp
  ${MASTER_DIR}/heartbeat_cluster.cpp
  test_main.cpp
  test_heartbeat_cluster.cpp
  )
set(TEST_HEARTBEAT_HEADERS
  ${MASTER_DIR}/heartbeat_config.hpp
  ${MASTER_DIR}/heartbeat_service.hpp
  ${MASTER_DIR}/heartbeat_transport.hpp
  ${MASTER_DIR}/master_request.h
  ${MASTER_DIR}/master_util.h
  ${MASTER_DIR}/master_heartbeat.hpp
  ${MASTER_DIR}/heartbeat_cluster.hpp
  test_heartbeat_cluster.hpp
  )
SET_SOURCE_FILES_PROPERTIES(
  ${TEST_HEARTBEAT_SOURCES}
  PROPERTIES LANGUAGE CXX
  )

add_executable(test_heartbeat
  ${TEST_HEARTBEAT_SOURCES}
  ${TEST_HEARTBEAT_HEADERS}
  )
  
target_compile_definitions(test_heartbeat PRIVATE
  ${COMMON_DEFS}
  )

target_include_directories(test_heartbeat PRIVATE
  ${TEST_INCLUDES}
  )

target_link_libraries(test_heartbeat LINK_PRIVATE
  test_common
  )
if(UNIX)
  target_link_libraries(test_heartbeat LINK_PRIVATE
    cubridcs
    )
else(UNIX)
  message( SEND_ERROR "heartbeat unit testing is for unix")
endif(UNIX)
