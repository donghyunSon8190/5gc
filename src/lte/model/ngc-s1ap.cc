/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Author: Michele Polese <michele.polese@gmail.com> 
 *          Support for real S1AP link
 */

#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/ngc-gtpu-header.h"
#include <ns3/simulator.h>

#include "ns3/ngc-s1ap-header.h"
#include "ns3/ngc-s1ap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcS1ap");

S1apIfaceInfo::S1apIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket)
{
  m_remoteIpAddr = remoteIpAddr;
  m_localCtrlPlaneSocket = localCtrlPlaneSocket;
}

S1apIfaceInfo::~S1apIfaceInfo (void)
{
  m_localCtrlPlaneSocket = 0;
}

S1apIfaceInfo& 
S1apIfaceInfo::operator= (const S1apIfaceInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_remoteIpAddr = value.m_remoteIpAddr;
  m_localCtrlPlaneSocket = value.m_localCtrlPlaneSocket;
  return *this;
}

///////////////////////////////////////////

S1apConnectionInfo::S1apConnectionInfo (uint16_t enbId, uint16_t mmeId)
{
  m_enbId = enbId;
  m_mmeId = mmeId;
}

S1apConnectionInfo::~S1apConnectionInfo (void)
{
  m_enbId = 0;
  m_mmeId = 0;
}

S1apConnectionInfo& 
S1apConnectionInfo::operator= (const S1apConnectionInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_enbId = value.m_enbId;
  m_mmeId = value.m_mmeId;
  return *this;
}

///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1apEnb);

NgcS1apEnb::NgcS1apEnb (Ptr<Socket> localSocket, Ipv4Address enbAddress, Ipv4Address mmeAddress, uint16_t cellId, uint16_t mmeId)
  : m_s1apUdpPort (36412) // As defined by IANA
{
  NS_LOG_FUNCTION (this);
  AddS1apInterface(cellId, enbAddress, mmeId, mmeAddress, localSocket);
  m_s1apSapProvider = new MemberNgcS1apSapEnbProvider<NgcS1apEnb> (this);
}

NgcS1apEnb::~NgcS1apEnb ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcS1apEnb::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_s1apInterfaceSockets.clear ();
  m_s1apInterfaceCellIds.clear ();
  delete m_s1apSapProvider;
}

TypeId
NgcS1apEnb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1apEnb")
    .SetParent<Object> ()
    .SetGroupName("Lte");
  return tid;
}

void
NgcS1apEnb::SetNgcS1apSapEnbUser (NgcS1apSapEnb * s)
{
  NS_LOG_FUNCTION (this << s);
  m_s1apSapUser = s;
}

NgcS1apSapEnbProvider*
NgcS1apEnb::GetNgcS1apSapEnbProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_s1apSapProvider;
}


void
NgcS1apEnb::AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t mmeId, Ipv4Address mmeAddress,
                       Ptr<Socket> localS1apSocket)
{
  NS_LOG_FUNCTION (this << enbId << enbAddress << mmeId << mmeAddress);

  localS1apSocket->SetRecvCallback (MakeCallback (&NgcS1apEnb::RecvFromS1apSocket, this));

  NS_ASSERT_MSG (m_s1apInterfaceSockets.find (mmeId) == m_s1apInterfaceSockets.end (),
                 "Mapping for mmeId = " << mmeId << " is already known");
  m_s1apInterfaceSockets [mmeId] = Create<S1apIfaceInfo> (mmeAddress, localS1apSocket);

  // TODO m_mmeId is initialized once since one mme is connected to this enb interface, consider when extending
  m_mmeId = mmeId;
}


