/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef NR_TEST_ENTITIES_H
#define NR_TEST_ENTITIES_H

#include "ns3/simulator.h"
#include "ns3/test.h"

#include "ns3/nr-mac-sap.h"
#include "ns3/nr-rlc-sap.h"
#include "ns3/nr-pdcp-sap.h"

#include "ns3/net-device.h"
#include <ns3/ngc-enb-n2-sap.h>

namespace ns3 {

/**
 * This class implements a testing RRC entity
 */
class NrTestRrc : public Object
{
    friend class NrPdcpSpecificNrPdcpSapUser<NrTestRrc>;
//   friend class EnbMacMemberNrEnbCmacSapProvider;
//   friend class EnbMacMemberNrMacSapProvider<NrTestMac>;
//   friend class EnbMacMemberFfMacSchedSapUser;
//   friend class EnbMacMemberFfMacCschedSapUser;
//   friend class EnbMacMemberNrEnbPhySapUser;

  public:
    static TypeId GetTypeId (void);

    NrTestRrc (void);
    virtual ~NrTestRrc (void);
    virtual void DoDispose (void);


    /**
    * \brief Set the PDCP SAP provider
    * \param s a pointer to the PDCP SAP provider
    */
    void SetNrPdcpSapProvider (NrPdcpSapProvider* s);
    /**
    * \brief Get the PDCP SAP user
    * \return a pointer to the SAP user of the RLC
    */
    NrPdcpSapUser* GetNrPdcpSapUser (void);

    void Start ();
    void Stop ();

    void SendData (Time at, std::string dataToSend);
    std::string GetDataReceived (void);

    // Stats
    uint32_t GetTxPdus (void);
    uint32_t GetTxBytes (void);
    uint32_t GetRxPdus (void);
    uint32_t GetRxBytes (void);

    Time GetTxLastTime (void);
    Time GetRxLastTime (void);

    void SetArrivalTime (Time arrivalTime);
    void SetPduSize (uint32_t pduSize);

  void SetDevice (Ptr<NetDevice> device);

  private:
    // Interface forwarded by NrPdcpSapUser
    virtual void DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params);

    NrPdcpSapUser* m_pdcpSapUser;
    NrPdcpSapProvider* m_pdcpSapProvider;

    std::string m_receivedData;

    uint32_t m_txPdus;
    uint32_t m_txBytes;
    uint32_t m_rxPdus;
    uint32_t m_rxBytes;
    Time     m_txLastTime;
    Time     m_rxLastTime;

    EventId m_nextPdu;
    Time m_arrivalTime;
    uint32_t m_pduSize;

    Ptr<NetDevice> m_device;
};

/////////////////////////////////////////////////////////////////////

/**
 * This class implements a testing PDCP entity
 */
class NrTestPdcp : public Object
{
  friend class NrRlcSpecificNrRlcSapUser<NrTestPdcp>;
  
  public:
    static TypeId GetTypeId (void);

    NrTestPdcp (void);
    virtual ~NrTestPdcp (void);
    virtual void DoDispose (void);


    /**
    * \brief Set the RLC SAP provider
    * \param s a pointer to the RLC SAP provider
    */
    void SetNrRlcSapProvider (NrRlcSapProvider* s);
    /**
    * \brief Get the RLC SAP user
    * \return a pointer to the SAP user of the RLC
    */
    NrRlcSapUser* GetNrRlcSapUser (void);

    void Start ();

    void SendData (Time time, std::string dataToSend);
    std::string GetDataReceived (void);

  private:
    // Interface forwarded by NrRlcSapUser
  virtual void DoReceivePdcpPdu (Ptr<Packet> p);

    NrRlcSapUser* m_rlcSapUser;
    NrRlcSapProvider* m_rlcSapProvider;

    std::string m_receivedData;
};

/////////////////////////////////////////////////////////////////////

/**
 * This class implements a testing loopback MAC layer
 */
