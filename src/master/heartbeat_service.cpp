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
 * heartbeat_service.cpp - TODO - add brief documentation
 */

#include "heartbeat_service.hpp"

#include "heartbeat_cluster.hpp"

namespace cubhb
{

  header::header ()
    : m_is_request (false)
    , m_state (node_state::UNKNOWN)
    , m_group_id ()
    , m_orig_hostname ()
    , m_dest_hostname ()
  {
    //
  }

  header::header (bool is_request, const cubbase::hostname_type &dest_hostname, const cluster &cluster_)
    : m_is_request (is_request)
    , m_state (cluster_.get_state ())
    , m_group_id (cluster_.get_group_id ())
    , m_orig_hostname ()
    , m_dest_hostname (dest_hostname)
  {
    if (cluster_.get_myself_node () != NULL)
      {
	m_orig_hostname = cluster_.get_myself_node ()->get_hostname ();
      }
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

  const cubbase::hostname_type &
  header::get_orig_hostname () const
  {
    return m_orig_hostname;
  }

  const cubbase::hostname_type &
  header::get_dest_hostname () const
  {
    return m_dest_hostname;
  }

  size_t
  header::get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const
  {
    size_t size = serializator.get_packed_bool_size (start_offset); // m_is_request
    size += serializator.get_packed_int_size (size); // m_state
    size += serializator.get_packed_string_size (m_group_id, size);
    size += m_orig_hostname.get_packed_size (serializator, size);
    size += m_dest_hostname.get_packed_size (serializator, size);

    return size;
  }

  void
  header::pack (cubpacking::packer &serializator) const
  {
    serializator.pack_bool (m_is_request);
    serializator.pack_to_int (m_state);
    serializator.pack_string (m_group_id);
    m_orig_hostname.pack (serializator);
    m_dest_hostname.pack (serializator);
  }

  void
  header::unpack (cubpacking::unpacker &deserializator)
  {
    deserializator.unpack_bool (m_is_request);
    deserializator.unpack_from_int (m_state);
    deserializator.unpack_string (m_group_id);
    m_orig_hostname.unpack (deserializator);
    m_dest_hostname.unpack (deserializator);
  }

  heartbeat_service::heartbeat_service (cluster &cluster_)
    : m_cluster (cluster_)
  {
    handler_registry::handler_type handler = std::bind (&heartbeat_service::on_hearbeat_request, std::ref (*this),
	std::placeholders::_1, std::placeholders::_2);

    get_handler_registry ().register_handler (HEARTBEAT, handler);
  }

  int
  heartbeat_service::send_heartbeat_request (const cubbase::hostname_type &dest_hostname) const
  {
    header header_ (true, dest_hostname, m_cluster);
    cubmem::extensible_block buffer;
    cubpacking::packer packer;

    packer.set_buffer_and_pack_all (buffer, header_);

    const request_type request (buffer.get_read_ptr (), buffer.get_size ());

    const udp_server &server = get_server ();
    int error_code = server.remote_call (dest_hostname, request);
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    return NO_ERROR;
  }

  void
  heartbeat_service::on_hearbeat_request (const request_type &request, response_type &response)
  {
    header header_req;
    request.get_body (header_req);

    m_cluster.on_hearbeat_request (header_req, (const sockaddr_in *) request.get_saddr (), request.get_saddr_len ());

    // must send heartbeat response in order to avoid split-brain
    // when heartbeat configuration changed
    if (header_req.is_request () && !m_cluster.hide_to_demote)
      {
	header header_resp (false, header_req.get_orig_hostname (), m_cluster);
	response.set_body (HEARTBEAT, header_resp);
      }
  }

} /* namespace cubhb */