void 
NgcS1apEnb::RecvFromS1apSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv S1ap message: S1AP eNB: from Socket at time " << Simulator::Now ().GetSeconds());
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NgcS1APHeader s1apHeader;
  packet->RemoveHeader (s1apHeader);

  NS_LOG_LOGIC ("S1ap header: " << s1apHeader);

  uint8_t procedureCode = s1apHeader.GetProcedureCode ();

  if (procedureCode == NgcS1APHeader::InitialContextSetupRequest)
  {
    NS_LOG_LOGIC ("Recv S1ap message: INITIAL CONTEXT SETUP REQUEST");
    NgcS1APInitialContextSetupRequestHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    NS_LOG_INFO ("S1ap Initial Context Setup Request " << reqHeader);

    uint64_t mmeUeS1apId = reqHeader.GetMmeUeS1Id();
    uint16_t enbUeS1apId = reqHeader.GetEnbUeS1Id();
    std::list<NgcS1apSap::ErabToBeSetupItem> erabToBeSetup = reqHeader.GetErabToBeSetupItem ();
    
    NS_LOG_LOGIC ("mmeUeS1apId " << mmeUeS1apId);
    NS_LOG_LOGIC ("enbUeS1apId " << enbUeS1apId);

    m_s1apSapUser->InitialContextSetupRequest(mmeUeS1apId, enbUeS1apId, erabToBeSetup);
  } 
  else if (procedureCode == NgcS1APHeader::PathSwitchRequestAck)
  {
    NS_LOG_LOGIC ("Recv S1ap message: PATH SWITCH REQUEST ACK");
    NgcS1APPathSwitchRequestAcknowledgeHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    NS_LOG_INFO ("S1ap Path Switch Request Acknowledge Header " << reqHeader);

    uint64_t mmeUeS1apId = reqHeader.GetMmeUeS1Id();
    uint16_t enbUeS1apId = reqHeader.GetEnbUeS1Id();
    uint16_t ecgi = reqHeader.GetEcgi();
    std::list<NgcS1apSap::ErabSwitchedInUplinkItem> pathErab = reqHeader.GetErabSwitchedInUplinkItemList ();
    
    NS_LOG_LOGIC ("mmeUeS1apId " << mmeUeS1apId);
    NS_LOG_LOGIC ("enbUeS1apId " << enbUeS1apId);
    NS_LOG_LOGIC ("ecgi " << ecgi);

    m_s1apSapUser->PathSwitchRequestAcknowledge(enbUeS1apId, mmeUeS1apId, ecgi, pathErab);
  }  
  else
  {
    NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!");
  }
}


//
// Implementation of the S1ap SAP Provider
//
void
NgcS1apEnb::DoSendInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t stmsi, uint16_t ecgi) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
  NS_LOG_LOGIC("stmsi = " << stmsi);
  NS_LOG_LOGIC("ecgi = " << ecgi);

  // TODO check if an assert is needed

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [m_mmeId]; // in case of multiple mme, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address mmeIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("mmeIpAddr = " << mmeIpAddr);

  NS_LOG_INFO ("Send S1ap message: INITIAL UE MESSAGE " << Simulator::Now ().GetSeconds());

  // build the header
  NgcS1APInitialUeMessageHeader initialMessage;
  initialMessage.SetMmeUeS1Id(mmeUeS1Id);
  initialMessage.SetEnbUeS1Id(enbUeS1Id);
  initialMessage.SetSTmsi(stmsi);
  initialMessage.SetEcgi(ecgi);
  NS_LOG_INFO ("S1ap Initial Message header " << initialMessage);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::InitialUeMessage);
  s1apHeader.SetLengthOfIes (initialMessage.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (initialMessage.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (initialMessage);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  std::cout << "Sends packet to " <<mmeIpAddr << ":" <<m_s1apUdpPort << std::endl; // jhlim
  sourceSocket->SendTo (packet, 0, InetSocketAddress (mmeIpAddr, m_s1apUdpPort));
}

void 
NgcS1apEnb::DoSendErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<NgcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication )
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);

  // TODO check if an assert is needed

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [m_mmeId]; // in case of multiple mme, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address mmeIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("mmeIpAddr = " << mmeIpAddr);

  NS_LOG_INFO ("Send S1ap message: E-RAB RELEASE INDICATION " << Simulator::Now ().GetSeconds());

  NgcS1APErabReleaseIndicationHeader indHeader;
  
  indHeader.SetMmeUeS1Id(mmeUeS1Id);
  indHeader.SetEnbUeS1Id(enbUeS1Id);
  indHeader.SetErabReleaseIndication(erabToBeReleaseIndication);
  NS_LOG_INFO ("S1ap E-rab Release Indication header " << indHeader);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::ErabReleaseIndication);
  s1apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (mmeIpAddr, m_s1apUdpPort));
}

