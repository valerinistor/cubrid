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

set(LOCALE_DIRS 
  en_US
  en_US.utf8
  de_DE.utf8
  es_ES.utf8
  fr_FR.utf8
  it_IT.utf8
  ja_JP.utf8
  km_KH.utf8
  ko_KR.euckr
  ko_KR.utf8
  ro_RO.utf8
  tr_TR.utf8
  vi_VN.utf8
  zh_CN.utf8
  )

foreach(locale ${LOCALE_DIRS})
  add_custom_command(
    OUTPUT ${locale}/cubrid.cat
    COMMAND ${CMAKE_COMMAND} -E make_directory ${locale}
    COMMAND gencat ${locale}/cubrid.cat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/cubrid.msg
    COMMENT "Generating cubrid.cat for ${locale}"
    DEPENDS gencat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/cubrid.msg
    )
  add_custom_command(
    OUTPUT ${locale}/csql.cat
    COMMAND ${CMAKE_COMMAND} -E make_directory ${locale}
    COMMAND gencat ${locale}/csql.cat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/csql.msg
    COMMENT "Generating csql.cat for ${locale}"
    DEPENDS gencat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/csql.msg
    )
  add_custom_command(
    OUTPUT ${locale}/utils.cat
    COMMAND ${CMAKE_COMMAND} -E make_directory ${locale}
    COMMAND gencat ${locale}/utils.cat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/utils.msg
    COMMENT "Generating utils.cat for ${locale}"
    DEPENDS gencat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/utils.msg
    )
  add_custom_command(
    OUTPUT ${locale}/esql.cat
    COMMAND ${CMAKE_COMMAND} -E make_directory ${locale}
    COMMAND gencat ${locale}/esql.cat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/esql.msg
    COMMENT "Generating esql.cat for ${locale}"
    DEPENDS gencat ${CMAKE_CURRENT_SOURCE_DIR}/${locale}/esql.msg
    )
  add_custom_target(gen_msgs_${locale} ALL DEPENDS ${locale}/cubrid.cat ${locale}/csql.cat ${locale}/utils.cat ${locale}/esql.cat)

  install(DIRECTORY
    ${locale}
    DESTINATION ${CUBRID_LOCALEDIR}
    FILES_MATCHING PATTERN "*.msg")
  install(DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/${locale}
    DESTINATION ${CUBRID_LOCALEDIR}
    FILES_MATCHING PATTERN PATTERN "*.cat")
endforeach()

