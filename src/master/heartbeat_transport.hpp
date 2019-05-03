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
 * heartbeat_transport.hpp - TODO - add brief documentation
 */

#ifndef _HEARTBEAT_TRANSPORT_HPP_
#define _HEARTBEAT_TRANSPORT_HPP_

#include "porting.h"
#include "packable_object.hpp"
#include "hostname.hpp"

#include <functional>
#include <thread>
#include <sys/socket.h>
#include <map>

namespace cubhb
{

  static const std::size_t BUFFER_SIZE = 4096;

  class transport
  {
      // TODO
  };

  class response_type
  {
    public:
      response_type ()
	: m_buffer (NULL)
	, m_buffer_size (0)
      {
	//
      }

      const char *
      get_buffer () const
      {
	return m_buffer;
      }

      std::size_t
      get_buffer_size () const
      {
	return m_buffer_size;
      }

    private:
      const char *m_buffer;
      std::size_t m_buffer_size;
  };

  enum message_type
  {
    HEARTBEAT,
  };

  class request_type
  {
    public:
      request_type (SOCKET sfd, const char *buffer, std::size_t buffer_size, const sockaddr *saddr, socklen_t saddr_len)
	: m_sfd (sfd)
	, m_saddr (saddr)
	, m_saddr_len (saddr_len)
	, m_buffer (buffer)
	, m_buffer_size (buffer_size)
      {
	//
      }

      message_type get_message_type () const;

      template <typename T>
      void get_body (T &t) const;

      int reply (const response_type &request) const;

    private:
      SOCKET m_sfd;
      const sockaddr *m_saddr;
      const socklen_t m_saddr_len;

      const char *m_buffer;
      std::size_t m_buffer_size;
  };

  class udp_server : public transport
  {
    public:
      explicit udp_server (int port);
      ~udp_server ();

      int start ();
      void stop ();

      void on_request (const request_type &request) const;

    private:
      std::thread m_thread;
      bool m_shutdown;

      int m_port;
      SOCKET m_sfd;

      int listen ();
      static void poll_internal (udp_server *arg);
  };

  class handler_registry
  {
    public:
      using handler_type = std::function<void (const request_type &, response_type &)>;

      handler_registry ()
	: m_handlers ()
      {
	//
      }

      void register_handler (message_type m_type, const handler_type &handler)
      {
	m_handlers.emplace (m_type, handler);
      }

      void handle (message_type m_type, const request_type &request) const
      {
	auto found = m_handlers.find (m_type);
	if (found == m_handlers.end ())
	  {
	    return;
	  }

	handler_type handler = found->second;

	response_type response;
	handler (request, response);
	request.reply (response);
      }

    private:
      using handlers_type = std::map<message_type, const handler_type &>;

      handlers_type m_handlers;
  };

  handler_registry &get_handler_registry ();

} /* namespace cubhb */

namespace cubhb
{

  template <typename T>
  void
  request_type::get_body (T &t) const
  {
    // // TODO [new slave]
    cubpacking::unpacker unpacker (m_buffer, m_buffer_size);
    t.unpack (unpacker);
  }

} /* namespace cubhb */

#endif /* _HEARTBEAT_TRANSPORT_HPP_ */
