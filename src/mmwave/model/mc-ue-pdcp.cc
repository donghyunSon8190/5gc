/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 * Extension to DC devices by Michele Polese <michele.polese@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/mw-mc-ue-pdcp.h"
#include "ns3/lte-pdcp-header.h"
#include "ns3/lte-pdcp-sap.h"
#include "ns3/lte-pdcp-tag.h"
#include "ns3/seq-ts-header.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <fstream>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("McUePdcp");

class McUePdcpSpecificLteRlcSapUser : public LteRlcSapUser
{
public:
  McUePdcpSpecificLteRlcSapUser (McUePdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from LteRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);

private:
  McUePdcpSpecificLteRlcSapUser ();
  McUePdcp* m_pdcp;
};

McUePdcpSpecificLteRlcSapUser::McUePdcpSpecificLteRlcSapUser (McUePdcp* pdcp)
  : m_pdcp (pdcp)
{
}

McUePdcpSpecificLteRlcSapUser::McUePdcpSpecificLteRlcSapUser ()
{
}

void
McUePdcpSpecificLteRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}

///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (McUePdcp);

McUePdcp::McUePdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0),
    m_useMmWaveConnection (false)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new LtePdcpSpecificLtePdcpSapProvider<McUePdcp> (this);
  m_rlcSapUser = new McUePdcpSpecificLteRlcSapUser (this);
  Last_Submitted_PDCP_RX_SN = -1;

   // sjkang
   receivedPDCP_SN =0;
   Next_PDCP_RX_SN =0;
   Reordering_PDCP_RX_COUNT =0;
 //  k=0;
   check =false;
   //m_isEnbPdcp =false; // woody
   //enable1X=false;
   present_RX_HFN =0;
   tempTime = Simulator :: Now();
  previousTime = Simulator :: Now();
  TotalTime=0;
  // AverageSnDifference =1;
   TotalPacketSize_ordered=0;
   TotalPacketSize=0;
   discardedPacketSize =0;
   cellIdToSN_1=0; cellIdToSN_2=0 ; //sjkang1116
   cellId_1 =0 ; cellId_2 =0 ; //sjkang1116
   m_isEnableReordering =false; //sjkang1116
   firstPacket = false;
}

McUePdcp::~McUePdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
McUePdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::McUePdcp")
    .SetParent<Object> ()
    .SetGroupName("Lte")
	.AddAttribute("ExpiredTime", "PDCP reordering time",
			 TimeValue(MilliSeconds(100)),
			 MakeTimeAccessor (&McUePdcp::expiredTime),
             MakeTimeChecker ())
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&McUePdcp::m_txPdu),
                     "ns3::McUePdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&McUePdcp::m_rxPdu),
                     "ns3::McUePdcp::PduRxTracedCallback")
    .AddAttribute ("LteUplink",
                    "Use LTE for uplink",
                    BooleanValue (false),
                    MakeBooleanAccessor (&McUePdcp::m_alwaysLteUplink),
                    MakeBooleanChecker ())
	.AddAttribute("EnableReordering", "Pdcp reordering function is eanble or not ",
					BooleanValue(false),
					MakeBooleanAccessor(&McUePdcp::m_isEnableReordering),
					MakeBooleanChecker())
    ;
  return tid;
}

void
McUePdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
  t_ReorderingTimer.Cancel();
}


void
McUePdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
McUePdcp::SetMmWaveRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_mmWaveRnti = rnti;
}

void
McUePdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
McUePdcp::SetLtePdcpSapUser (LtePdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

LtePdcpSapProvider*
McUePdcp::GetLtePdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
McUePdcp::SetLteRlcSapProvider (LteRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rlcSapProvider = s;
}

void
McUePdcp::SetMmWaveRlcSapProvider (LteRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveRlcSapProvider = s;
}
void
McUePdcp::SetMmWaveRlcSapProvider_2 (LteRlcSapProvider * s) //sjkang1110
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveRlcSapProvider_2 = s;
}
LteRlcSapUser*
McUePdcp::GetLteRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}

