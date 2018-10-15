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

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include <ns3/epc-helper.h>
#include <ns3/simulator.h>
#include "lte-enb-net-device.h"
#include "epc-ue-nas.h"
#include "lte-as-sap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcUeNas");



/// Map each of UE NAS states to its string representation.
static const std::string g_ueNasStateName[EpcUeNas::NUM_STATES] =
{
  "OFF",
  "ATTACHING",
  "IDLE_REGISTERED",
  "CONNECTING_TO_EPC",
  "ACTIVE"
};

/**
 * \param s The UE NAS state.
 * \return The string representation of the given state.
 */
static inline const std::string & ToString (EpcUeNas::State s)
{
  return g_ueNasStateName[s];
}




NS_OBJECT_ENSURE_REGISTERED (EpcUeNas);

EpcUeNas::EpcUeNas ()
  : m_state (OFF),
    m_csgId (0),
    m_asSapProvider (0),
    m_bidCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_asSapUser = new MemberLteAsSapUser<EpcUeNas> (this);
  m_mmWaveAsSapProvider = 0;
  m_mmWaveAsSapProvider_2 = 0;
}


EpcUeNas::~EpcUeNas ()
{
  NS_LOG_FUNCTION (this);
}

void
EpcUeNas::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_asSapUser;
}

TypeId
EpcUeNas::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcUeNas")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<EpcUeNas> ()
    .AddTraceSource ("StateTransition",
                     "fired upon every UE NAS state transition",
                     MakeTraceSourceAccessor (&EpcUeNas::m_stateTransitionCallback),
                     "ns3::EpcUeNas::StateTracedCallback")
  ;
  return tid;
}

void 
EpcUeNas::SetDevice (Ptr<NetDevice> dev)
{
  NS_LOG_FUNCTION (this << dev);
  m_device = dev;
}

void 
EpcUeNas::SetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

void
EpcUeNas::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  m_asSapProvider->SetCsgWhiteList (csgId);
}

uint32_t
EpcUeNas::GetCsgId () const
{
  NS_LOG_FUNCTION (this);
  return m_csgId;
}

void
EpcUeNas::SetAsSapProvider (LteAsSapProvider* s)
{
  NS_LOG_FUNCTION (this << s);
  m_asSapProvider = s;
}

LteAsSapUser*
EpcUeNas::GetAsSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_asSapUser;
}

void
EpcUeNas::SetMmWaveAsSapProvider (LteAsSapProvider* s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveAsSapProvider = s;
}
void
EpcUeNas::SetMmWaveAsSapProvider_2 (LteAsSapProvider* s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveAsSapProvider_2 = s;
}

void
EpcUeNas::SetForwardUpCallback (Callback <void, Ptr<Packet> > cb)
{
  NS_LOG_FUNCTION (this);
  m_forwardUpCallback = cb;
}

void
EpcUeNas::StartCellSelection (uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << dlEarfcn);
  m_asSapProvider->StartCellSelection (dlEarfcn);
}

void 
EpcUeNas::Connect ()
{
  NS_LOG_FUNCTION (this);

  // tell RRC to go into connected mode
  m_asSapProvider->Connect ();
}

void
EpcUeNas::Connect (uint16_t cellId, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << cellId << dlEarfcn);
 m_cellId = cellId;
 m_dlEarfcn = dlEarfcn;
  // force the UE RRC to be camped on a specific eNB
  m_asSapProvider->ForceCampedOnEnb (cellId, dlEarfcn);

  // tell RRC to go into connected mode
  m_asSapProvider->Connect ();
}

void
EpcUeNas::ConnectMc (uint16_t cellId, uint16_t dlEarfcn, uint16_t mmWaveCellId)
{
  NS_LOG_FUNCTION (this << cellId << dlEarfcn);

  // force the UE RRC to be camped on a specific eNB
  m_asSapProvider->ForceCampedOnEnb (cellId, dlEarfcn);

  // tell RRC to go into connected mode
  m_asSapProvider->Connect ();

  m_mmWaveCellId = mmWaveCellId;
  m_dlEarfcn = dlEarfcn;
}


void 
EpcUeNas::Disconnect ()
{
  NS_LOG_FUNCTION (this);
  m_asSapProvider->Disconnect ();
  SwitchToState (OFF);
}


void 
EpcUeNas::ActivateEpsBearer (EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case ACTIVE:
      NS_FATAL_ERROR ("the necessary NAS signaling to activate a bearer after the initial context has already been setup is not implemented");
      break;

    default:
      BearerToBeActivated btba;
      btba.bearer = bearer;
      btba.tft = tft;
      m_bearersToBeActivatedList.push_back (btba);
      break;
    }
}

