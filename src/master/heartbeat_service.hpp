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
 * heartbeat_service.hpp - TODO - add brief documentation
 */

#ifndef _HEARTBEAT_SERVICE_HPP_
#define _HEARTBEAT_SERVICE_HPP_

#include "heartbeat_transport.hpp"

namespace cubhb
{
  class cluster;

  class header : public cubpacking::packable_object
  {
    public:
      header ();
      header (bool is_request, const cubbase::hostname_type &dest_hostname, const cluster &c);

      const bool &is_request () const;
      //const node_state &get_state () const;
      const std::string &get_group_id () const;
      const cubbase::hostname_type &get_orig_hostname () const;
      const cubbase::hostname_type &get_dest_hostname () const;

      size_t get_packed_size (cubpacking::packer &serializator, std::size_t start_offset) const override;
      void pack (cubpacking::packer &serializator) const override;
      void unpack (cubpacking::unpacker &deserializator) override;

    private:
      bool m_is_request;

      //node_state m_state;
      std::string m_group_id;
      cubbase::hostname_type m_orig_hostname;
      cubbase::hostname_type m_dest_hostname;
  };

  class heartbeat_service
  {
    public:
      heartbeat_service ();

      int send_heartbeat_request (const cubbase::hostname_type &dest_hostname);
      void on_hearbeat_request (const header &header_);

    private:
      void handle (const request_type &request, response_type &response);
  };

} /* namespace cubhb */

#endif /* _HEARTBEAT_SERVICE_HPP_ */
