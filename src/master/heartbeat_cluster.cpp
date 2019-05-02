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
 * heartbeat_cluster.cpp - heartbeat cluster module
 */

#include "heartbeat_cluster.hpp"

#include "master_heartbeat.hpp"
#include "master_util.h"
#if defined (LINUX)
#include "tcp.h"
#else
#include "wintcp.h"
#endif

namespace cubhb
{
  void trim_str (std::string &str);

  void split_str (const std::string &str, std::string &delim, std::vector<std::string> &tokens);
}

namespace cubhb
{

  hostname_type::hostname_type (const char *hostname)
    :m_hostname (hostname)
  {
    //
  }

  hostname_type::hostname_type (const std::string &hostname)
    :m_hostname (hostname)
  {
    //
  }

  hostname_type &
  hostname_type::operator= (const char *hostname)
  {
    m_hostname.assign (hostname);
    return *this;
  }

  hostname_type &
  hostname_type::operator= (const std::string &hostname)
  {
    m_hostname.assign (hostname);
    return *this;
  }

  /**
   * Compare two host names if are equal, if one of the host names is canonical name and the other is not, then
   * only host part (e.g. for canonical name "host-1.cubrid.org" host part is "host-1") is used for comparison
   *
   * for example following hosts are equal:
   *  "host-1"            "host-1"
   *  "host-1"            "host-1.cubrid.org"
   *  "host-1.cubrid.org" "host-1"
   *  "host-1.cubrid.org" "host-1.cubrid.org"
   *
   * for example following hosts are not equal:
   *  "host-1"            "host-2"
   *  "host-1.cubrid.org" "host-2"
   *  "host-1"            "host-2.cubrid.org"
   *  "host-1.cubrid.org" "host-2.cubrid.org"
   *  "host-1.cubrid.org" "host-1.cubrid.com"
   *
   * @param other second hostname (first hostname is this->m_hostname)
   *
   * @return true if this->m_hostname is same as other
   */
  bool
  hostname_type::operator== (const char *other) const
  {
    const char *lhs = other;
    const char *rhs = this->m_hostname.c_str ();

    for (; *rhs && *lhs && (*rhs == *lhs); ++rhs, ++lhs)
      ;

    if (*rhs == '\0' && *lhs != '\0')
      {
	// if rhs reached the end and lhs does not, lhs must be '.'
	return *lhs == '.';
      }
    else if (*rhs != '\0' && *lhs == '\0')
      {
	// if lhs reached the end and rhs does not, rhs must be '.'
	return *rhs == '.';
      }
    else
      {
	return *rhs == *lhs;
      }
  }

  bool
  hostname_type::operator== (const std::string &other) const
  {
    return *this == other.c_str ();
  }

  bool
  hostname_type::operator== (const hostname_type &other) const
  {
    return *this == other.m_hostname.c_str ();
  }

  bool
  hostname_type::operator!= (const char *other) const
  {
    return ! (*this == other);
  }

  bool
  hostname_type::operator!= (const std::string &other) const
  {
    return ! (*this == other.c_str ());
  }

  bool
  hostname_type::operator!= (const hostname_type &other) const
  {
    return ! (*this == other.m_hostname.c_str ());
  }

  const char *
  hostname_type::as_c_str () const
  {
    return m_hostname.c_str ();
  }

  const std::string &
  hostname_type::as_str () const
  {
    return m_hostname;
  }

  size_t
  hostname_type::get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const
  {
    return serializator.get_packed_string_size (m_hostname, start_offset);
  }

  void
  hostname_type::pack (cubpacking::packer &serializator) const
  {
    serializator.pack_string (m_hostname);
  }

  void
  hostname_type::unpack (cubpacking::unpacker &deserializator)
  {
    deserializator.unpack_string (m_hostname);
  }

  node_entry::node_entry (hostname_type &hostname, priority_type priority)
    : hostname (hostname)
    , priority (priority)
    , state (node_state::UNKNOWN)
    , score (0)
    , heartbeat_gap (0)
    , last_recv_hbtime {0, 0}
  {
    //
  }