McUePdcp::Status 
McUePdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void 
McUePdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}
void
McUePdcp::SetStreams(std::ofstream * stream){
	m_SN_DifferenceStream = stream; //sjkang1116
}
////////////////////////////////////////

void
McUePdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());
  //std::cout << " UE will transmit uplink data to enb " << p->GetSize() << std::endl;
  LtePdcpHeader pdcpHeader;
  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);

  m_txSequenceNumber++;
  if (m_txSequenceNumber > m_maxPdcpSn)
    {
      m_txSequenceNumber = 0;
    }

  pdcpHeader.SetDcBit (LtePdcpHeader::DATA_PDU);

  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
  p->AddHeader (pdcpHeader);

  // Sender timestamp
  PdcpTag pdcpTag (Simulator::Now ());
  p->AddByteTag (pdcpTag);
  //m_txPdu (m_rnti, m_lcid, p->GetSize ());

  LteRlcSapProvider::TransmitPdcpPduParameters params;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.pdcpPdu = p;

  // WARN TODO hack: always use LTE for uplink (i.e. TCP acks)
 //m_useMmWaveConnection=true;  // will transmit uplink data to LTE cell
m_useMmWaveConnection = false; m_alwaysLteUplink = true;
  if(m_mmWaveRlcSapProvider == 0 || (!m_useMmWaveConnection) || m_alwaysLteUplink)
  {
    NS_LOG_LOGIC(this << " McUePdcp: Tx packet to uplink primary stack");
    m_rlcSapProvider->TransmitPdcpPdu (params);
  }
  else if (m_useMmWaveConnection)
  {
    NS_LOG_LOGIC(this << " McUePdcp: Tx packet to uplink secondary stack");
    //m_mmWaveRlcSapProvider->TransmitPdcpPdu (params);
    //m_mmWaveRlcSapProvider_2->TransmitPdcpPdu (params);//sjkang1110
  }
  else
  {
    NS_FATAL_ERROR ("Invalid combination");
  }
}
void
McUePdcp::DoTransmitPdcpControlPacket(Ptr<Packet> p, uint16_t cellId){
	 NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());
	  //std::cout << " UE will transmit uplink data to enb " << p->GetSize() << std::endl;
	  LtePdcpHeader pdcpHeader;
	  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);
	  pdcpHeader.SetSourceCellId(cellId);
	  m_txSequenceNumber++;
	  if (m_txSequenceNumber > m_maxPdcpSn)
	    {
	      m_txSequenceNumber = 0;
	    }

	  pdcpHeader.SetDcBit (LtePdcpHeader::CONTROL_PDU);

	  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
	  p->AddHeader (pdcpHeader);

	  // Sender timestamp
	  PdcpTag pdcpTag (Simulator::Now ());
	  p->AddByteTag (pdcpTag);
	  //m_txPdu (m_rnti, m_lcid, p->GetSize ());

	  LteRlcSapProvider::TransmitPdcpPduParameters params;
	  params.rnti = m_rnti;
	  params.lcid = m_lcid;
	  params.pdcpPdu = p;

	  // WARN TODO hack: always use LTE for uplink (i.e. TCP acks)
	 //m_useMmWaveConnection=true;  // will transmit uplink data to LTE cell

	  if(m_mmWaveRlcSapProvider == 0 || (!m_useMmWaveConnection) || m_alwaysLteUplink)
	  {
	    NS_LOG_LOGIC(this << " McUePdcp: Tx packet to uplink primary stack");
	    m_rlcSapProvider->TransmitPdcpPdu (params);
	  }
	  else if (m_useMmWaveConnection)
	  {
	    NS_LOG_LOGIC(this << " McUePdcp: Tx packet to uplink secondary stack");
	    m_mmWaveRlcSapProvider->TransmitPdcpPdu (params);
	   // m_mmWaveRlcSapProvider_2->TransmitPdcpPdu (params);//sjkang1110
	  }
	  else
	  {
	    NS_FATAL_ERROR ("Invalid combination");
	  }
}
void
McUePdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());
 NS_LOG_INFO(this << " McUePdcp received dowlink Pdu");
  // Receiver timestamp
  PdcpTag pdcpTag;
  Time delay;
