/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab

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
 */

#ifndef NGC_S1AP_H
#define NGC_S1AP_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

#include "ns3/ngc-s1ap-sap.h"

#include <map>

namespace ns3 {


class S1apIfaceInfo : public SimpleRefCount<S1apIfaceInfo>
{
public:
  S1apIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket);
  virtual ~S1apIfaceInfo (void);

  S1apIfaceInfo& operator= (const S1apIfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr;
  Ptr<Socket>   m_localCtrlPlaneSocket;
};


class S1apConnectionInfo : public SimpleRefCount<S1apConnectionInfo>
{
public:
  S1apConnectionInfo (uint16_t enbId, uint16_t mmeId);
  virtual ~S1apConnectionInfo (void);

  S1apConnectionInfo& operator= (const S1apConnectionInfo &);

public:
  uint16_t m_enbId;
  uint16_t m_mmeId;
};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the S1AP interface
 */
class NgcS1apEnb : public Object
{
  friend class MemberNgcS1apSapEnbProvider<NgcS1apEnb>;

public:
  /** 
   * Constructor
   */
  NgcS1apEnb (Ptr<Socket> localSocket, Ipv4Address enbAddress, Ipv4Address mmeAddress, uint16_t cellId, uint16_t mmeId);

  /**
   * Destructor
   */
  virtual ~NgcS1apEnb (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the S1ap SAP User to be used by this NGC S1ap eNB entity in order to call methods of ngcEnbApplication
   */
  void SetNgcS1apSapEnbUser (NgcS1apSapEnb * s);

  /**
   * \return the S1ap SAP Provider interface offered by this NGC S1ap entity
   */
  NgcS1apSapEnbProvider* GetNgcS1apSapEnbProvider ();


  /**
   * Add an S1ap interface to this NGC S1ap entity
   * \param enbId the cell ID of the eNodeB on which this is installed
   * \param enbAddress the address of the eNodeB on which this is installed
   * \param mmeId the ID of the MME to which the eNB is connected
   * \param mmeAddress the address of the MME to which the eNB is connected
   * \param the socket created in the Ngc Helper
   */
  void AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t mmeId, Ipv4Address mmeAddress, Ptr<Socket> localS1apSocket);


  /** 
   * Method to be assigned to the recv callback of the S1ap-C (S1ap Control Plane) socket.
   * It is called when the eNB receives a packet from the MME on the S1ap interface
   * 
   * \param socket socket of the S1ap interface
   */
  void RecvFromS1apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by NgcS1apSapEnbProvider
  virtual void DoSendInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t stmsi, uint16_t ecgi);
  virtual void DoSendErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<NgcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication );
  virtual void DoSendInitialContextSetupResponse (uint64_t mmeUeS1Id,
                                                  uint16_t enbUeS1Id,
                                                  std::list<NgcS1apSap::ErabSetupItem> erabSetupList);
  virtual void DoSendPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
  ;

  NgcS1apSapEnb* m_s1apSapUser;
  NgcS1apSapEnbProvider* m_s1apSapProvider;


private:

  /**
   * Map the mmeId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the S1ap message
   */
  std::map < uint16_t, Ptr<S1apIfaceInfo> > m_s1apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the S1ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the S1ap interface
   */
  std::map < Ptr<Socket>, Ptr<S1apConnectionInfo> > m_s1apInterfaceCellIds;

  /**
   * UDP port to be used for the S1ap interfaces: S1ap
   */
  uint16_t m_s1apUdpPort;

  /**
   * Mme ID, stored as a private variable until more than one MME will be implemented
   */
  uint16_t m_mmeId; 

};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the S1AP interface
 */
class NgcS1apMme : public Object
{
  friend class MemberNgcS1apSapMmeProvider<NgcS1apMme>;

public:
  /** 
   * Constructor
   * \param the socket opened on the node in which this object is installed
   */
  NgcS1apMme (const Ptr<Socket> s1apSocket, uint16_t mmeId);

  /**
   * Destructor
   */
  virtual ~NgcS1apMme (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the S1ap SAP User to be used by this NGC S1ap Mme entity in order to call methods of ngcMmeApplication
   */
  void SetNgcS1apSapMmeUser (NgcS1apSapMme * s);

  /**
   * \return the S1ap SAP Provider interface offered by this NGC S1ap entity
   */
  NgcS1apSapMmeProvider* GetNgcS1apSapMmeProvider ();


  /**
   * Add an S1ap interface to this NGC S1ap entity
   * \param enbId the cell ID of the eNodeB which the MME is connected to
   * \param enbAddress the address of the eNodeB which the MME is connected to
   */
  void AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress);


  /** 
   * Method to be assigned to the recv callback of the S1ap-C (S1ap Control Plane) socket.
   * It is called when the MME receives a packet from the eNB on the S1ap interface
   * 
   * \param socket socket of the S1ap interface
   */
  void RecvFromS1apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by NgcS1apSapMmeProvider
  virtual void DoSendInitialContextSetupRequest (uint64_t mmeUeS1Id,
                                           uint16_t enbUeS1Id,
                                           std::list<NgcS1apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId);

  virtual void DoSendPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi, 
                                        std::list<NgcS1apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);

  NgcS1apSapMme* m_s1apSapUser;
  NgcS1apSapMmeProvider* m_s1apSapProvider;


private:

  /**
   * Map the enbId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the S1ap message
   */
  std::map < uint16_t, Ptr<S1apIfaceInfo> > m_s1apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the S1ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the S1ap interface
   */
  std::map < Ptr<Socket>, Ptr<S1apConnectionInfo> > m_s1apInterfaceCellIds;

  /**
   * UDP port to be used for the S1ap interfaces: S1ap
   */
  uint16_t m_s1apUdpPort;

  uint16_t m_mmeId; // ID of the MME to which this S1AP endpoint is installed

  Ptr<Socket> m_localS1APSocket; // local socket to receive from the eNBs S1AP endpoints

};

} //namespace ns3

#endif // NGC_S1AP_H
