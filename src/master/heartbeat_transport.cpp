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
 * heartbeat_transport.cpp - TODO - add brief documentation
 */

#include "heartbeat_transport.hpp"

#include "error_context.hpp"
#include "memory_alloc.h"

#include <poll.h>
#include <netinet/in.h>

namespace cubhb
{

  udp_server::udp_server (int port)
    : m_thread ()
    , m_shutdown (true)
    , m_port (port)
    , m_sfd (INVALID_SOCKET)
  {
    //
  }

  udp_server::~udp_server ()
  {
    stop ();
  }

  int
  udp_server::start ()
  {
    int error_code = listen ();
    if (error_code != NO_ERROR)
      {
	return error_code;
      }

    m_shutdown = false;
    m_thread = std::thread (udp_server::poll_internal, this);

    return NO_ERROR;
  }

  void
  udp_server::stop ()
  {
    close (m_sfd);
    m_sfd = INVALID_SOCKET;

    m_shutdown = true;
    m_thread.join ();
  }

  void
  udp_server::on_request (const request_type &request) const
  {
    get_handler_registry ().handle (request.get_message_type (), request);
  }

  message_type
  request_type::get_message_type () const
  {
    cubpacking::unpacker unpacker (m_buffer, m_buffer_size);

    message_type m_type;
    unpacker.unpack_from_int (m_type);

    return m_type;
  }

  int
  request_type::reply (const response_type &response) const
  {
    if (m_sfd == INVALID_SOCKET)
      {
	return ERR_CSS_TCP_DATAGRAM_SOCKET;
      }

    ssize_t len = sendto (m_sfd, (void *) response.get_buffer (), response.get_buffer_size (), 0, m_saddr, m_saddr_len);
    if (len <= 0)
      {
	return ER_FAILED;
      }

    return NO_ERROR;
  }

  int
  udp_server::listen ()
  {
    m_sfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (m_sfd < 0)
      {
	return ERR_CSS_TCP_DATAGRAM_SOCKET;
      }

    sockaddr_in udp_sockaddr;
    memset ((void *) &udp_sockaddr, 0, sizeof (udp_sockaddr));
    udp_sockaddr.sin_family = AF_INET;
    udp_sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    udp_sockaddr.sin_port = htons (m_port);

    if (bind (m_sfd, (sockaddr *) &udp_sockaddr, sizeof (udp_sockaddr)) < 0)
      {
	return ERR_CSS_TCP_DATAGRAM_BIND;
      }

    return NO_ERROR;
  }

  void
  udp_server::poll_internal (udp_server *arg)
  {
    int error_code;
    sockaddr_in from;
    socklen_t from_len;
    std::size_t buffer_size;
    pollfd po[1] = {{0, 0, 0}};
    char *aligned_buffer = NULL;
    char buffer[BUFFER_SIZE + MAX_ALIGNMENT];

    SOCKET sfd = arg->m_sfd;
    cuberr::context er_context (true);
    memset (&from, 0, sizeof (sockaddr_in));
    aligned_buffer = PTR_ALIGN (buffer, MAX_ALIGNMENT);

    while (!arg->m_shutdown)
      {
	po[0].fd = sfd;
	po[0].events = POLLIN;

	error_code = poll (po, 1, 1);
	if (error_code <= 0)
	  {
	    continue;
	  }

	if ((po[0].revents & POLLIN) && sfd == arg->m_sfd)
	  {
	    from_len = sizeof (from);
	    buffer_size = recvfrom (sfd, (void *) aligned_buffer, BUFFER_SIZE, 0, (sockaddr *) &from, &from_len);
	    if (buffer_size > 0)
	      {
		request_type request (arg->m_sfd, aligned_buffer, buffer_size, (sockaddr *) &from, from_len);
		arg->on_request (request);
	      }
	  }
      }
  }

  static handler_registry Registry;

  handler_registry &
  get_handler_registry ()
  {
    return Registry;
  }

} /* namespace cubhb */