if(!m_isEnableReordering){
  if (p->FindFirstMatchingByteTag (pdcpTag))
    {
      delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
    }
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  p->RemoveAllByteTags();
  NS_LOG_LOGIC("ALL BYTE TAGS REMOVED. NetAmin and FlowMonitor won't work");

  if(p->GetSize() > 3)
  {
    LtePdcpHeader pdcpHeader;
    p->RemoveHeader (pdcpHeader);
    NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);

    // SeqTsHeader seqTs;
    // p->PeekHeader (seqTs);
    // NS_LOG_UNCOND("Carrying UDP packet " << (uint32_t)seqTs.GetSeq());


    m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
    if (m_rxSequenceNumber > m_maxPdcpSn)
      {
        m_rxSequenceNumber = 0;
      }
//  std::cout <<"received PDCP SN in McUePdcp--->" << m_rxSequenceNumber << std::endl;
    if(p->GetSize() > 20 + 8 + 12)
    {

    LtePdcpSapUser::ReceivePdcpSduParameters params;
	//NS_LOG_UNCOND("McUePdcp received packet from LTE Enb " << p->GetSize()<<" byte");

      params.pdcpSdu = p;
      params.rnti = m_rnti;
      params.lcid = m_lcid;
      m_pdcpSapUser->ReceivePdcpSdu (params);
    }
  }
  else
  {
    NS_LOG_INFO("Packet fragmented, too small!");
  }
}else { //if reordering is true
 BufferingAndReordering(p);
}

}
//uint8_t checkPdcp=0;
void
McUePdcp::BufferingAndReordering(Ptr<Packet> p){ // sjkang
  NS_LOG_FUNCTION (this);
  PdcpTag pdcpTag;
  Time delay;

  // mmwave pdcp implementation
  if (p->FindFirstMatchingByteTag (pdcpTag))
  {
    delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
  }

  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  LtePdcpHeader pdcpHeader;
  p->RemoveHeader (pdcpHeader);
 // std::cout << "UE Pdcp receives packet from ---> " << (unsigned)pdcpHeader.GetSourceCellId() << std::endl;


  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);

  m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
  if (m_rxSequenceNumber > m_maxPdcpSn)
  {
    m_rxSequenceNumber = 0;
  }


  LtePdcpSapUser::ReceivePdcpSduParameters params;
  params.pdcpSdu = p;
  params.rnti = m_rnti;
  params.lcid = m_lcid;

  BufferedPackets PacketInBuffer;
  PacketInBuffer.sequenceNumber = pdcpHeader.GetSequenceNumber();
  PacketInBuffer.params =params;
  receivedPDCP_SN =PacketInBuffer.sequenceNumber;

//  CheckingArrivalOfSN[pdcpHeader.GetSequenceNumber()]= Simulator::Now();
//  PropagationDelaybySN[pdcpHeader.GetSequenceNumber()] = delay;
  //printData("RX_SN", PacketInBuffer.sequenceNumber);

  // for checking whether there is the same PDCP SDU in buffer
  std::map<int, LtePdcpSapUser::ReceivePdcpSduParameters>::iterator iter;
  for (iter =PdcpBuffer.begin(); iter != PdcpBuffer.end() ; ++iter)
  {
    if (iter->first == receivedPDCP_SN ) {
	NS_LOG_INFO(iter->first << "   discard ");
	discardedPacketSize+=params.pdcpSdu->GetSize();
	numberOfDiscaredPackets++;
	return;
    }
  }

  PdcpBuffer[PacketInBuffer.sequenceNumber] = params;