bool
EpcUeNas::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  switch (m_state)
    {
    case SECOND_ACTIVE:
      {
        uint32_t id = m_tftClassifier.Classify (packet, EpcTft::UPLINK);
        NS_ASSERT ((id & 0xFFFFFF00) == 0);
        uint8_t bid = (uint8_t) (id & 0x000000FF);
        if (bid == 0)
          {
            return false;
          }
        else
          {
        	// NS_LOG_UNCOND("wil send data in EpcUeNas");

        	m_asSapProvider->SendData (packet, bid);
            return true;
          }
      }
      break;

    default:
      NS_LOG_WARN (this << " NAS OFF, discarding packet");
      return false;
      break;
    }
}

void 
EpcUeNas::DoNotifyConnectionSuccessful (uint16_t rnti)
{
  NS_LOG_FUNCTION (this<<rnti <<m_state);
  switch (m_state)
  {
  case ACTIVE:
  {

   SwitchToState(SECOND_ACTIVE);
   m_mmWaveRnti_73G = rnti;
  // m_asSapProvider->NotifySecondaryCellConnected(rnti, m_mmWaveCellId); ///sjkang

  	  }
  	  break;
  	case SECOND_ACTIVE: // this means the Master LTE Cell was already connected
      {

    	  // notify the LTE eNB RRC that a secondary cell is available
    	  m_mmWaveRnti_28G = rnti;
    	  std::cout << "UE "<< m_imsi << " completed two eNB connection and will notify to LTE eNB RRC " << std::endl;
        m_asSapProvider->NotifySecondaryCellConnected(m_mmWaveRnti_73G,m_mmWaveRnti_28G, m_mmWaveCellId_2, m_mmWaveCellId); ///sjkang
      }
      break;

   default:
   {
	    SwitchToState (ACTIVE); // will eventually activate dedicated bearers
      break;
   }
  }
}

void
EpcUeNas::DoNotifyHandoverSuccessful (uint16_t rnti, uint16_t mmWaveCellId)
{
  m_mmWaveCellId = mmWaveCellId;
  NS_LOG_FUNCTION (this);
  switch (m_state)
  {
    case ACTIVE: // this means the Master LTE Cell was already connected 
      {
        // notify the LTE eNB RRC that a secondary cell is available
        m_asSapProvider->NotifySecondaryCellConnected(rnti, m_mmWaveCellId);
      }
      break;

    default:
      SwitchToState (ACTIVE); // will eventually activate dedicated bearers
      break;
  } 
}

void
EpcUeNas::DoNotifyConnectToMmWave(uint16_t mmWaveCellId, uint16_t mmWaveCellId_2)
{
  NS_LOG_FUNCTION(mmWaveCellId <<"\t"<< m_state);
 // std::cout << mmWaveCellId << "\t"<<mmWaveCellId_2 << std::endl;
 if (mmWaveCellId && mmWaveCellId_2)
 {
	 m_mmWaveCellId_2 = mmWaveCellId;
	 m_mmWaveCellId = mmWaveCellId_2;
 }
 if(m_mmWaveAsSapProvider !=0 && m_state==SECOND_ACTIVE) {

	 // m_mmWaveCellId = mmWaveCellId;
    NS_ASSERT_MSG(m_mmWaveCellId > 0, "Invalid CellId");
    NS_LOG_INFO("Connect to cell " << m_mmWaveCellId);
    // force the UE RRC to be camped on a specific eNB

    m_mmWaveAsSapProvider->ForceCampedOnEnb (m_mmWaveCellId, m_dlEarfcn); // TODO probably the second argument is useless
    cellIdMmWavePhy[m_mmWaveCellId]->isAddtionalMmWavPhy =false; //sjkang
    std::cout<<"UE "<<m_imsi << " completes first mmwave eNB connection and  will start to connect to second mmWave eNB "<<m_mmWaveCellId << std::endl;

    // tell RRC to go into connected mode
    m_mmWaveAsSapProvider->Connect ();
  }
  else if (m_mmWaveAsSapProvider_2 != 0 && m_state==ACTIVE)
  {
	//  m_mmWaveCellId_2 = mmWaveCellId;

	  NS_ASSERT_MSG(m_mmWaveCellId_2 > 0, "Invalid CellId");
	    NS_LOG_INFO("Connect to cell " << m_mmWaveCellId);
	    // force the UE RRC to be camped on a specific eNB
	   cellIdMmWavePhy[m_mmWaveCellId_2]->isAddtionalMmWavPhy =true; //sjkang
	    m_mmWaveAsSapProvider_2->ForceCampedOnEnb (m_mmWaveCellId_2, m_dlEarfcn); // TODO probably the second argument is useless
	    std::cout<<"UE "<<m_imsi << " complete LTE connection and  will start to connect to first mmWave eNB " << m_mmWaveCellId_2 << std::endl;

	    // tell RRC to go into connected mode
	    m_mmWaveAsSapProvider_2->Connect ();
  }
  else{
    NS_LOG_WARN("Trying to connect to a secondary cell a non MC capable device");
  }
  
}

