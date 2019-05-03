/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/*
 * heartbeat_config.cpp - TODO - add brief documentation
 */

#include "heartbeat_config.hpp"

#include "system_parameter.h"

namespace cubhb
{

  int
  config_file::get_state () const
  {
    return prm_get_integer_value (PRM_ID_HA_STATE);
  }

  const char *
  config_file::get_master_host () const
  {
    return prm_get_string_value (PRM_ID_HA_MASTER_HOST);
  }

  int
  config_file::get_port () const
  {
    return prm_get_integer_value (PRM_ID_HA_PORT_ID);
  }

  const char *
  config_file::get_node_list () const
  {
    return prm_get_string_value (PRM_ID_HA_NODE_LIST);
  }

  const char *
  config_file::get_replica_list () const
  {
    return prm_get_string_value (PRM_ID_HA_REPLICA_LIST);
  }

  const char *
  config_file::get_ping_hosts () const
  {
    return prm_get_string_value (PRM_ID_HA_PING_HOSTS);
  }

} /* namespace cubhb */