// Logging module
/*    uint16_t nextPDCP_SN = (Last_Submitted_PDCP_RX_SN + 1)%(m_maxPdcpSn+1);
    std::map<uint16_t, LtePdcpSapUser::ReceivePdcpSduParameters>::iterator it;
NS_LOG_UNCOND("Buffer status when receivedPDCP " << PacketInBuffer.sequenceNumber << " nextPDCP " << nextPDCP_SN << " LastPDCP " << Last_Submitted_PDCP_RX_SN << " Next_PDCP_RX_SN " << Next_PDCP_RX_SN << " Reordering_PDCP_RX_COUNT " << Reordering_PDCP_RX_COUNT);
    for (it = PdcpBuffer.begin(); it->first != nextPDCP_SN && it != PdcpBuffer.end(); it++)
    {
NS_LOG_UNCOND(it->first);
    }
*/


  // ETSI TS 136 323 v14.2.0.
  if (((receivedPDCP_SN -Last_Submitted_PDCP_RX_SN) > reorderingWindow)
      || ((((Last_Submitted_PDCP_RX_SN-receivedPDCP_SN)) >=0 && ((Last_Submitted_PDCP_RX_SN-receivedPDCP_SN)< reorderingWindow
    		 ) )) )
  {
	// if(firstOne ==1){
    NS_LOG_INFO(Simulator::Now().GetSeconds() << "last SN " << Last_Submitted_PDCP_RX_SN << "\t"
                "received SN "	<< PacketInBuffer.sequenceNumber<< " discard" << "  Next PDCP SN  " << Next_PDCP_RX_SN
				<<"   Q Size " << PdcpBuffer.size());
    discardedPacketSize+=params.pdcpSdu->GetSize();
    PdcpBuffer.erase(PacketInBuffer.sequenceNumber);
    numberOfDiscaredPackets++;

    return ;
	 //}
  }
  else if ((Next_PDCP_RX_SN-receivedPDCP_SN) > reorderingWindow)
  {

	  Next_PDCP_RX_SN = receivedPDCP_SN +1;
	  present_RX_HFN ++;
	      RX_HFNbySN[receivedPDCP_SN] = present_RX_HFN;

  }
  else if (receivedPDCP_SN - Next_PDCP_RX_SN >= reorderingWindow)
  {

			  RX_HFNbySN[receivedPDCP_SN]= present_RX_HFN -1;

  }


  else if (receivedPDCP_SN >= Next_PDCP_RX_SN){

	  Next_PDCP_RX_SN = receivedPDCP_SN +1;
	        RX_HFNbySN[receivedPDCP_SN] =present_RX_HFN;


	        if (Next_PDCP_RX_SN >= m_maxPdcpSn)
	        {
	          Next_PDCP_RX_SN =0;
	          present_RX_HFN ++;

	        }
  }
  else if (receivedPDCP_SN < Next_PDCP_RX_SN) {
	  RX_HFNbySN[receivedPDCP_SN] = present_RX_HFN;
  }

  ///sjkang1116 below procedure is for measuring SN difference between two different path.
    if(cellId_1 == 0 && cellId_1 != pdcpHeader.GetSourceCellId()){
      cellId_1=pdcpHeader.GetSourceCellId();
     }else if (cellId_2 == 0 && cellId_1 !=0 && cellId_2 != pdcpHeader.GetSourceCellId()){
   	cellId_2 = pdcpHeader.GetSourceCellId();
   	 //sjkang1116
     }else if(cellId_1 == pdcpHeader.GetSourceCellId()){
   	  cellIdToSN_1= m_rxSequenceNumber+RX_HFNbySN[receivedPDCP_SN]*MAX_PDCP_SN;
   	  	  	 // if (cellIdToSN_1 >40000) cellIdToSN_1 -=32768;
     }else if (cellId_2 == pdcpHeader.GetSourceCellId()){
  	   cellIdToSN_2 = m_rxSequenceNumber + RX_HFNbySN[receivedPDCP_SN]*MAX_PDCP_SN;
  	   	   	  //if (cellIdToSN_2 > 40000) cellIdToSN_2 -=32768;
     }
    if (cellId_1 != 0 && cellId_2 != 0 && firstPacket == false){
    	MeasureSN_Difference();
    	sendControlMessage();
    	firstPacket = true;
    }