void 
NgcS1apEnb::DoSendInitialContextSetupResponse (uint64_t mmeUeS1Id,
              uint16_t enbUeS1Id,
              std::list<NgcS1apSap::ErabSetupItem> erabSetupList) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);

  // TODO check if an assert is needed

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [m_mmeId]; // in case of multiple mme, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address mmeIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("mmeIpAddr = " << mmeIpAddr);

  NS_LOG_INFO ("Send S1ap message: INITIAL CONTEXT SETUP RESPONSE " << Simulator::Now ().GetSeconds());

  NgcS1APInitialContextSetupResponseHeader indHeader;
  
  indHeader.SetMmeUeS1Id(mmeUeS1Id);
  indHeader.SetEnbUeS1Id(enbUeS1Id);
  indHeader.SetErabSetupItem(erabSetupList);
  NS_LOG_INFO ("S1AP Initial Context Setup Response header " << indHeader);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::InitialContextSetupResponse);
  s1apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (mmeIpAddr, m_s1apUdpPort));
}

void 
NgcS1apEnb::DoSendPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, 
            std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
  NS_LOG_LOGIC("ecgi = " << gci);

  // TODO check if an assert is needed

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [m_mmeId]; // in case of multiple mme, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address mmeIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("mmeIpAddr = " << mmeIpAddr);

  NS_LOG_INFO ("Send S1ap message: PATH SWITCH REQUEST " << Simulator::Now ().GetSeconds());

  NgcS1APPathSwitchRequestHeader indHeader;
  
  indHeader.SetMmeUeS1Id(mmeUeS1Id);
  indHeader.SetEnbUeS1Id(enbUeS1Id);
  indHeader.SetEcgi(gci);
  indHeader.SetErabSwitchedInDownlinkItemList(erabToBeSwitchedInDownlinkList);
  NS_LOG_INFO ("S1AP Path Switch Request header " << indHeader);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::PathSwitchRequest);
  s1apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (mmeIpAddr, m_s1apUdpPort));
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (NgcS1apMme);

NgcS1apMme::NgcS1apMme (const Ptr<Socket> s1apSocket, uint16_t mmeId)
  : m_s1apUdpPort (36412) // As defined by IANA
{
  NS_LOG_FUNCTION (this);
  m_localS1APSocket = s1apSocket;
  m_s1apSapProvider = new MemberNgcS1apSapMmeProvider<NgcS1apMme> (this);
  m_localS1APSocket->SetRecvCallback (MakeCallback (&NgcS1apMme::RecvFromS1apSocket, this));
  m_mmeId = mmeId;
}

NgcS1apMme::~NgcS1apMme ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcS1apMme::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_s1apInterfaceSockets.clear ();
  m_s1apInterfaceCellIds.clear ();
  delete m_s1apSapProvider;
}

TypeId
NgcS1apMme::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1apMme")
    .SetParent<Object> ()
    .SetGroupName("Lte");
  return tid;
}

void
NgcS1apMme::SetNgcS1apSapMmeUser (NgcS1apSapMme * s)
{
  NS_LOG_FUNCTION (this << s);
  m_s1apSapUser = s;
}

NgcS1apSapMmeProvider*
NgcS1apMme::GetNgcS1apSapMmeProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_s1apSapProvider;
}


void
NgcS1apMme::AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress)
{
  NS_LOG_FUNCTION (this << enbId << enbAddress << m_mmeId);

  NS_ASSERT_MSG (m_s1apInterfaceSockets.find (enbId) == m_s1apInterfaceSockets.end (),
                 "Mapping for enbId = " << enbId << " is already known");
  m_s1apInterfaceSockets [enbId] = Create<S1apIfaceInfo> (enbAddress, m_localS1APSocket); // TODO m_localS1APSocket is useless
}


