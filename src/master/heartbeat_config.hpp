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
 * heartbeat_config.hpp - TODO - add brief documentation
 */

#ifndef _HEARTBEAT_CONFIG_HPP_
#define _HEARTBEAT_CONFIG_HPP_

namespace cubhb
{

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

  class config
  {
    public:
      virtual ~config () = default;

      virtual int get_port () const = 0;
      virtual node_state get_state () const = 0;
      virtual int get_heartbeat_interval () const = 0;
      virtual const char *get_master_host () const = 0;
      virtual const char *get_node_list () const = 0;
      virtual const char *get_replica_list () const = 0;
      virtual const char *get_ping_hosts () const = 0;
  };

  class config_file : public config
  {
    public:
      ~config_file () override = default;

      int get_port () const override;
      node_state get_state () const override;
      int get_heartbeat_interval () const override;
      const char *get_master_host () const override;
      const char *get_node_list () const override;
      const char *get_replica_list () const override;
      const char *get_ping_hosts () const override;
  };

} /* namespace cubhb */

#endif /* _HEARTBEAT_CONFIG_HPP_ */