/*
  if (tempTime == Simulator::Now()){
	  tempBuffer[counter] = pdcpHeader.GetSequenceNumber() + RX_HFNbySN[pdcpHeader.GetSequenceNumber()]*MAX_PDCP_SN;
	  counter++;
  } else if (tempTime != Simulator::Now()){
	  Time pastTime = tempTime;
	tempTime= Simulator::Now();
	  int s = tempBuffer[0],pp=0;
	  while ( s == tempBuffer[pp] && pp != counter){
		  s = tempBuffer[pp]+1;
		  pp++;
	  }
	  if (tempBuffer[pp-1] %MAX_PDCP_SN < 40000  && tempBuffer[pp-1] %MAX_PDCP_SN > -40000 &&
			  tempBuffer[counter-1] %MAX_PDCP_SN <40000 && tempBuffer[counter-1]%MAX_PDCP_SN>-40000
			  && ( tempBuffer[counter-1]-tempBuffer[pp-1])<40000 && ( tempBuffer[counter-1]-tempBuffer[pp-1])>-40000
			  && checkPdcp ==1)
	  {
	//  *stream_1->GetStream() <<tempTime.GetSeconds()<<"\t"<< tempBuffer[pp-1]%MAX_PDCP_SN << "\t"<< tempBuffer[counter-1]%MAX_PDCP_SN
		//	  << "\t" << tempBuffer[counter-1]-tempBuffer[pp-1]<< std::endl;

	//double deltaTime = Simulator::Now().GetSeconds() - pastTime.GetSeconds();
	//	  alpha_t = (1-exp(-1*deltaTime/(averageWindow * 1e-3)));
		//  AverageSnDifference = AverageSnDifference * (1-alpha_t) + alpha_t* std::abs(tempBuffer[counter-1]-tempBuffer[pp-1]);
//	count_forSn++;
//	AverageSnDifference = (double)(AverageSnDifference*(count_forSn-1)+ (uint64_t) abs(tempBuffer[counter-1]-tempBuffer[pp-1]))/count_forSn;

	counter =0;
	  }


            for (uint16_t k =0; k<100 ;k++) tempBuffer[k]=0;
  }*/


  if ((receivedPDCP_SN == Last_Submitted_PDCP_RX_SN +1)||(receivedPDCP_SN==Last_Submitted_PDCP_RX_SN-m_maxPdcpSn) )
        {

			  std::map<int, LtePdcpSapUser::ReceivePdcpSduParameters> ::iterator it;
        	  	  temp = new int[(size_t)PdcpBuffer.size()+1];

        	  	    int i=0 , COUNT=0;
        	  		for (it = PdcpBuffer.begin() ;it !=PdcpBuffer.end(); ++it, ++i){
        	  			COUNT = it->first + RX_HFNbySN[it->first]*MAX_PDCP_SN;
        	  			temp[i]=COUNT;

        	  		}

        	  		std::sort(temp, temp + i);

        	             int sum = temp[0];
        	            uint16_t p=0;


        	             while (sum == temp[p] && p!=PdcpBuffer.size()){
        	          	   sum =temp[p]+1;
        	          	   p++;
        	             }

        	            for (int c = 0 ; c< p; c++){
        	            	if (temp[c]> m_maxPdcpSn) temp[c]= temp[c]% MAX_PDCP_SN;
        	       		 m_pdcpSapUser->ReceivePdcpSdu(PdcpBuffer[temp[c]]);
        	        		// printData("Reordered_SN", temp[c]);
        	        		 SumOfPacketSize +=PdcpBuffer[temp[c]].pdcpSdu->GetSize();//sjkang0718
        	                 orderdedSumOfPacket=SumOfPacketSize;
        	        		 PdcpBuffer.erase(temp[c]);
        	                    Last_Submitted_PDCP_RX_SN = temp[c];
        	                     //PdcpDelayCalculater(temp[c]);

        	             if(Last_Submitted_PDCP_RX_SN == Reordering_PDCP_RX_COUNT % MAX_PDCP_SN -1)
        	                    	                    	check =true;
        	              if(Last_Submitted_PDCP_RX_SN ==m_maxPdcpSn && Reordering_PDCP_RX_COUNT % MAX_PDCP_SN==0)
        	                    	                    	  check =true;

        	            }



        	     delete [] temp;
          }

  if (t_ReorderingTimer.IsRunning())
  {
    if(check == true)
    {
      t_ReorderingTimer.Cancel();
     check = false;
    }
  }

  if (!t_ReorderingTimer.IsRunning())
  {
    if (PdcpBuffer.size()>=1){
      t_ReorderingTimer = Simulator::Schedule(expiredTime, &McUePdcp::t_ReordringTimer_Expired, this);
      Reordering_PDCP_RX_COUNT = Next_PDCP_RX_SN + present_RX_HFN* MAX_PDCP_SN;

    }
  }

}