class NrTestMac : public Object
{
//   friend class EnbMacMemberNrEnbCmacSapProvider;
    friend class EnbMacMemberNrMacSapProvider<NrTestMac>;
//   friend class EnbMacMemberFfMacSchedSapUser;
//   friend class EnbMacMemberFfMacCschedSapUser;
//   friend class EnbMacMemberNrEnbPhySapUser;

  public:
    static TypeId GetTypeId (void);

    NrTestMac (void);
    virtual ~NrTestMac (void);
    virtual void DoDispose (void);

    void SetDevice (Ptr<NetDevice> device);

    void SendTxOpportunity (Time, uint32_t);
    std::string GetDataReceived (void);

    bool Receive (Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol, const Address& addr);

    /**
     * \brief Set the MAC SAP user
     * \param s a pointer to the MAC SAP user
     */
    void SetNrMacSapUser (NrMacSapUser* s);
    /**
     * \brief Get the MAC SAP provider
     * \return a pointer to the SAP provider of the MAC
     */
    NrMacSapProvider* GetNrMacSapProvider (void);

    /**
     * \brief Set the other side of the MAC Loopback
     * \param s a pointer to the other side of the MAC loopback
     */
    void SetNrMacLoopback (Ptr<NrTestMac> s);

    /**
     *
     */
    void SetPdcpHeaderPresent (bool present);

    /**
     *
     */
    void SetRlcHeaderType (uint8_t rlcHeaderType);

    typedef enum {
      UM_RLC_HEADER = 0,
      AM_RLC_HEADER = 1,
    } RlcHeaderType_t;

    /**
     *
     */
    void SetTxOpportunityMode (uint8_t mode);

    typedef enum {
      MANUAL_MODE     = 0,
      AUTOMATIC_MODE  = 1,
      RANDOM_MODE     = 2
    } TxOpportunityMode_t;

    void SetTxOppTime (Time txOppTime);
    void SetTxOppSize (uint32_t txOppSize);

    // Stats
    uint32_t GetTxPdus (void);
    uint32_t GetTxBytes (void);
    uint32_t GetRxPdus (void);
    uint32_t GetRxBytes (void);

  private:
    // forwarded from NrMacSapProvider
    void DoTransmitPdu (NrMacSapProvider::TransmitPduParameters);
    void DoReportBufferStatus (NrMacSapProvider::ReportBufferStatusParameters);

    NrMacSapProvider* m_macSapProvider;
    NrMacSapUser* m_macSapUser;
    Ptr<NrTestMac> m_macLoopback;

    std::string m_receivedData;

    uint8_t m_rlcHeaderType;
    bool m_pdcpHeaderPresent;
    uint8_t m_txOpportunityMode;

    Ptr<NetDevice> m_device;

    // TxOpportunity configuration
    EventId m_nextTxOpp;
    Time m_txOppTime;
    uint32_t m_txOppSize;
    std::list<EventId> m_nextTxOppList;

    // Stats
    uint32_t m_txPdus;
    uint32_t m_txBytes;
    uint32_t m_rxPdus;
    uint32_t m_rxBytes;

};



/**
 * RRC stub providing a testing N2 SAP user to be used with the NgcEnbApplication
 * 
 */
class NgcTestRrc : public Object
{
  friend class MemberNgcEnbN2SapUser<NgcTestRrc>;

public:
  NgcTestRrc ();
  virtual ~NgcTestRrc ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  /** 
   * Set the N2 SAP Provider
   * 
   * \param s the N2 SAP Provider
   */
  void SetN2SapProvider (NgcEnbN2SapProvider* s);

  /** 
   * 
   * \return the N2 SAP user
   */
  NgcEnbN2SapUser* GetN2SapUser ();

private:

  // N2 SAP methods
  void DoDataRadioBearerSetupRequest (NgcEnbN2SapUser::DataRadioBearerSetupRequestParameters params);
  void DoPathSwitchRequestAcknowledge (NgcEnbN2SapUser::PathSwitchRequestAcknowledgeParameters params);  
  
  NgcEnbN2SapProvider* m_n2SapProvider;
  NgcEnbN2SapUser* m_n2SapUser;
  

};


} // namespace ns3

#endif /* NR_TEST_MAC_H */
