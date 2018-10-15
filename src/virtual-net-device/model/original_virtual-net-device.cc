/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 INESC Porto
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
 * Author:  Gustavo J. A. M. Carneiro  <gjc@inescporto.pt>
 */

#include "ns3/log.h"
#include "ns3/original_queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "original_virtual-net-device.h"
#include "ns3/original_channel.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VirtualNetDevice1");

NS_OBJECT_ENSURE_REGISTERED (VirtualNetDevice1);

TypeId
VirtualNetDevice1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VirtualNetDevice1")
    .SetParent<NetDevice> ()
    .SetGroupName ("VirtualNetDevice1")
    .AddConstructor<VirtualNetDevice1> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&VirtualNetDevice1::SetMtu,
                                         &VirtualNetDevice1::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived "
                     "for transmission by this device",
                     MakeTraceSourceAccessor (&VirtualNetDevice1::m_macTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&VirtualNetDevice1::m_macPromiscRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&VirtualNetDevice1::m_macRxTrace),
                     "ns3::Packet::TracedCallback")
    //
    // Trace sources designed to simulate a packet sniffer facility (tcpdump). 
    //
    .AddTraceSource ("Sniffer", 
                     "Trace source simulating a non-promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&VirtualNetDevice1::m_snifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PromiscSniffer", 
                     "Trace source simulating a promiscuous "
                     "packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&VirtualNetDevice1::m_promiscSnifferTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

VirtualNetDevice1::VirtualNetDevice1 ()
{
  m_needsArp = false;
  m_supportsSendFrom = true;
  m_isPointToPoint = true;
}


void
VirtualNetDevice1::SetSendCallback (SendCallback sendCb)
{
  m_sendCb = sendCb;
}

void
VirtualNetDevice1::SetNeedsArp (bool needsArp)
{
  m_needsArp = needsArp;
}

void
VirtualNetDevice1::SetSupportsSendFrom (bool supportsSendFrom)
{
  m_supportsSendFrom = supportsSendFrom;
}

void
VirtualNetDevice1::SetIsPointToPoint (bool isPointToPoint)
{
  m_isPointToPoint = isPointToPoint;
}

bool
VirtualNetDevice1::SetMtu (const uint16_t mtu)
{
  m_mtu = mtu;
  return true;
}


VirtualNetDevice1::~VirtualNetDevice1()
{
  NS_LOG_FUNCTION_NOARGS ();
}


void VirtualNetDevice1::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  NetDevice1::DoDispose ();
}

bool
VirtualNetDevice1::Receive (Ptr<Packet> packet, uint16_t protocol,
                           const Address &source, const Address &destination,
                           PacketType packetType)
{
  //
  // Check if this device is configure as an OpenFlow switch port.
  //
  if (!m_openFlowRxCallback.IsNull ())
    {
      // For all kinds of packetType we receive, we hit the promiscuous sniffer
      // hook. If the packet is addressed to this device (which is not supposed
      // to happen in normal situations), we also hit the non-promiscuous
      // sniffer hook, but in both cases we don't forward the packt up the
      // stack.
      m_promiscSnifferTrace (packet);
      if (packetType != PACKET_OTHERHOST)
        {
          m_snifferTrace (packet);
        }

      // We then forward the original packet to the OpenFlow receive callback
      // for all kinds of packetType we receive (broadcast, multicast, host or
      // other host).
      m_openFlowRxCallback (this, packet, protocol, source, destination, packetType);
      return true;
    }

  // 
  // For all kinds of packetType we receive, we hit the promiscuous sniffer
  // hook and pass a copy up to the promiscuous callback.  Pass a copy to 
  // make sure that nobody messes with our packet.
  //
  m_promiscSnifferTrace (packet);
  if (!m_promiscRxCallback.IsNull ())
    {
      m_macPromiscRxTrace (packet);
      m_promiscRxCallback (this, packet, protocol, source, destination, packetType);
    }

  //
  // If this packet is not destined for some other host, it must be for us
  // as either a broadcast, multicast or unicast.  We need to hit the mac
  // packet received trace hook and forward the packet up the stack.
  //
  if (packetType != PACKET_OTHERHOST)
    {
      m_snifferTrace (packet);
      m_macRxTrace (packet);
      return m_rxCallback (this, packet, protocol, source);
    }
  return true;
}


void
VirtualNetDevice1::SetIfIndex (const uint32_t index)
{
  m_index = index;
}

uint32_t
VirtualNetDevice1::GetIfIndex (void) const
{
  return m_index;
}

Ptr<Channel1>
VirtualNetDevice1::GetChannel (void) const
{
  return Ptr<Channel1> (); //sjkang_x
 //retrun Ptr<Channel>();
}

Address
VirtualNetDevice1::GetAddress (void) const
{
  return m_myAddress;
}

void
VirtualNetDevice1::SetAddress (Address addr)
{
  m_myAddress = addr;
}

uint16_t
VirtualNetDevice1::GetMtu (void) const
{
  return m_mtu;
}

bool
VirtualNetDevice1::IsLinkUp (void) const
{
  return true;
}

void
VirtualNetDevice1::AddLinkChangeCallback (Callback<void> callback)
{
}

bool
VirtualNetDevice1::IsBroadcast (void) const
{
  return true;
}

Address
VirtualNetDevice1::GetBroadcast (void) const
{
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
VirtualNetDevice1::IsMulticast (void) const
{
  return false;
}

Address VirtualNetDevice1::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

Address VirtualNetDevice1::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}


bool
VirtualNetDevice1::IsPointToPoint (void) const
{
  return m_isPointToPoint;
}

bool
VirtualNetDevice1::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  m_macTxTrace (packet);
  if (m_sendCb (packet, GetAddress (), dest, protocolNumber))
    {
      return true;
    }
  return false;
}

bool
VirtualNetDevice1::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_ASSERT (m_supportsSendFrom);
  m_macTxTrace (packet);
  if (m_sendCb (packet, source, dest, protocolNumber))
    {
      return true;
    }
  return false;
}

Ptr<Node>
VirtualNetDevice1::GetNode (void) const
{
  return m_node;
}

void
VirtualNetDevice1::SetNode (Ptr<Node> node)
{
  m_node = node;
}

bool
VirtualNetDevice1::NeedsArp (void) const
{
  return m_needsArp;
}

void
VirtualNetDevice1::SetReceiveCallback (NetDevice1::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
VirtualNetDevice1::SetPromiscReceiveCallback (NetDevice1::PromiscReceiveCallback cb)
{
  m_promiscRxCallback = cb;
}

bool
VirtualNetDevice1::SupportsSendFrom () const
{
  return m_supportsSendFrom;
}

bool VirtualNetDevice1::IsBridge (void) const
{
  return false;
}

void
VirtualNetDevice1::SetOpenFlowReceiveCallback (NetDevice1::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
  m_openFlowRxCallback = cb;
}

} // namespace ns3
