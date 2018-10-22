/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef LTE_RLC_H
#define LTE_RLC_H

#include <ns3/simple-ref-count.h>
#include <ns3/packet.h>
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include <ns3/ngc-x2-sap.h>

#include "ns3/object.h"

#include "ns3/lte-rlc-sap.h"
#include "ns3/lte-mac-sap.h"
#include <fstream>//sjkang

namespace ns3 {


// class LteRlcSapProvider;
// class LteRlcSapUser;
// 
// class LteMacSapProvider;
// class LteMacSapUser;

class LteRlc;

class LteRlcSpecificLteMacSapUser : public LteMacSapUser
{

public:
  LteRlcSpecificLteMacSapUser (LteRlc* rlc);

  // Interface implemented from LteMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId);
  virtual void NotifyHarqDeliveryFailure ();
  virtual void NotifyHarqDeliveryFailure (uint8_t harqId);
  virtual void ReceivePdu (Ptr<Packet> p);

private:
  LteRlcSpecificLteMacSapUser ();
  LteRlc* m_rlc;
};


/**
 * This abstract base class defines the API to interact with the Radio Link Control
 * (LTE_RLC) in LTE, see 3GPP TS 36.322
 *
 */
class LteRlc : public Object 
{
  friend class LteRlcSpecificLteMacSapUser;
  friend class NgcX2RlcSpecificUser<LteRlc>;
  friend class LteRlcSpecificLteRlcSapProvider<LteRlc>;
public:
  LteRlc ();
  virtual ~LteRlc ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   *
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);
  void SetDrbId(uint8_t drbId); //sjkang1115
  void SetStreamForQueueStatistics(std::ofstream * stream); //sjkang1116

  /**
   *
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the RLC SAP user to be used by this LTE_RLC
   */
  void SetLteRlcSapUser (LteRlcSapUser * s);
  void SetLteRlcAssistantSapUser(LteRlcSapUser *s); //sjkang0104

  /**
   *
   *
   * \return the RLC SAP Provider interface offered to the PDCP by this LTE_RLC
   */
  LteRlcSapProvider* GetLteRlcSapProvider ();

  /**
   * Set the param needed for X2 tunneling
   * \param the UeDataParams defined in RRC
   */
  void SetUeDataParams(NgcX2Sap::UeDataParams params);

  /**
   * \param s the NgcX2Rlc Provider to the Ngc X2 interface
   */
  void SetNgcX2RlcProvider (NgcX2RlcProvider * s);

  /**
   * \return the NgcX2Rlc User, given to X2 to access Rlc SendMcPdcpPdu method
   */
  NgcX2RlcUser* GetNgcX2RlcUser ();

  /**
   *
   *
   * \param s the MAC SAP Provider to be used by this LTE_RLC
   */
  void SetLteMacSapProvider (LteMacSapProvider * s);

  /**
   *
   *
   * \return the MAC SAP User interface offered to the MAC by this LTE_RLC
   */
  LteMacSapUser* GetLteMacSapUser ();


  /**
   * TracedCallback signature for NotifyTxOpportunity events.
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The number of bytes to transmit
   */
  typedef void (* NotifyTxTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes);

  /**
   * TracedCallback signature for
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The packet size.
   * \param [in] delay Delay since sender timestamp, in ns.
   */
  typedef void (* ReceiveTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);

  /**
   * TracedCallback signature for
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The packet size.
   * \param [in] the number of RLC AM retransmissions for that packet
   */
  typedef void (* RetransmissionCountCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint32_t numRetx);

  /// \todo MRE What is the sense to duplicate all the interfaces here???
  // NB to avoid the use of multiple inheritance
  virtual void CalculatePathThroughput(std::ofstream *stream)=0; //sjkang

protected:
  // Interface forwarded by LteRlcSapProvider
  virtual void DoTransmitPdcpPdu (Ptr<Packet> p) = 0;

  LteRlcSapUser* m_rlcSapUser;
  LteRlcSapUser* m_rlc_Assistant_User; //sjkang0104
  LteRlcSapProvider* m_rlcSapProvider;

  // Interface forwarded by LteMacSapUser
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId) = 0;
  virtual void DoNotifyHarqDeliveryFailure () = 0;
  virtual void DoNotifyHarqDeliveryFailure (uint8_t harqId);
  virtual void DoReceivePdu (Ptr<Packet> p) = 0;

  virtual void DoSendMcPdcpSdu(NgcX2Sap::UeDataParams params) = 0;

  virtual void DoRequestAssistantInfo();//sjkang

  LteMacSapUser* m_macSapUser;
  LteMacSapProvider* m_macSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;
  uint8_t m_drbId;//sjkang1115
  std::ofstream *measuringQusizeQueueDelayStream; //sjkang1116
 // uint16_t m_rnti; //sjkang1115
  /**
   * Used to inform of a PDU delivery to the MAC SAP provider
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the MAC SAP user
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

  TracedCallback<uint16_t, uint8_t, uint32_t, uint32_t> m_txCompletedCallback; // callback used to broadcast the number of retx for each RLC packet

  // MC functionalities
  // UeDataParams needed to forward data to MmWave
  NgcX2Sap::UeDataParams m_ueDataParams;
  bool isMc;
  NgcX2RlcProvider* m_ngcX2RlcProvider;
  NgcX2RlcUser* m_ngcX2RlcUser;

};



/**
 * LTE_RLC Saturation Mode (SM): simulation-specific mode used for
 * experiments that do not need to consider the layers above the LTE_RLC.
 * The LTE_RLC SM, unlike the standard LTE_RLC modes, it does not provide
 * data delivery services to upper layers; rather, it just generates a
 * new LTE_RLC PDU whenever the MAC notifies a transmission opportunity.
 *
 */
class LteRlcSm : public LteRlc
{
public:
  LteRlcSm ();
  virtual ~LteRlcSm ();
  static TypeId GetTypeId (void);
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void DoTransmitPdcpPdu (Ptr<Packet> p);
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId);
  virtual void DoNotifyHarqDeliveryFailure ();
  virtual void DoReceivePdu (Ptr<Packet> p);
  virtual void DoSendMcPdcpSdu (NgcX2Sap::UeDataParams params);
  virtual void CalculatePathThroughput(std::ofstream *stream); //sjkang
  virtual void DoRequestAssistantInfo();//sjkang


private:
  void ReportBufferStatus ();

};




// /**
//  * Implements LTE_RLC Transparent Mode (TM), see  3GPP TS 36.322
//  *
//  */
// class LteRlcTm : public LteRlc
// {
// public:
//   virtual ~LteRlcTm ();

// };


// /**
//  * Implements LTE_RLC Unacknowledged Mode (UM), see  3GPP TS 36.322
//  *
//  */
// class LteRlcUm : public LteRlc
// {
// public:
//   virtual ~LteRlcUm ();

// };

// /**
//  * Implements LTE_RLC Acknowledged Mode (AM), see  3GPP TS 36.322
//  *
//  */

// class LteRlcAm : public LteRlc
// {
// public:
//   virtual ~LteRlcAm ();
// };





} // namespace ns3

#endif // LTE_RLC_H