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
 * hostname.hpp - hostname management class
 */

#ifndef _HOSTNAME_HPP_
#define _HOSTNAME_HPP_

#include <arpa/inet.h>
#include <string>
#include <netdb.h>

#include "error_code.h"
#include "packable_object.hpp"
#include "porting.h"
#if defined (WINDOWS)
#include "wintcp.h"
#else
#include "tcp.h"
#endif

namespace cubbase
{

  class hostname_type : public cubpacking::packable_object
  {
    private:
      std::string m_hostname;

    public:
      // C-tors
      hostname_type () = default;
      explicit hostname_type (const char *hostname)
	:m_hostname (hostname)
      {
	//
      }

      explicit hostname_type (const std::string &hostname)
	:m_hostname (hostname)
      {
	//
      }

      hostname_type (const hostname_type &other) = default;

      // Assignment operators
      hostname_type &
      operator= (const char *hostname)
      {
	m_hostname.assign (hostname);
	return *this;
      }

      hostname_type &
      operator= (const std::string &hostname)
      {
	m_hostname.assign (hostname);
	return *this;
      }

      hostname_type &operator= (const hostname_type &other) = default;

      // Comparison operators
      bool
      operator!= (const hostname_type &other) const
      {
	return ! (*this == other.m_hostname.c_str ());
      }

      bool
      operator!= (const std::string &other) const
      {
	return ! (*this == other.c_str ());
      }

      bool
      operator!= (const char *other) const
      {
	return ! (*this == other);
      }

      bool
      operator== (const hostname_type &other) const
      {
	return *this == other.m_hostname.c_str ();
      }

      bool
      operator== (const std::string &other) const
      {
	return *this == other.c_str ();
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
      operator== (const char *other) const
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

      int
      to_udp_sockaddr (int port, sockaddr *saddr, socklen_t *slen) const
      {
	// Construct address for UDP socket
	sockaddr_in udp_saddr;
	memset ((void *) &udp_saddr, 0, sizeof (udp_saddr));
	udp_saddr.sin_family = AF_INET;
	udp_saddr.sin_port = htons (port);

	int error_code = to_sin_addr (&udp_saddr.sin_addr);
	if (error_code != NO_ERROR)
	  {
	    return error_code;
	  }

	*slen = sizeof (udp_saddr);
	memcpy ((void *) saddr, (void *) &udp_saddr, *slen);

	return NO_ERROR;
      }

      int
      to_sin_addr (in_addr *addr) const
      {
	// First try to convert to the host name as a dotten-decimal number.
	// Only if that fails do we call gethostbyname.
	in_addr_t in_addr = inet_addr (as_c_str ());
	if (in_addr != INADDR_NONE)
	  {
	    memcpy ((void *) addr, (void *) &in_addr, sizeof (in_addr));
	  }
	else
	  {
	    int herr;
	    char buf[1024];
	    hostent hent, *hp = NULL;

	    if (gethostbyname_r (as_c_str (), &hent, buf, sizeof (buf), &hp, &herr) != 0 || hp == NULL)
	      {
		return ERR_CSS_TCP_HOST_NAME_ERROR;
	      }

	    memcpy ((void *) addr, (void *) hent.h_addr, hent.h_length);
	  }

	return NO_ERROR;
      }

      // Public functions
      int
      fetch ()
      {
	char hostname_cstr[CUB_MAXHOSTNAMELEN];
	int error_code = css_gethostname (hostname_cstr, CUB_MAXHOSTNAMELEN);
	if (error_code != NO_ERROR)
	  {
	    return error_code;
	  }

	m_hostname = hostname_cstr;

	return NO_ERROR;
      }

      const std::string &
      as_str () const
      {
	return m_hostname;
      }

      const char *
      as_c_str () const
      {
	return m_hostname.c_str ();
      }

      // pack/unpack functions
      size_t
      get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const override
      {
	return serializator.get_packed_string_size (m_hostname, start_offset);
      }

      void
      pack (cubpacking::packer &serializator) const override
      {
	serializator.pack_string (m_hostname);
      }

      void
      unpack (cubpacking::unpacker &deserializator) override
      {
	deserializator.unpack_string (m_hostname);
      }
  };

} /* namespace cubbase */

#endif /* _HOSTNAME_HPP_ */