  node_entry::node_entry (const node_entry &other)
    : hostname (other.hostname)
    , priority (other.priority)
    , state (other.state)
    , score (other.score)
    , heartbeat_gap (other.heartbeat_gap)
    , last_recv_hbtime ()
  {
    last_recv_hbtime.tv_sec = other.last_recv_hbtime.tv_usec;
    last_recv_hbtime.tv_usec = other.last_recv_hbtime.tv_usec;
  }

  node_entry &
  node_entry::operator= (const node_entry &other)
  {
    hostname = other.hostname;
    priority = other.priority;
    state = other.state;
    score = other.score;
    heartbeat_gap = other.heartbeat_gap;
    last_recv_hbtime.tv_sec = other.last_recv_hbtime.tv_sec;
    last_recv_hbtime.tv_usec = other.last_recv_hbtime.tv_usec;

    return *this;
  }

  const hostname_type &
  node_entry::get_hostname () const
  {
    return hostname;
  }

  size_t
  node_entry::get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const
  {
    size_t size = hostname.get_packed_size (serializator, start_offset);
    size += serializator.get_packed_short_size (size);
    size += serializator.get_packed_int_size (size);
    return size;
  }

  void
  node_entry::pack (cubpacking::packer &serializator) const
  {
    hostname.pack (serializator);
    serializator.pack_short (priority);
    serializator.pack_int ((int) state);
  }

  void
  node_entry::unpack (cubpacking::unpacker &deserializator)
  {
    hostname.unpack (deserializator);

    short priority_;
    deserializator.unpack_short (priority_);
    priority = priority_;

    int state_;
    deserializator.unpack_int (state_);
    state = (node_state) state_;
  }

  ping_host::ping_host (const std::string &hostname)
    : hostname (hostname)
    , result (ping_result::UNKNOWN)
  {
    //
  }

  void
  ping_host::ping ()
  {
    result = hb_check_ping (hostname.as_c_str ());
  }

  bool
  ping_host::is_ping_successful ()
  {
    return result == ping_result::SUCCESS;
  }

  const hostname_type &
  ping_host::get_hostname () const
  {
    return hostname;
  }

  ui_node::ui_node (const hostname_type &hostname, const std::string &group_id, const sockaddr_in &sockaddr, int v_result)
    : hostname (hostname)
    , group_id (group_id)
    , saddr ()
    , last_recv_time (std::chrono::system_clock::now ())
    , v_result (v_result)
  {
    memcpy ((void *) &saddr, (void *) &sockaddr, sizeof (sockaddr_in));
  }

  void
  ui_node::set_last_recv_time_to_now ()
  {
    last_recv_time = std::chrono::system_clock::now ();
  }

  const hostname_type &
  ui_node::get_hostname () const
  {
    return hostname;
  }

  cluster::cluster ()
    : lock ()
    , sfd (INVALID_SOCKET)
    , state (node_state ::UNKNOWN)
    , group_id ()
    , hostname ()
    , nodes ()
    , myself (NULL)
    , master (NULL)
    , shutdown (false)
    , hide_to_demote (false)
    , is_isolated (false)
    , is_ping_check_enabled (false)
    , ui_nodes ()
    , ping_hosts ()
  {
    pthread_mutex_init (&lock, NULL);
  }

  cluster::cluster (const cluster &other)
    : lock ()
    , sfd (other.sfd)
    , state (other.state)
    , group_id (other.group_id)
    , hostname (other.hostname)
    , nodes (other.nodes)
    , myself (other.myself)
    , master (other.master)
    , shutdown (other.shutdown)
    , hide_to_demote (other.hide_to_demote)
    , is_isolated (other.is_isolated)
    , is_ping_check_enabled (other.is_ping_check_enabled)
    , ui_nodes (other.ui_nodes)
    , ping_hosts (other.ping_hosts)
  {
    memcpy (&lock, &other.lock, sizeof (pthread_mutex_t));
  }