void 
NgcS1apMme::RecvFromS1apSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv S1ap message: S1AP MME: from Socket " << Simulator::Now ().GetSeconds());
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NgcS1APHeader s1apHeader;
  packet->RemoveHeader (s1apHeader);

  NS_LOG_LOGIC ("S1ap header: " << s1apHeader);

  uint8_t procedureCode = s1apHeader.GetProcedureCode ();

  if (procedureCode == NgcS1APHeader::InitialUeMessage)
  {
    NS_LOG_LOGIC ("Recv S1ap message: INITIAL UE MESSAGE");
    NgcS1APInitialUeMessageHeader initialMessage;
    packet->RemoveHeader(initialMessage);
    NS_LOG_INFO ("S1ap Initial Message header " << initialMessage);

    uint64_t mmeUeS1Id = initialMessage.GetMmeUeS1Id();
    uint16_t enbUeS1Id = initialMessage.GetEnbUeS1Id();
    uint64_t stmsi = initialMessage.GetSTmsi();
    uint16_t ecgi = initialMessage.GetEcgi();

    NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
    NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
    NS_LOG_LOGIC("stmsi = " << stmsi);
    NS_LOG_LOGIC("ecgi = " << ecgi);

    // TODO check if ASSERT is needed

    m_s1apSapUser->InitialUeMessage(mmeUeS1Id, enbUeS1Id, stmsi, ecgi);

  }
  else if (procedureCode == NgcS1APHeader::PathSwitchRequest)
  {
    NS_LOG_LOGIC ("Recv S1ap message: PATH SWITCH REQUEST " << Simulator::Now ().GetSeconds());
    NgcS1APPathSwitchRequestHeader psrHeader;
    packet->RemoveHeader(psrHeader);
    NS_LOG_INFO ("S1ap Path Switch Request header " << psrHeader);
    
    uint64_t mmeUeS1Id = psrHeader.GetMmeUeS1Id();
    uint16_t enbUeS1Id = psrHeader.GetEnbUeS1Id();
    uint16_t ecgi = psrHeader.GetEcgi();

    std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> erabToBeSwitched = psrHeader.GetErabSwitchedInDownlinkItemList ();


    NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
    NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
    NS_LOG_LOGIC("ecgi = " << ecgi);

    m_s1apSapUser->PathSwitchRequest (enbUeS1Id, mmeUeS1Id, ecgi, erabToBeSwitched);
  }
  else if (procedureCode == NgcS1APHeader::ErabReleaseIndication)
  {
   NS_LOG_LOGIC ("Recv S1ap message: E-RAB RELEASE INDICATION " << Simulator::Now ().GetSeconds());
    NgcS1APErabReleaseIndicationHeader eriHeader;
    packet->RemoveHeader(eriHeader);
    NS_LOG_INFO ("S1ap Erab Release Indication header " << eriHeader);
    
    uint64_t mmeUeS1Id = eriHeader.GetMmeUeS1Id();
    uint16_t enbUeS1Id = eriHeader.GetEnbUeS1Id();

    std::list<NgcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication = eriHeader.GetErabToBeReleaseIndication ();

    NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
    NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);

    m_s1apSapUser->ErabReleaseIndication (mmeUeS1Id, enbUeS1Id, erabToBeReleaseIndication);
  }
  else
  {
    NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!");
  }
}

void 
NgcS1apMme::DoSendInitialContextSetupRequest (uint64_t mmeUeS1Id,
                                           uint16_t enbUeS1Id,
                                           std::list<NgcS1apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_s1apInterfaceSockets.find (cellId) != m_s1apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send S1ap message: INITIAL CONTEXT SETUP REQUEST " << Simulator::Now ().GetSeconds());

  NgcS1APInitialContextSetupRequestHeader reqHeader;
  
  reqHeader.SetMmeUeS1Id(mmeUeS1Id);
  reqHeader.SetEnbUeS1Id(enbUeS1Id);
  reqHeader.SetErabToBeSetupItem(erabToBeSetupList);
  NS_LOG_INFO ("S1AP Initial Context Setup Request header " << reqHeader);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::InitialContextSetupRequest);
  s1apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  m_localS1APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_s1apUdpPort));
}

void 
NgcS1apMme::DoSendPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi, 
                                        std::list<NgcS1apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  // cgi is the cellId of the other endpoint of this interface
  uint16_t cellId = cgi;

  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("mmeUeS1apId = " << mmeUeS1Id);
  NS_LOG_LOGIC("enbUeS1apId = " << enbUeS1Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_s1apInterfaceSockets.find (cellId) != m_s1apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<S1apIfaceInfo> socketInfo = m_s1apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send S1ap message: PATH SWITCH REQUEST ACKNOWLEDGE " << Simulator::Now ().GetSeconds());

  NgcS1APPathSwitchRequestAcknowledgeHeader reqHeader;
  
  reqHeader.SetMmeUeS1Id(mmeUeS1Id);
  reqHeader.SetEnbUeS1Id(enbUeS1Id);
  reqHeader.SetEcgi(cgi);
  reqHeader.SetErabSwitchedInUplinkItemList(erabToBeSwitchedInUplinkList);
  NS_LOG_INFO ("S1AP PathSwitchRequestAcknowledge header " << reqHeader);

  NgcS1APHeader s1apHeader;
  s1apHeader.SetProcedureCode (NgcS1APHeader::PathSwitchRequestAck);
  s1apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  s1apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("S1ap header: " << s1apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (s1apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the S1ap message through the socket
  m_localS1APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_s1apUdpPort));
}



} // namespace ns3
