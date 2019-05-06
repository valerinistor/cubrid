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
 * test_heartbeat_cluster.hpp - TODO - add brief documentation
 */

#ifndef _TEST_HEARTBEAT_CLUSTER_HPP_
#define _TEST_HEARTBEAT_CLUSTER_HPP_

#include "heartbeat_config.hpp"
#include "heartbeat_transport.hpp"

namespace test_heartbeat
{

  int test_cluster ();

  class test_config : public cubhb::config
  {
    public:
      test_config (std::string &node_list);
      ~test_config () override = default;

      int get_port () const override;
      cubhb::node_state get_state () const override;
      int get_heartbeat_interval () const override;
      const char *get_master_host () const override;
      const char *get_node_list () const override;
      const char *get_replica_list () const override;
      const char *get_ping_hosts () const override;

    private:
      std::string m_node_list;
  };

  class in_memory_transport : public cubhb::transport
  {
    public:
      in_memory_transport () = default;
      ~in_memory_transport () override = default;

      in_memory_transport (const in_memory_transport &other) = delete; // Copy c-tor
      in_memory_transport &operator= (const in_memory_transport &other) = delete; // Copy assignment

      in_memory_transport (in_memory_transport &&other) = delete; // Move c-tor
      in_memory_transport &operator= (in_memory_transport &&other) = delete; // Move assignment

      int start () override;
      void stop () override;

      int send_request (const cubhb::request_type &request) const override;
  };

}
#endif /* _TEST_HEARTBEAT_CLUSTER_HPP_ */