  cluster &
  cluster::operator= (const cluster &other)
  {
    memcpy (&lock, &other.lock, sizeof (pthread_mutex_t));
    sfd = other.sfd;
    state = other.state;
    group_id = other.group_id;
    hostname = other.hostname;
    nodes = other.nodes;
    *myself = *other.myself;
    *master = *other.master;
    shutdown = other.shutdown;
    hide_to_demote = other.hide_to_demote;
    is_isolated = other.is_isolated;
    is_ping_check_enabled = other.is_ping_check_enabled;
    ui_nodes = other.ui_nodes;
    ping_hosts = other.ping_hosts;

    return *this;
  }

  cluster::~cluster ()
  {
    destroy ();
  }

  int
  cluster::init ()
  {
    char hostname_cstr[MAXHOSTNAMELEN];
    int error_code = css_gethostname (hostname_cstr, MAXHOSTNAMELEN);
    if (error_code != NO_ERROR)
      {
	MASTER_ER_SET_WITH_OSERROR (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_BO_UNABLE_TO_FIND_HOSTNAME, 0);
	return ER_BO_UNABLE_TO_FIND_HOSTNAME;
      }

    hostname = hostname_cstr;
    is_ping_check_enabled = true;

    error_code = init_state ();
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    error_code = init_nodes ();
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    if (state == node_state::REPLICA && myself != NULL)
      {
	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "myself should be in the ha_replica_list\n");
	return ER_FAILED;
      }