void
EpcUeNas::DoNotifySecondaryCellHandoverStarted (uint16_t oldRnti, uint16_t newRnti, uint16_t mmWaveCellId, LteRrcSap::RadioResourceConfigDedicated rrcd)
{
  m_mmWaveCellId = mmWaveCellId;
  NS_ASSERT(m_asSapProvider != 0); 
  
  NS_ASSERT_MSG(mmWaveCellId > 0, "Invalid CellId");

  NS_LOG_INFO("Notify the LTE RRC of the secondary cell HO to " << mmWaveCellId);
  // Notify the LTE RRC of the secondary cell HO
  m_asSapProvider->NotifySecondaryCellHandover(oldRnti, newRnti, mmWaveCellId, rrcd);
   
}

void
EpcUeNas::DoNotifyConnectionFailed ()
{
  NS_LOG_FUNCTION (this);
std::cout<<"imsi " << m_imsi << " --> Connection failed----..  " <<std::endl;
std::cout<< m_imsi << "--> will try to connect to Enb-->"<<m_cellId << " with " <<m_dlEarfcn << std::endl;
  // immediately retry the connection
  //Simulator::ScheduleNow (&LteAsSapProvider::Connect, m_asSapProvider);
//Simulator::Schedule (MilliSeconds(0.0),&EpcUeNas::Connect,this, m_cellId, m_dlEarfcn);
Connect(m_cellId, m_dlEarfcn);
}

void
EpcUeNas::DoRecvData (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_forwardUpCallback (packet);
}

void 
EpcUeNas::DoNotifyConnectionReleased ()
{
  NS_LOG_FUNCTION (this);
  SwitchToState (OFF);
}

void 
EpcUeNas::DoActivateEpsBearer (EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_bidCounter < 11, "cannot have more than 11 EPS bearers");
  uint8_t bid = ++m_bidCounter;
  m_tftClassifier.Add (tft, bid);
}

EpcUeNas::State
EpcUeNas::GetState () const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}

void 
EpcUeNas::SwitchToState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  State oldState = m_state;
  m_state = newState;
  NS_LOG_INFO ("IMSI " << m_imsi << " NAS " << ToString (oldState) << " --> " << ToString (newState));
  m_stateTransitionCallback (oldState, newState);

  // actions to be done when entering a new state:
  switch (m_state)
    {
    case ACTIVE:
      for (std::list<BearerToBeActivated>::iterator it = m_bearersToBeActivatedList.begin ();
           it != m_bearersToBeActivatedList.end ();
           m_bearersToBeActivatedList.erase (it++))
        {
          DoActivateEpsBearer (it->bearer, it->tft);
        }
      break;
    case SECOND_ACTIVE:
          /*for (std::list<BearerToBeActivated>::iterator it = m_bearersToBeActivatedList.begin ();
               it != m_bearersToBeActivatedList.end ();
               m_bearersToBeActivatedList.erase (it++))
            {
              DoActivateEpsBearer (it->bearer, it->tft);
            }*/
          break;
    default:
      break;
    }

}
void
EpcUeNas::DoRecvRrcConnectionReconfigurationMessageforHandover(LteRrcSap::RrcConnectionReconfiguration msg){
	if (msg.HandoverCase ==2 ){
		msg.HandoverCase =0;
	m_mmWaveAsSapProvider_2->SendRrcConnectionReconfigurationMessageToMmWaveRrc(msg); //sjkang0417

	}
	else if (msg.HandoverCase ==1){
		msg.HandoverCase =0;
		m_mmWaveAsSapProvider->SendRrcConnectionReconfigurationMessageToMmWaveRrc(msg); //sjkang0417


	}
std::cout << "EpcUeNas receive rrc reconfiguration message " <<std::endl;
}

} // namespace ns3