void
McUePdcp::t_ReordringTimer_Expired(){ // sjkang
  NS_LOG_FUNCTION (this);
NS_LOG_INFO("PDCP Reordering Timer Expired at " << Simulator::Now().GetSeconds() << " Reordering_PDCP_RX_COUNT " << Reordering_PDCP_RX_COUNT);
numberOfReorderingTimeout ++;
//statics <<UeLocation <<  "\t" << Simulator::Now().GetSeconds() << "\t  "<<1<<"\t" << "number of t_reordering timeout " <<
//		" \t " << numberOfReorderingTimeout << "\t" << "inter-expire time" <<" \t"<<
//		Simulator:: Now().GetSeconds()-previousTime.GetSeconds()
//		<< std::endl;
previousTime = Simulator :: Now();
/*
  uint16_t nextPDCP_SN;
  bool compareFlag = false;
  nextPDCP_SN = (Last_Submitted_PDCP_RX_SN + 1)%(m_maxPdcpSn+1);
  while(PdcpBuffer.size() > 0)
  {
//NS_LOG_UNCOND("nextPDCP_SN " << nextPDCP_SN << " Last_Submitted_PDCP_RX_SN " << Last_Submitted_PDCP_RX_SN);

    if (nextPDCP_SN == Reordering_PDCP_RX_COUNT) compareFlag = true;

    std::map<uint16_t, LtePdcpSapUser::ReceivePdcpSduParameters>::iterator it;
    for (it = PdcpBuffer.begin(); it->first != nextPDCP_SN && it != PdcpBuffer.end(); it++)
    { }

    if (it == PdcpBuffer.end())
    {
      if (compareFlag == true) break;
      nextPDCP_SN = (nextPDCP_SN + 1)%(m_maxPdcpSn+1);
    }
    else
    {
      PdcpDelayCalculater(nextPDCP_SN);
      m_pdcpSapUser->ReceivePdcpSdu(PdcpBuffer[nextPDCP_SN]);

      printData("Reordered_SN", nextPDCP_SN);
      PdcpBuffer.erase(nextPDCP_SN);
      Last_Submitted_PDCP_RX_SN = nextPDCP_SN;
      nextPDCP_SN = (Last_Submitted_PDCP_RX_SN + 1)%(m_maxPdcpSn+1);
    }
  }*/


  std::map<int, LtePdcpSapUser::ReceivePdcpSduParameters> ::iterator it;

  // copy the SN from buffer for a while
  temp = new int [(size_t)PdcpBuffer.size()];
  int i=0;
  for (it = PdcpBuffer.begin();it !=PdcpBuffer.end(); ++it, ++i) {
	  temp[i] = it->first + RX_HFNbySN [it->first]*MAX_PDCP_SN;  }

  // sort the SN in acsending order
  std::sort(temp , temp +i);

// ETSI TS 136 323  5.1.2.4.2 procedure: when t- reordering expires

  for (uint16_t c=0; c<i; c++)
  {
    if (temp[c] < Reordering_PDCP_RX_COUNT )
    {
    	if (temp[c]> m_maxPdcpSn) temp[c] = temp[c] % MAX_PDCP_SN;
      m_pdcpSapUser->ReceivePdcpSdu(PdcpBuffer[temp[c]]);
    SumOfPacketSize +=PdcpBuffer[temp[c]].pdcpSdu->GetSize();//sjkang0718
     Last_Submitted_PDCP_RX_SN =temp[c];
     //printData("Reordered_SN", temp[c]);
      PdcpBuffer.erase(temp[c]);
      if(Last_Submitted_PDCP_RX_SN == Reordering_PDCP_RX_COUNT % MAX_PDCP_SN-1) check =true;
    }
  }
  delete [] temp;
  i=0;


  temp =new int [(size_t)PdcpBuffer.size()+1];
  for (it = PdcpBuffer.begin();it !=PdcpBuffer.end(); ++it, ++i)
	  temp[i] = it->first + RX_HFNbySN[it->first]*MAX_PDCP_SN;

  // sort the SN in acsending order
  std::sort(temp , temp +i);

  //if(temp[0]==Reordering_PDCP_RX_COUNT % MAX_PDCP_SN + 1 ){
	  int sum = temp[0],p=0;
	  while (sum == temp[p] && p!= (int)PdcpBuffer.size())
	  {
		  sum =temp[p]+1;
		  p++;
	  }

	  for (uint16_t c = 0 ; c< p; c++)
	  {
		  if (temp[c]> m_maxPdcpSn) temp[c] = temp[c] % MAX_PDCP_SN;
   // PdcpDelayCalculater(temp[c]);
		  m_pdcpSapUser->ReceivePdcpSdu(PdcpBuffer[temp[c]]);
	  SumOfPacketSize +=PdcpBuffer[temp[c]].pdcpSdu->GetSize();//sjkang0718

		 // printData("Reordered_SN", temp[c]);
		  PdcpBuffer.erase(temp[c]);
		  Last_Submitted_PDCP_RX_SN = temp[c];

		  if(Last_Submitted_PDCP_RX_SN == Reordering_PDCP_RX_COUNT % MAX_PDCP_SN-1) check =true;
	  }

 //}
  delete [] temp;
  if (PdcpBuffer.size()>=1 )
  {
    Reordering_PDCP_RX_COUNT = Next_PDCP_RX_SN + present_RX_HFN *MAX_PDCP_SN;
	//  Reordering_PDCP_RX_COUNT =Last_Submitted_PDCP_RX_SN+present_RX_HFN*MAX_PDCP_SN+1;
    t_ReorderingTimer= Simulator::Schedule(expiredTime, &McUePdcp::t_ReordringTimer_Expired, this);
  }
}
std::ofstream OutFile1("pdcp_1_RX_SN.txt");
std::ofstream OutFile2("pdcp_1_Reordered_SN.txt");
//std::ofstream OutFile3("pdcp_1_RX_SN.txt");
//std::ofstream OutFile4("pdcp_2_Reordered_SN.txt");

