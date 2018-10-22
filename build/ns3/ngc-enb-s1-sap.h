/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#ifndef NGC_ENB_S1_SAP_H
#define NGC_ENB_S1_SAP_H

#include <list>
#include <stdint.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address.h>

using namespace std;

namespace ns3 {


/**
 * This class implements the Service Access Point (SAP) between the
 * LteEnbRrc and the NgcEnbApplication. In particular, this class implements the
 * Provider part of the SAP, i.e., the methods exported by the
 * NgcEnbApplication and called by the LteEnbRrc.
 * 
 */
class NgcEnbS1SapProvider
{
public:
  virtual ~NgcEnbS1SapProvider ();  

  /** 
   * 
   * 
   * \param imsi 
   * \param rnti 
   */
  virtual void InitialUeMessage (uint64_t imsi, uint16_t rnti) = 0;
  virtual void InitialUeMessage (uint64_t imsi, uint16_t rnti, int dummy) = 0; // jhlim for S1apSapMme

  /**
   *  \brief Triggers ngc-enb-application to send ERAB Release Indication message towards MME
   *  \param imsi the UE IMSI
   *  \param rnti the UE RNTI
   *  \param bearerId Bearer Identity which is to be de-activated
   */
  virtual void DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId) = 0;
  

  struct BearerToBeSwitched
  {
    uint8_t epsBearerId;
    uint32_t teid;
  };
  
  struct PathSwitchRequestParameters
  {
    uint16_t rnti;
    uint16_t cellId;
    uint32_t mmeUeS1Id;
    std::list<BearerToBeSwitched> bearersToBeSwitched;
  };

  virtual void PathSwitchRequest (PathSwitchRequestParameters params) = 0;

  /** 
   * release UE context at the S1 Application of the source eNB after
   * reception of the UE CONTEXT RELEASE X2 message from the target eNB
   * during X2-based handover 
   * 
   * \param rnti 
   */
  virtual void UeContextRelease (uint16_t rnti) = 0;
    
};
  


/**
 * This class implements the Service Access Point (SAP) between the
 * LteEnbRrc and the NgcEnbApplication. In particular, this class implements the
 * User part of the SAP, i.e., the methods exported by the LteEnbRrc
 * and called by the NgcEnbApplication.
 * 
 */
class NgcEnbS1SapUser
{
public:
  virtual ~NgcEnbS1SapUser ();
  
  /**
   * Parameters passed to DataRadioBearerSetupRequest ()
   * 
   */
  struct DataRadioBearerSetupRequestParameters
  {
    uint16_t rnti;   /**< the RNTI identifying the UE for which the
			DataRadioBearer is to be created */ 
    EpsBearer bearer; /**< the characteristics of the bearer to be set
                         up */
    uint8_t bearerId; /**< the EPS Bearer Identifier */
    uint32_t    gtpTeid; /**< S1-bearer GTP tunnel endpoint identifier, see 36.423 9.2.1 */
    Ipv4Address transportLayerAddress; /**< IP Address of the SMF, see 36.423 9.2.1 */
  };

  /**
   * request the setup of a DataRadioBearer
   * 
   */
  virtual void DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params) = 0;

  
  struct PathSwitchRequestAcknowledgeParameters
  {
    uint16_t rnti;
  };

  virtual void PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params) = 0;
  
};
  



/**
 * Template for the implementation of the NgcEnbS1SapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcEnbS1SapProvider : public NgcEnbS1SapProvider
{
public:
  MemberNgcEnbS1SapProvider (C* owner);

  // inherited from NgcEnbS1SapProvider
  virtual void InitialUeMessage (uint64_t imsi, uint16_t rnti);
  virtual void InitialUeMessage (uint64_t imsi, uint16_t rnti, int dummy);
  virtual void DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId);

  virtual void PathSwitchRequest (PathSwitchRequestParameters params);
  virtual void UeContextRelease (uint16_t rnti);

private:
  MemberNgcEnbS1SapProvider ();
  C* m_owner;
};

template <class C>
MemberNgcEnbS1SapProvider<C>::MemberNgcEnbS1SapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcEnbS1SapProvider<C>::MemberNgcEnbS1SapProvider ()
{
}


template <class C>
void MemberNgcEnbS1SapProvider<C>::InitialUeMessage (uint64_t imsi, uint16_t rnti)
{
  cout<<"DoInitialUeMessage for Provider is called" << endl;
  //m_owner->DoInitialUeMessage (imsi, rnti, 0); // jhlim
  m_owner->DoInitialUeMessage (imsi, rnti);
}

template <class C>
void MemberNgcEnbS1SapProvider<C>::DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  //m_owner->DoReleaseIndication (imsi, rnti, bearerId, 0); // jhlim
	m_owner->DoReleaseIndication (imsi, rnti, bearerId); // jhlim
}

template <class C>
void MemberNgcEnbS1SapProvider<C>::PathSwitchRequest (PathSwitchRequestParameters params)
{
  //m_owner->DoPathSwitchRequest (params, 0); // jhlim
  m_owner->DoPathSwitchRequest (params); // jhlim
}

/* jhlim */
template <class C>
void MemberNgcEnbS1SapProvider<C>::InitialUeMessage (uint64_t imsi, uint16_t rnti, int dummy)
{
  cout<<"DoInitialUeMessage for Mme is called" << endl;
  m_owner->DoInitialUeMessage (imsi, rnti, 0);
}
/*
template <class C>
void MemberNgcEnbS1SapProvider<C>::DoSendReleaseIndication1 (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  cout<<"DoReleaseIndication is called" << endl;
  m_owner->DoReleaseIndication (imsi, rnti, bearerId, 0);
}
template <class C>
void MemberNgcEnbS1SapProvider<C>::PathSwitchRequest1 (PathSwitchRequestParameters params)
{
  cout<<"DoPathSwitchRequest is called" << endl;
  m_owner->DoPathSwitchRequest (params, 0);
}
*/

template <class C>
void MemberNgcEnbS1SapProvider<C>::UeContextRelease (uint16_t rnti)
{
  m_owner->DoUeContextRelease (rnti);
}

/**
 * Template for the implementation of the NgcEnbS1SapUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcEnbS1SapUser : public NgcEnbS1SapUser
{
public:
  MemberNgcEnbS1SapUser (C* owner);

  // inherited from NgcEnbS1SapUser
  virtual void DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params);
  virtual void PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params);

private:
  MemberNgcEnbS1SapUser ();
  C* m_owner;
};

template <class C>
MemberNgcEnbS1SapUser<C>::MemberNgcEnbS1SapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcEnbS1SapUser<C>::MemberNgcEnbS1SapUser ()
{
}

template <class C>
void MemberNgcEnbS1SapUser<C>::DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params)
{
  m_owner->DoDataRadioBearerSetupRequest (params);
}

template <class C>
void MemberNgcEnbS1SapUser<C>::PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params)
{
  m_owner->DoPathSwitchRequestAcknowledge (params);
}

} // namespace ns3

#endif // NGC_ENB_S1_SAP_H
