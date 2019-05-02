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

  enum node_state
  {
    UNKNOWN = 0,
    SLAVE = 1,
    TO_BE_MASTER = 2,
    TO_BE_SLAVE = 3,
    MASTER = 4,
    REPLICA = 5,
    MAX
  };

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

      size_t get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const override;
      void pack (cubpacking::packer &serializator) const override;
      void unpack (cubpacking::unpacker &deserializator) override;

    public: // TODO CBRD-22864 members should be private
      cubbase::hostname_type hostname;
      priority_type priority;
      node_state state;
      short score;
      short heartbeat_gap;
      timeval last_recv_hbtime; // last received heartbeat time
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

  /* heartbeat unidentified host entries */
  class ui_node
  {
    public:
      explicit ui_node (const cubbase::hostname_type &hostname, const std::string &group_id, const sockaddr_in &sockaddr,
			int v_result);
      ~ui_node () = default;

      void set_last_recv_time_to_now ();
      const cubbase::hostname_type &get_hostname () const;

    public: // TODO CBRD-22864 members should be private
      cubbase::hostname_type hostname;
      std::string group_id;
      sockaddr_in saddr;
      std::chrono::system_clock::time_point last_recv_time;
      int v_result;
  };

  class cluster
  {
    public:
      cluster ();

      cluster (const cluster &other); // Copy c-tor
      cluster &operator= (const cluster &other); // Copy assignment

      ~cluster ();

      int init ();
      void destroy ();
      int reload ();

      int listen ();
      void stop ();

      const cubbase::hostname_type &get_hostname () const;
      const node_state &get_state () const;
      const std::string &get_group_id () const;
      const node_entry *get_myself_node () const;

      node_entry *find_node (const cubbase::hostname_type &node_hostname) const;

      void remove_ui_node (ui_node *&node);
      void cleanup_ui_nodes ();
      ui_node *find_ui_node (const cubbase::hostname_type &node_hostname, const std::string &node_group_id,
			     const sockaddr_in &sockaddr) const;
      ui_node *insert_ui_node (const cubbase::hostname_type &node_hostname, const std::string &node_group_id,
			       const sockaddr_in &sockaddr, int v_result);

      bool check_valid_ping_host ();

    private:
      void get_config_node_list (PARAM_ID prm_id, std::string &group, std::vector<std::string> &hostnames) const;

      int init_state ();
      int init_nodes ();
      int init_replica_nodes ();
      void init_ping_hosts ();

      node_entry *insert_host_node (const std::string &node_hostname, node_entry::priority_type priority);

    public: // TODO CBRD-22864 members should be private
      pthread_mutex_t lock; // TODO CBRD-22864 replace with std::mutex

      SOCKET sfd;

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

  enum message_type
  {
    MSG_UNKNOWN = 0,
    HEARTBEAT = 1,
  };

  class header : public cubpacking::packable_object
  {
    public:
      header ();
      header (message_type type, bool is_request, const cubbase::hostname_type &dest_hostname, const cluster &c);

      const message_type &get_message_type () const;
      const bool &is_request () const;
      const node_state &get_state () const;
      const std::string &get_group_id () const;
      const cubbase::hostname_type &get_orig_hostname () const;
      const cubbase::hostname_type &get_dest_hostname () const;

      size_t get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const override;
      void pack (cubpacking::packer &serializator) const override;
      void unpack (cubpacking::unpacker &deserializator) override;

    private:
      message_type m_message_type;
      bool m_is_request;

      node_state m_state;
      std::string m_group_id;
      cubbase::hostname_type m_orig_hostname;
      cubbase::hostname_type m_dest_hostname;
  };
} // namespace cubhb

#endif /* _HEARTBEAT_CLUSTER_HPP_ */