//int count =0;

Ptr <ns3::LtePdcp> tempAddress1;

void
McUePdcp::printData(std::string filename, uint16_t SN) // sjkang
{
  NS_LOG_FUNCTION (this);
//  if (count ==0){ tempAddress1 =this;count ++;}
 // count ++;

 // if (tempAddress1==this)
  //{
    if (filename == "RX_SN")
    {
      if (!OutFile1.is_open ())
      {
        OutFile1.open ("1_RX_SN.txt");
      }
    OutFile1 << this<< "\t"<<Simulator::Now ().GetSeconds() << "\t"<<"Received SN "<< "\t" << SN<< std::endl;
    }
    else if (filename == "Reordered_SN")
    {
      if (!OutFile2.is_open ())
      {
        OutFile2.open ("1_Reordered_SN.txt");
      }
      OutFile2 << this<< "\t"<<Simulator::Now ().GetSeconds() << "\t"<< "Reordered SN " << "\t" << SN<<
    	 "\t" <<RX_HFNbySN[SN] <<  std::endl;
     }
//}
  /*else
  {
    if (filename == "RX_SN")
    {
      if (!OutFile3.is_open ())
      {
        OutFile3.open ("2_RX_SN.txt");
      }
      OutFile3 <<this<< "\t"<< Simulator::Now ().GetSeconds() << "\t"<<"Received SN "<< "\t" << SN<< std::endl;
    }
    else if (filename == "Reordered_SN")
    {
      if (!OutFile4.is_open ())
      {
        OutFile4.open ("2_Reordered_SN.txt");
      }
      OutFile4<<this<< "\t"<< Simulator::Now ().GetSeconds() << "\t"<< "Reordered SN " << "\t" << SN<< std::endl;
    }
  }*/
}
void
McUePdcp::SwitchConnection (bool useMmWaveConnection)
{
 // std::cout <<this << "switch connection to -----> useMmWaveConnecion == " << useMmWaveConnection << std::endl;
	m_useMmWaveConnection = useMmWaveConnection;
}

