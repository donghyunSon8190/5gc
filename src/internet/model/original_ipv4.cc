/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h" 
#include "ns3/node.h" 
#include "ns3/boolean.h"
#include "original_ipv4.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv41");

NS_OBJECT_ENSURE_REGISTERED (Ipv41);

TypeId 
Ipv41::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv41")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddAttribute ("IpForward", "Globally enable or disable IP forwarding for all current and future Ipv41 devices.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Ipv41::SetIpForward,
                                        &Ipv41::GetIpForward),
                   MakeBooleanChecker ())
    .AddAttribute ("WeakEsModel", 
                   "RFC1122 term for whether host accepts datagram with a dest. address on another interface",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Ipv41::SetWeakEsModel,
                                        &Ipv41::GetWeakEsModel),
                   MakeBooleanChecker ())
#if 0
    .AddAttribute ("MtuDiscover", "If enabled, every outgoing ip packet will have the DF flag set.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpSocket::SetMtuDiscover,
                                        &UdpSocket::GetMtuDiscover),
                   MakeBooleanChecker ())
#endif
  ;
  return tid;
}

Ipv41::Ipv41 ()
{
  NS_LOG_FUNCTION (this);
}

Ipv41::~Ipv41 ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
