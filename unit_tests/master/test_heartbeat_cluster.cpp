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
 * test_heartbeat_cluster.cpp - TODO - add brief documentation
 */

#include "test_heartbeat_cluster.hpp"

#include "heartbeat_cluster.hpp"

namespace test_heartbeat
{

  int
  test_cluster ()
  {
    std::string node_list;
    node_list.append ("some_group@");

    char hostname_cstr[CUB_MAXHOSTNAMELEN];
    css_gethostname (hostname_cstr, CUB_MAXHOSTNAMELEN);

    node_list.append (hostname_cstr);

    cubhb::config *conf = new test_config (node_list);
    cubhb::transport *transport = new in_memory_transport ();
    cubhb::cluster *cluster = new cubhb::cluster (conf, transport);

    int error_code = cluster->init ();
    assert (error_code == NO_ERROR);

    cluster->stop ();
    assert (cluster->shutdown);

    delete cluster;

    return 0;
  }


  test_config::test_config (std::string &node_list)
    : m_node_list (node_list)
  {
    //
  }

  cubhb::node_state
  test_config::get_state () const
  {
    return (cubhb::node_state) prm_get_integer_value (PRM_ID_HA_STATE);
  }

  int
  test_config::get_heartbeat_interval () const
  {
    return prm_get_integer_value (PRM_ID_HA_HEARTBEAT_INTERVAL_IN_MSECS);
  }

  const char *
  test_config::get_master_host () const
  {
    return prm_get_string_value (PRM_ID_HA_MASTER_HOST);
  }

  int
  test_config::get_port () const
  {
    return prm_get_integer_value (PRM_ID_HA_PORT_ID);
  }

  const char *
  test_config::get_node_list () const
  {
    return m_node_list.c_str ();
  }

  const char *
  test_config::get_replica_list () const
  {
    return prm_get_string_value (PRM_ID_HA_REPLICA_LIST);
  }

  const char *
  test_config::get_ping_hosts () const
  {
    return prm_get_string_value (PRM_ID_HA_PING_HOSTS);
  }

  int
  in_memory_transport::start ()
  {
    return NO_ERROR;
  }

  void
  in_memory_transport::stop ()
  {
    //
  }

  int
  in_memory_transport::send_request (const cubhb::request_type &request) const
  {
    handle_request (request);
  }

} /* namespace test_heartbeat */
