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
 * heartbeat_cluster.hpp - heartbeat cluster module
 */

#ifndef _HEARTBEAT_CLUSTER_HPP_
#define _HEARTBEAT_CLUSTER_HPP_

#include "heartbeat_config.hpp"
#include "heartbeat_service.hpp"
#include "heartbeat_transport.hpp"
#include "hostname.hpp"
#include "packable_object.hpp"
#include "porting.h"
#include "system_parameter.h"

#include <chrono>
#include <list>
#if defined (LINUX)
#include <netinet/in.h> // for sockaddr_in
#endif
#include <string>
#include <vector>

namespace cubhb
{

  static const std::chrono::milliseconds UI_NODE_CACHE_TIME_IN_MSECS (60 * 1000);
  static const std::chrono::milliseconds UI_NODE_CLEANUP_TIME_IN_MSECS (3600 * 1000);

  /* heartbeat node entries */
  class node_entry : public cubpacking::packable_object
  {
    public:
      using priority_type = unsigned short;

      static const priority_type HIGHEST_PRIORITY = std::numeric_limits<priority_type>::min () + 1;
      static const priority_type LOWEST_PRIORITY = std::numeric_limits<priority_type>::max ();
      static const priority_type REPLICA_PRIORITY = LOWEST_PRIORITY;

      node_entry () = delete;
      node_entry (cubbase::hostname_type &hostname, priority_type priority);
      ~node_entry () override = default;

      node_entry (const node_entry &other); // Copy c-tor
      node_entry &operator= (const node_entry &other); // Copy assignment

      const cubbase::hostname_type &get_hostname () const;
      bool is_time_initialized () const;

      size_t get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const override;
      void pack (cubpacking::packer &serializator) const override;
      void unpack (cubpacking::unpacker &deserializator) override;

    public: // TODO CBRD-22864 members should be private
      cubbase::hostname_type hostname;
      priority_type priority;
      node_state state;
      short score;
      short heartbeat_gap;
      std::chrono::system_clock::time_point last_recv_hbtime; // last received heartbeat time
  };

  /* heartbeat ping host entries */
  class ping_host
  {
    public:
      ping_host () = delete;
      explicit ping_host (const std::string &hostname);
      ~ping_host () = default;

      void ping ();
      bool is_ping_successful ();

      const cubbase::hostname_type &get_hostname () const;

      enum ping_result
      {
	UNKNOWN = -1,
	SUCCESS = 0,
	USELESS_HOST = 1,
	SYS_ERR = 2,
	FAILURE = 3
      };

    public: // TODO CBRD-22864 members should be private
      cubbase::hostname_type hostname;
      ping_result result;
  };

  enum ui_node_result
  {
    VALID_NODE = 0,
    UNIDENTIFIED_NODE = 1,
    GROUP_NAME_MISMATCH = 2,
    IP_ADDR_MISMATCH = 3,
    CANNOT_RESOLVE_HOST = 4
  };

  /* heartbeat unidentified host entries */
  class ui_node
  {
    public:
      ui_node (const cubbase::hostname_type &hostname, const std::string &group_id, const sockaddr_in &sockaddr,
	       ui_node_result v_result);
      ~ui_node () = default;

      void set_last_recv_time_to_now ();
      const cubbase::hostname_type &get_hostname () const;

    public: // TODO CBRD-22864 members should be private
      cubbase::hostname_type hostname;
      std::string group_id;
      sockaddr_in saddr;
      std::chrono::system_clock::time_point last_recv_time;
      ui_node_result v_result;
  };

  class cluster
  {
    public:
      explicit cluster (config *conf);

      cluster (const cluster &other); // Copy c-tor
      cluster &operator= (const cluster &other); // Copy assignment

      ~cluster ();

      int init ();
      void destroy ();
      int reload ();
      void stop ();

      const cubbase::hostname_type &get_hostname () const;
      const node_state &get_state () const;
      const std::string &get_group_id () const;
      const node_entry *get_myself_node () const;

      void on_hearbeat_request (const header &header_, const sockaddr_in *from, socklen_t from_len);
      void send_heartbeat_to_all ();
      bool is_heartbeat_received_from_all ();

      void cleanup_ui_nodes ();

      bool check_valid_ping_host ();

    private:
      void get_config_node_list (const char *prm, std::string &group, std::vector<std::string> &hostnames) const;

      int init_state ();
      int init_nodes ();
      int init_replica_nodes ();
      void init_ping_hosts ();

      node_entry *find_node (const cubbase::hostname_type &node_hostname) const;
      node_entry *find_node_except_me (const cubbase::hostname_type &node_hostname) const;

      void remove_ui_node (ui_node *&node);
      ui_node *find_ui_node (const cubbase::hostname_type &node_hostname, const std::string &node_group_id,
			     const sockaddr_in &sockaddr) const;
      ui_node *insert_ui_node (const cubbase::hostname_type &node_hostname, const std::string &node_group_id,
			       const sockaddr_in &sockaddr, ui_node_result v_result);

      node_entry *insert_host_node (const std::string &node_hostname, node_entry::priority_type priority);

      ui_node_result is_heartbeat_valid (const cubbase::hostname_type &node_hostname, const std::string &node_group_id,
					 const sockaddr_in *from) const;

    public: // TODO CBRD-22864 members should be private
      config *m_config;
      heartbeat_service m_hb_service;

      pthread_mutex_t lock; // TODO CBRD-22864 replace with std::mutex

      node_state state;
      std::string group_id;
      cubbase::hostname_type hostname;

      std::list<node_entry *> nodes;

      node_entry *myself;
      node_entry *master;

      bool shutdown;
      bool hide_to_demote;
      bool is_isolated;
      bool is_ping_check_enabled;

      std::list<ui_node *> ui_nodes;
      std::list<ping_host> ping_hosts;
  };

} // namespace cubhb

#endif /* _HEARTBEAT_CLUSTER_HPP_ */