    error_code = init_replica_nodes ();
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    if (myself == NULL)
      {
	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "cannot find myself\n");
	return ER_FAILED;
      }
    if (nodes.empty ())
      {
	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "cluster cluster node list is empty\n");
	MASTER_ER_SET (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_PRM_BAD_VALUE, 1, prm_get_name (PRM_ID_HA_NODE_LIST));
	return ER_PRM_BAD_VALUE;
      }

    init_ping_hosts ();
    bool valid_ping_host_exists = check_valid_ping_host ();
    if (!valid_ping_host_exists)
      {
	return ER_FAILED;
      }

    return error_code;
  }

  void
  cluster::destroy ()
  {
    for (const node_entry *node : nodes)
      {
	delete node;
      }
    nodes.clear ();

    for (const ui_node *node : ui_nodes)
      {
	delete node;
      }
    ui_nodes.clear ();

    ping_hosts.clear ();

    pthread_mutex_destroy (&lock);
  }

  int
  cluster::reload ()
  {
    int error_code = sysprm_reload_and_init (NULL, NULL);
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    // save current state of the cluster
    cluster copy = *this;

    // clear existing hosts
    ping_hosts.clear ();
    nodes.clear ();

    error_code = init ();
    if (error_code != NO_ERROR)
      {
	// something went wrong, restore old state of the cluster
	*this = copy;
	return error_code;
      }
    if (master != NULL && find_node (master->get_hostname ()) == NULL)
      {
	// could not find master, restore old state of the cluster
	*this = copy;

	MASTER_ER_SET (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_PRM_BAD_VALUE, 1, prm_get_name (PRM_ID_HA_NODE_LIST));
	return ER_PRM_BAD_VALUE;
      }

    // re initialization went successfully
    for (node_entry *new_node : nodes)
      {
	node_entry *old_node = copy.find_node (new_node->get_hostname ());
	if (old_node != NULL)
	  {
	    // copy node members
	    *new_node = *old_node;
	  }

	if (copy.master && new_node->get_hostname () == copy.master->get_hostname ())
	  {
	    master = new_node;
	  }
      }

    state = copy.state;
    is_ping_check_enabled = copy.is_ping_check_enabled;

    return NO_ERROR;
  }

  int
  cluster::listen ()
  {
    sfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
      {
	MASTER_ER_SET_WITH_OSERROR (ER_ERROR_SEVERITY, ARG_FILE_LINE, ERR_CSS_TCP_DATAGRAM_SOCKET, 0);
	return ERR_CSS_TCP_DATAGRAM_SOCKET;
      }

    sockaddr_in udp_sockaddr;
    memset ((void *) &udp_sockaddr, 0, sizeof (udp_sockaddr));
    udp_sockaddr.sin_family = AF_INET;
    udp_sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    udp_sockaddr.sin_port = htons (prm_get_integer_value (PRM_ID_HA_PORT_ID));

    if (bind (sfd, (sockaddr *) &udp_sockaddr, sizeof (udp_sockaddr)) < 0)
      {
	MASTER_ER_SET_WITH_OSERROR (ER_ERROR_SEVERITY, ARG_FILE_LINE, ERR_CSS_TCP_DATAGRAM_BIND, 0);
	return ERR_CSS_TCP_DATAGRAM_BIND;
      }

    return NO_ERROR;
  }

  void
  cluster::stop ()
  {
    master = NULL;
    myself = NULL;
    shutdown = true;
    state = node_state::UNKNOWN;

    destroy ();

    close (sfd);
    sfd = INVALID_SOCKET;
  }

  const hostname_type &
  cluster::get_hostname () const
  {
    return hostname;
  }

  const node_state &
  cluster::get_state () const
  {
    return state;
  }

  const std::string &
  cluster::get_group_id () const
  {
    return group_id;
  }

  node_entry *
  cluster::find_node (const hostname_type &node_hostname) const
  {
    for (node_entry *node : nodes)
      {
	if (node_hostname == node->get_hostname ())
	  {
	    return node;
	  }
      }

    return NULL;
  }

  void
  cluster::remove_ui_node (ui_node *&node)
  {
    if (node == NULL)
      {
	return;
      }

    ui_nodes.remove (node);
    delete node;
    node = NULL;
  }

  void
  cluster::cleanup_ui_nodes ()
  {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now ();
    for (ui_node *node : ui_nodes)
      {
	if ((now - node->last_recv_time) > UI_NODE_CLEANUP_TIME_IN_MSECS)
	  {
	    remove_ui_node (node);
	  }
      }
  }

  ui_node *
  cluster::find_ui_node (const hostname_type &node_hostname, const std::string &node_group_id,
			 const sockaddr_in &sockaddr) const
  {
    for (ui_node *node : ui_nodes)
      {
	if (node->get_hostname () == node_hostname && node->group_id == node_group_id
	    && node->saddr.sin_addr.s_addr == sockaddr.sin_addr.s_addr)
	  {
	    return node;
	  }
      }

    return NULL;
  }

  ui_node *
  cluster::insert_ui_node (const hostname_type &node_hostname, const std::string &node_group_id,
			   const sockaddr_in &sockaddr, const int v_result)
  {
    assert (v_result == HB_VALID_UNIDENTIFIED_NODE || v_result == HB_VALID_GROUP_NAME_MISMATCH
	    || v_result == HB_VALID_IP_ADDR_MISMATCH || v_result == HB_VALID_CANNOT_RESOLVE_HOST);

    ui_node *node = find_ui_node (node_hostname, node_group_id, sockaddr);
    if (node)
      {
	return node;
      }

    node = new ui_node (node_hostname, node_group_id, sockaddr, v_result);
    ui_nodes.push_front (node);

    return node;
  }

  /*
   * check_valid_ping_host
   *   return: whether a valid ping host exists or not
   *
   * NOTE: it returns true when no ping host is specified.
   */
  bool
  cluster::check_valid_ping_host ()
  {
    if (ping_hosts.empty ())
      {
	return true;
      }

    bool valid_ping_host_exists = false;
    for (ping_host &host : ping_hosts)
      {
	host.ping ();
	if (host.is_ping_successful ())
	  {
	    valid_ping_host_exists = true;
	  }
      }

    return valid_ping_host_exists;
  }

  void
  cluster::get_config_node_list (PARAM_ID prm_id, std::string &group, std::vector<std::string> &hostnames) const
  {
    group.clear ();
    hostnames.clear ();

    char *prm_string_value = prm_get_string_value (prm_id);
    if (prm_string_value == NULL)
      {
	return;
      }

    std::string delimiter = "@:,";
    std::vector<std::string> tokens;
    split_str (prm_string_value, delimiter, tokens);

    if (tokens.size () < 2)
      {
	return;
      }

    // first entry is group id
    group.assign (tokens.front ());

    // starting from second element, all elements are hostnames
    hostnames.resize (tokens.size () - 1);
    std::copy (tokens.begin () + 1, tokens.end (), hostnames.begin ());
  }

  int
  cluster::init_state ()
  {
    /*    node_state ha_state = (node_state) prm_get_integer_value (PRM_ID_HA_STATE);
        if (ha_state != node_state::UNKNOWN)
          {
    	state = ha_state;
    	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "state %s was configured\n", hb_node_state_string (ha_state));
          }

        if (state != node_state::MASTER)
          {
    	char *master_host = prm_get_string_value (PRM_ID_HA_MASTER_HOST);
    	if (master_host == NULL)
    	  {
    	    MASTER_ER_SET (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_PRM_BAD_VALUE, 1, prm_get_name (PRM_ID_HA_MASTER_HOST));
    	    return ER_PRM_BAD_VALUE;
    	  }

    	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "master host: %s\n", master_host);

    	node_entry *master_node = insert_host_node (master_host, node_entry::HIGHEST_PRIORITY);
    	master_node->state = node_state::MASTER;
    	master = master_node;

    	node_entry *node = NULL;
    	if (state == node_state::REPLICA)
    	  {
    	    node = insert_host_node ("localhost", node_entry::REPLICA_PRIORITY);
    	    node->state = node_state::REPLICA;
    	  }
    	else
    	  {
    	    node = insert_host_node ("localhost", node_entry::HIGHEST_PRIORITY + 1);
    	    node->state = node_state::SLAVE;
    	  }

    	myself = node;

    	return NO_ERROR;
          }
        if (state == node_state::MASTER)
          {
    	node_entry *node = insert_host_node ("localhost", node_entry::HIGHEST_PRIORITY);
    	node->state = node_state::MASTER;

    	myself = node;
    	master = node;
          }*/

    if (HA_GET_MODE () == HA_MODE_REPLICA)
      {
	state = node_state::REPLICA;
      }
    else
      {
	state = node_state::SLAVE;
      }

    return NO_ERROR;
  }

  int
  cluster::init_nodes ()
  {
    std::vector<std::string> hostnames;

    get_config_node_list (PRM_ID_HA_NODE_LIST, group_id, hostnames);
    if (hostnames.empty () || group_id.empty ())
      {
	// TODO [new slave]
	//MASTER_ER_SET (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_PRM_BAD_VALUE, 1, prm_get_name (PRM_ID_HA_NODE_LIST));
	return NO_ERROR;
      }

    node_entry::priority_type priority = node_entry::HIGHEST_PRIORITY;
    for (const std::string &node_hostname : hostnames)
      {
	node_entry *node = insert_host_node (node_hostname, priority);
	if (node->get_hostname () == hostname)
	  {
	    myself = node;
#if defined (HB_VERBOSE_DEBUG)
	    MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "find myself node (myself:%p, priority:%d)\n", myself, myself->priority);
#endif
	  }

	++priority;
      }

    return NO_ERROR;
  }

  int
  cluster::init_replica_nodes ()
  {
    std::string replica_group_id;
    std::vector<std::string> hostnames;

    get_config_node_list (PRM_ID_HA_REPLICA_LIST, replica_group_id, hostnames);
    if (hostnames.empty ())
      {
	return NO_ERROR;
      }
    if (replica_group_id != group_id)
      {
	MASTER_ER_LOG_DEBUG (ARG_FILE_LINE, "different group id ('ha_node_list', 'ha_replica_list')\n");
	return ER_FAILED;
      }

    for (const std::string &replica_hostname : hostnames)
      {
	node_entry *replica_node = insert_host_node (replica_hostname, node_entry::REPLICA_PRIORITY);
	if (replica_node->get_hostname () == hostname)
	  {
	    myself = replica_node;
	    state = node_state::REPLICA;
	  }
      }

    return NO_ERROR;
  }

  void
  cluster::init_ping_hosts ()
  {
    char *ha_ping_hosts = prm_get_string_value (PRM_ID_HA_PING_HOSTS);
    if (ha_ping_hosts == NULL)
      {
	return;
      }

    std::string delimiter = ":,";
    std::vector<std::string> tokens;
    split_str (ha_ping_hosts, delimiter, tokens);

    for (const std::string &token : tokens)
      {
	ping_hosts.emplace_front (token);
      }
  }

  node_entry *
  cluster::insert_host_node (const std::string &node_hostname, const node_entry::priority_type priority)
  {
    node_entry *node = NULL;
    hostname_type node_hostname_ (node_hostname);

    if (node_hostname_ == "localhost")
      {
	node = new node_entry (hostname, priority);
      }
    else
      {
	node = new node_entry (node_hostname_, priority);
      }

    nodes.push_front (node);

    return node;
  }

  inline void
  trim_str (std::string &str)
  {
    std::string ws (" \t\f\v\n\r");

    str.erase (0, str.find_first_not_of (ws));
    str.erase (str.find_last_not_of (ws) + 1);
  }

  /*
   * split string to tokens by a specified delimiter:
   *
   * @param str: input string
   * @param delimiter: delimiter to use for split
   * @param tokens: output vector of tokens
   */
  void
  split_str (const std::string &str, std::string &delimiter, std::vector<std::string> &tokens)
  {
    if (str.empty ())
      {
	return;
      }

    std::string::size_type start = str.find_first_not_of (delimiter);
    std::string::size_type pos = str.find_first_of (delimiter, start);

    while (pos != std::string::npos)
      {
	std::string token = str.substr (start, pos - start);
	trim_str (token);
	tokens.push_back (token);

	start = str.find_first_not_of (delimiter, pos);
	pos = str.find_first_of (delimiter, start);
      }

    std::string::size_type str_length = str.length ();
    if (str_length > start) // rest
      {
	std::string token = str.substr (start, str_length - start);
	trim_str (token);
	tokens.push_back (token);
      }
  }


  header::header ()
    : m_message_type (message_type::MSG_UNKNOWN)
    , m_is_request (false)
    , m_state (node_state::UNKNOWN)
    , m_group_id ()
    , m_orig_hostname ()
    , m_dest_hostname ()
  {
    //
  }

  header::header (message_type type, bool is_request, const hostname_type &dest_hostname, const cluster &c)
    : m_message_type (type)
    , m_is_request (is_request)
    , m_state (c.get_state ())
    , m_group_id (c.get_group_id ())
    , m_orig_hostname (c.get_hostname ())
    , m_dest_hostname (dest_hostname)
  {
    //
  }

  const message_type &
  header::get_message_type () const
  {
    return m_message_type;
  }

  const bool &
  header::is_request () const
  {
    return m_is_request;
  }

  const node_state &
  header::get_state () const
  {
    return m_state;
  }

  const std::string &
  header::get_group_id () const
  {
    return m_group_id;
  }

  const hostname_type &
  header::get_orig_hostname () const
  {
    return m_orig_hostname;
  }

  const hostname_type &
  header::get_dest_hostname () const
  {
    return m_dest_hostname;
  }

  size_t
  header::get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const
  {
    size_t size = serializator.get_packed_int_size (start_offset); // type
    size += serializator.get_packed_bool_size (size); // is_request
    size += serializator.get_packed_int_size (size); // m_state
    size += serializator.get_packed_string_size (m_group_id, size);
    size += m_orig_hostname.get_packed_size (serializator, size);
    size += m_dest_hostname.get_packed_size (serializator, size);

    return size;
  }

  void
  header::pack (cubpacking::packer &serializator) const
  {
    serializator.pack_int ((int) m_message_type);
    serializator.pack_bool (m_is_request);
    serializator.pack_int ((int) m_state);
    serializator.pack_string (m_group_id);
    m_orig_hostname.pack (serializator);
    m_dest_hostname.pack (serializator);
  }

  void
  header::unpack (cubpacking::unpacker &deserializator)
  {
    int type;
    deserializator.unpack_int (type);
    m_message_type = (message_type) type;

    deserializator.unpack_bool (m_is_request);

    int state;
    deserializator.unpack_int (state);
    m_state = (node_state) state;

    deserializator.unpack_string (m_group_id);
    m_orig_hostname.unpack (deserializator);
    m_dest_hostname.unpack (deserializator);
  }
} // namespace cubhb
