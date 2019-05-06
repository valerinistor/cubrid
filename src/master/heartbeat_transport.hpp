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

  enum message_type
  {
    HEARTBEAT,
  };

  class response_type
  {
    public:
      response_type ()
	: m_buffer ()
      {
	//
      }

      template <typename T>
      void set_body (message_type type, T &t)
      {
	cubpacking::packer packer;

	size_t total_size = t.get_packed_size (packer, 0);
	m_buffer.extend_to (total_size + packer.get_packed_int_size (0));

	packer.set_buffer (m_buffer.get_ptr (), total_size);

	packer.pack_to_int (type);
	packer.pack_overloaded (t);
      }

    private:
      friend class request_type;

      cubmem::extensible_block m_buffer;
  };

  class request_type
  {
    public:
      request_type (const char *buffer, std::size_t buffer_size)
	: m_sfd (INVALID_SOCKET)
	, m_saddr (NULL)
	, m_saddr_len (0)
	, m_buffer (buffer)
	, m_buffer_size (buffer_size)
      {
	//
      }

      request_type (SOCKET sfd, const char *buffer, std::size_t buffer_size, const sockaddr *saddr, socklen_t saddr_len)
	: m_sfd (sfd)
	, m_saddr (saddr)
	, m_saddr_len (saddr_len)
	, m_buffer (buffer)
	, m_buffer_size (buffer_size)
      {
	//
      }

      message_type
      get_message_type () const
      {
	cubpacking::unpacker unpacker (m_buffer, m_buffer_size);
	message_type type;
	unpacker.unpack_from_int (type);

	return type;
      }

      template <typename T>
      void get_body (T &t) const
      {
	cubpacking::unpacker unpacker (m_buffer, m_buffer_size);

	message_type type;
	unpacker.unpack_from_int (type);
	unpacker.unpack_overloaded (t);
      }

      int reply (const response_type &request) const;

      const sockaddr *
      get_saddr () const
      {
	return m_saddr;
      }
      socklen_t
      get_saddr_len () const
      {
	return m_saddr_len;
      }

    private:
      friend class udp_server;

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

      udp_server (const udp_server &other) = delete; // Copy c-tor
      udp_server &operator= (const udp_server &other) = delete; // Copy assignment

      udp_server (udp_server &&other) = delete; // Move c-tor
      udp_server &operator= (udp_server &&other) = delete; // Move assignment

      int start ();
      void stop ();

      int remote_call (const cubbase::hostname_type &hostname, const request_type &request) const;

    private:
      std::thread m_thread;
      bool m_shutdown;
      int m_port;
      SOCKET m_sfd;

      int listen ();
      static void poll_func (udp_server *arg);

      void on_request (const request_type &request) const;
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

      void handle (const request_type &request) const
      {
	auto found = m_handlers.find (request.get_message_type ());
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

  int start_server (int port);
  void stop_server ();
  const udp_server &get_server ();

  handler_registry &get_handler_registry ();

} /* namespace cubhb */

#endif /* _HEARTBEAT_TRANSPORT_HPP_ */