void
McUePdcp::CalculatePdcpThroughput(std::ofstream *stream){//sjkang0729
	NS_LOG_FUNCTION(this);

	Time time = Simulator::Now();
   TotalPacketSize +=(double)SumOfPacketSize;
   TotalPacketSize_ordered +=(double)orderdedSumOfPacket;
		TotalTime +=0.1;
 *stream <<time.GetSeconds() << "\t" << (double)((SumOfPacketSize*8)/0.1)/1e6 <<" \t"<< (double)((TotalPacketSize*8)/TotalTime)/1e6
		 <<"\t"<<discardedPacketSize<<"\t"<<(double)((TotalPacketSize_ordered*8)/TotalTime)/1e6<<std::endl;
/*
if (time.GetSeconds() >= m_simulationTime.GetSeconds() && countAtPdcp==0){
	outputAtPdcp<<  (double)((TotalPacketSize*8)/TotalTime)/1e6 << "\t"  ;
	countAtPdcp =1;
	outputAtPdcp.close();
	set<< "\t"<< AverageSnDifference <<  "\t" << numberOfReorderingTimeout<< "\t" << numberOfDiscaredPackets<< "\t"
			<<(double)((TotalPacketSize*8)/TotalTime)/1e6 <<"\t" ;
	set.close();
        }*/
	SumOfPacketSize = 0;
	orderdedSumOfPacket=0;

	  Simulator::Schedule(Seconds(0.1),&McUePdcp::CalculatePdcpThroughput, this,stream);
}
void
McUePdcp::MeasureSN_Difference(){ //sjkang1116
	//if(cellIdToSN_1 != 0 && cellIdToSN_2 != 0 ) {
	*m_SN_DifferenceStream<<Simulator::Now().GetSeconds() << "\t" << (int)(cellIdToSN_1-cellIdToSN_2) << "\t"<< cellId_1 << "\t " << cellId_2 << std::endl;
	//:cellIdToSN_1 =0 ; cellIdToSN_2 = 0;
	//}

	Simulator::Schedule(MilliSeconds(1.0), &McUePdcp::MeasureSN_Difference, this);
}
void
McUePdcp::sendControlMessage(){
	 /*if (cellIdToSN_1 - cellIdToSN_2 > 3000){
	    	Ptr<Packet> empty_p = Create <Packet> ();
	    	  m_alwaysLteUplink =true;
	    	  	  DoTransmitPdcpControlPacket(empty_p, cellId_1);
	     }else if (cellIdToSN_2 - cellIdToSN_1 > 3000){
	    	 	 Ptr<Packet> empty_p = Create <Packet> ();
	    	     m_alwaysLteUplink =true;
	    	    	  DoTransmitPdcpControlPacket(empty_p, cellId_2);

	    }else{
	    	  	    	Ptr<Packet> empty_p = Create <Packet> ();
	        	     	  m_alwaysLteUplink =true;
	        	    	  DoTransmitPdcpControlPacket(empty_p,0);
	        	     }
	 Simulator::Schedule(MilliSeconds(5), &McUePdcp::sendControlMessage,this);*/
}
} // namespace ns3
