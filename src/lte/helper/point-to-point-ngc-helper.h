/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *        Support for real S1AP link
 */

#ifndef POINT_TO_POINT_NGC_HELPER_H
#define POINT_TO_POINT_NGC_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/ngc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-helper.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class NgcSmfUpfApplication;
class NgcX2;
class NgcMme;
class NgcUeNas;
class NgcMmeApplication;
class NgcS1apEnb;
class NgcS1apMme;

/**
 * \ingroup lte
 * \brief Create an NGC network with PointToPoint links
 *
 * This Helper will create an NGC network topology comprising of a
 * single node that implements both the SMF and UPF functionality, and
 * an MME node. The S1-U, S1-AP, X2-U and X2-C interfaces are realized over
 * PointToPoint links. 
 */
class PointToPointNgcHelper : public NgcHelper
{
public:
  
  /** 
   * Constructor
   */
  PointToPointNgcHelper ();

  /** 
   * Destructor
   */  
  virtual ~PointToPointNgcHelper ();
  
  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  // inherited from NgcHelper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueLteDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, Ptr<NgcUeNas> ueNas, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetUpfNode ();
  virtual Ptr<Node> GetMmeNode ();
  virtual Ipv4InterfaceContainer AssignUeIpv4Address (NetDeviceContainer ueDevices);
  virtual Ipv4Address GetUeDefaultGatewayAddress ();



private:

  /** 
   * helper to assign addresses to UE devices as well as to the TUN device of the SMF/UPF
   */
  Ipv4AddressHelper m_ueAddressHelper; 
  
  /**
   * SMF-UPF network element
   */
  Ptr<Node> m_smfUpf; 

  /**
   * SMF-UPF application
   */
  Ptr<NgcSmfUpfApplication> m_smfUpfApp;

  /**
   * TUN device implementing tunneling of user data over GTP-U/UDP/IP
   */
  Ptr<VirtualNetDevice> m_tunDevice;

  /**
   * MME network element
   */
  Ptr<Node> m_mmeNode;

  /**
   * MME application
   */
  Ptr<NgcMmeApplication> m_mmeApp;

  /**
   * S1-U interfaces
   */

  /** 
   * helper to assign addresses to S1-U NetDevices 
   */
  Ipv4AddressHelper m_s1uIpv4AddressHelper; 

  /**
   * The data rate to be used for the next S1-U link to be created
   */
  DataRate m_s1uLinkDataRate;

  /**
   * The delay to be used for the next S1-U link to be created
   */
  Time     m_s1uLinkDelay;

  /**
   * The MTU of the next S1-U link to be created. Note that,
   * because of the additional GTP/UDP/IP tunneling overhead,
   * you need a MTU larger than the end-to-end MTU that you
   * want to support.
   */
  uint16_t m_s1uLinkMtu;

  /**
   * UDP port where the GTP-U Socket is bound, fixed by the standard as 2152
   */
  uint16_t m_gtpuUdpPort;

  /**
   * Map storing for each IMSI the corresponding eNB NetDevice
   */
  std::map<uint64_t, Ptr<NetDevice> > m_imsiEnbDeviceMap;

  /**
   * S1-AP interfaces
   */

  /** 
   * helper to assign addresses to S1-AP NetDevices 
   */
  Ipv4AddressHelper m_s1apIpv4AddressHelper; 

  /**
   * The data rate to be used for the next S1-AP link to be created
   */
  DataRate m_s1apLinkDataRate;

  /**
   * The delay to be used for the next S1-AP link to be created
   */
  Time     m_s1apLinkDelay;

  /**
   * The MTU of the next S1-AP link to be created. 
   */
  uint16_t m_s1apLinkMtu;

  /**
   * UDP port where the UDP Socket is bound, fixed by the standard as 
   * 36412 (it should be sctp, but it is not supported in ns-3)
   */
  uint16_t m_s1apUdpPort;

  /**
   * Map storing for each eNB the corresponding MME NetDevice
   */
  std::map<uint16_t, Ptr<NetDevice> > m_cellIdMmeDeviceMap;

  
  /** 
   * helper to assign addresses to X2 NetDevices 
   */
  Ipv4AddressHelper m_x2Ipv4AddressHelper;   

  /**
   * The data rate to be used for the next X2 link to be created
   */
  DataRate m_x2LinkDataRate;

  /**
   * The delay to be used for the next X2 link to be created
   */
  Time     m_x2LinkDelay;

  /**
   * The MTU of the next X2 link to be created. Note that,
   * because of some big X2 messages, you need a big MTU.
   */
  uint16_t m_x2LinkMtu;

};




} // namespace ns3

#endif // POINT_TO_POINT_NGC_HELPER_H