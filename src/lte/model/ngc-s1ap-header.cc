/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Michele Polese <michele.polese@gmail.com>
 * Inspired by ngc-x2-header.cc
 */

#include "ns3/log.h"
#include "ns3/ngc-s1ap-header.h"
#include <list>


// TODO 
// According to 36.413 9.2.3.4: enbUeS1Id should be 3 byte, but in the SAP interface 
// already defined in the ns-3 release is 2 byte
// The same holds for mmeUeS1Id, which should be 4 byte, but is 8 byte in the SAP interface

namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("NgcS1APHeader");

NS_OBJECT_ENSURE_REGISTERED (NgcS1APHeader);

NgcS1APHeader::NgcS1APHeader ()
  : m_procedureCode (0xfa),
    m_lengthOfIes (0xfa),
    m_numberOfIes (0xfa)
{
}

NgcS1APHeader::~NgcS1APHeader ()
{
  m_procedureCode = 0xfb;
  m_lengthOfIes = 0xfb;
  m_numberOfIes = 0xfb;
}

TypeId
NgcS1APHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APHeader::GetSerializedSize (void) const
{
  return 6; // 6 bytes in this header
}

void
NgcS1APHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_procedureCode);

  i.WriteU8 (0x00); // 36.413 9.1.2.2 Criticality - if not recognized, 0 == reject
  i.WriteU8 (m_lengthOfIes + 3);
  i.WriteHtonU16 (0);
  i.WriteU8 (m_numberOfIes);
}

uint32_t
NgcS1APHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_procedureCode = i.ReadU8 ();

  i.ReadU8 ();
  m_lengthOfIes = i.ReadU8 () - 3;
  i.ReadNtohU16 ();
  m_numberOfIes = i.ReadU8 ();
  
  return GetSerializedSize ();
}

void
NgcS1APHeader::Print (std::ostream &os) const
{
  os << " ProcedureCode=" << (uint32_t) m_procedureCode;
  os << " LengthOfIEs=" << (uint32_t) m_lengthOfIes;
  os << " NumberOfIEs=" << (uint32_t) m_numberOfIes;
}

uint8_t
NgcS1APHeader::GetProcedureCode () const
{
  return m_procedureCode;
}

void
NgcS1APHeader::SetProcedureCode (uint8_t procedureCode)
{
  m_procedureCode = procedureCode;
}


void
NgcS1APHeader::SetLengthOfIes (uint32_t lengthOfIes)
{
  m_lengthOfIes = lengthOfIes;
}

void
NgcS1APHeader::SetNumberOfIes (uint32_t numberOfIes)
{
  m_numberOfIes = numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APInitialUeMessageHeader);

NgcS1APInitialUeMessageHeader::NgcS1APInitialUeMessageHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1 + 1 + 1),
    m_headerLength (3 + 2 + 6 + 4 + 2 + 9 + 9),
    m_stmsi (0xfffffffa),
    m_mmeUeS1Id (0xfffffffa),
    m_enbUeS1Id (0xfffa),
    m_ecgi (0xfffa)
{
}

NgcS1APInitialUeMessageHeader::~NgcS1APInitialUeMessageHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_stmsi = 0xfffffffb;
  m_enbUeS1Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
}

TypeId
NgcS1APInitialUeMessageHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APInitialUeMessageHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APInitialUeMessageHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APInitialUeMessageHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APInitialUeMessageHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcS1APInitialUeMessageHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader
  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU8 (0);                    // NAS PDU, not implemented
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU16 (m_ecgi);          // E-UTRAN CGI, it should have a different size
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU8 (0);                    // RRC Establishment cause
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU64 (m_stmsi);             // S-TMSI
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id, not in the standard?
  i.WriteU8 (0);                    // criticality = REJECT

}

uint32_t
NgcS1APInitialUeMessageHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeS1Id = i.ReadNtohU16 ();
  i.ReadU8 ();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();                    
  m_headerLength += 6;
  m_numberOfIes++;

  m_ecgi = i.ReadNtohU16 ();    // E-UTRAN CGI, it should have a different size
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  m_stmsi = i.ReadU64 ();             // S-TMSI
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  m_mmeUeS1Id = i.ReadU64 ();             // MME UE ID
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcS1APInitialUeMessageHeader::Print (std::ostream &os) const
{
  os << "MmeUeS1apId = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  os << " ECGI = " << m_ecgi;
  os << " S-TMSI = " << m_stmsi;
}

uint64_t 
NgcS1APInitialUeMessageHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APInitialUeMessageHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APInitialUeMessageHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APInitialUeMessageHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

uint64_t 
NgcS1APInitialUeMessageHeader::GetSTmsi () const 
{
  return m_stmsi;
}

void 
NgcS1APInitialUeMessageHeader::SetSTmsi (uint64_t stmsi) 
{
  m_stmsi = stmsi;
}

uint16_t 
NgcS1APInitialUeMessageHeader::GetEcgi () const 
{
  return m_ecgi;
}

void 
NgcS1APInitialUeMessageHeader::SetEcgi (uint16_t ecgi)
{
  m_ecgi = ecgi;
}

uint32_t
NgcS1APInitialUeMessageHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APInitialUeMessageHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APErabReleaseIndicationHeader);

NgcS1APErabReleaseIndicationHeader::NgcS1APErabReleaseIndicationHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 4 + 1),
    m_enbUeS1Id (0xfffa),
    m_mmeUeS1Id (0xfffffffa)
{
  m_erabToBeReleaseIndication.clear();
}

NgcS1APErabReleaseIndicationHeader::~NgcS1APErabReleaseIndicationHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeS1Id = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
  m_erabToBeReleaseIndication.clear();
}

TypeId
NgcS1APErabReleaseIndicationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APErabReleaseIndicationHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APErabReleaseIndicationHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APErabReleaseIndicationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APErabReleaseIndicationHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcS1APErabReleaseIndicationHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader
  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (0);                    // criticality = REJECT

  std::list <NgcS1apSap::ErabToBeReleasedIndication>::size_type sz = m_erabToBeReleaseIndication.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcS1apSap::ErabToBeReleasedIndication>::const_iterator l_iter = m_erabToBeReleaseIndication.begin(); l_iter != m_erabToBeReleaseIndication.end(); ++l_iter) // content of ErabToBeReleasedIndication
  {
    i.WriteU8 (l_iter->erabId);
  }
  i.WriteU8(0); // criticality = REJECT, just one for the whole list

}

uint32_t
NgcS1APErabReleaseIndicationHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_mmeUeS1Id = i.ReadU64 ();         // mmeUeS1Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeS1Id = i.ReadNtohU16 ();     // m_enbUeS1Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcS1apSap::ErabToBeReleasedIndication erabItem;
    erabItem.erabId = i.ReadU8 ();

    m_erabToBeReleaseIndication.push_back(erabItem);
    m_headerLength += 1;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcS1APErabReleaseIndicationHeader::Print (std::ostream &os) const
{
  os << "MmeUeS1apId = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  for(std::list <NgcS1apSap::ErabToBeReleasedIndication>::const_iterator l_iter = m_erabToBeReleaseIndication.begin(); l_iter != m_erabToBeReleaseIndication.end(); ++l_iter)
  { 
    os << " ErabId " << l_iter->erabId;
  }

}

uint64_t 
NgcS1APErabReleaseIndicationHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APErabReleaseIndicationHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APErabReleaseIndicationHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APErabReleaseIndicationHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

std::list<NgcS1apSap::ErabToBeReleasedIndication>
NgcS1APErabReleaseIndicationHeader::GetErabToBeReleaseIndication () const 
{
  return m_erabToBeReleaseIndication;
}

void 
NgcS1APErabReleaseIndicationHeader::SetErabReleaseIndication (std::list<NgcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  m_headerLength += erabToBeReleaseIndication.size();
  m_erabToBeReleaseIndication = erabToBeReleaseIndication;
}

uint32_t
NgcS1APErabReleaseIndicationHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APErabReleaseIndicationHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APInitialContextSetupResponseHeader);

NgcS1APInitialContextSetupResponseHeader::NgcS1APInitialContextSetupResponseHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 4 + 1),
    m_enbUeS1Id (0xfffa),
    m_mmeUeS1Id (0xfffffffa)
{
  m_erabSetupList.clear();
}

NgcS1APInitialContextSetupResponseHeader::~NgcS1APInitialContextSetupResponseHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeS1Id = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
  m_erabSetupList.clear();
}

TypeId
NgcS1APInitialContextSetupResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APInitialContextSetupResponseHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APInitialContextSetupResponseHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APInitialContextSetupResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APInitialContextSetupResponseHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcS1APInitialContextSetupResponseHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader
  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  std::list <NgcS1apSap::ErabSetupItem>::size_type sz = m_erabSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcS1apSap::ErabSetupItem>::const_iterator l_iter = m_erabSetupList.begin(); l_iter != m_erabSetupList.end(); ++l_iter) // content of m_erabSetupList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->enbTransportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (1 << 6);               // criticality = IGNORE each
  }
  i.WriteU8 (1 << 6);               // criticality = IGNORE

}

uint32_t
NgcS1APInitialContextSetupResponseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_mmeUeS1Id = i.ReadU64 ();         // mmeUeS1Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeS1Id = i.ReadNtohU16 ();     // m_enbUeS1Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcS1apSap::ErabSetupItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.enbTransportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabSetupList.push_back(erabItem);
    m_headerLength += 10;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcS1APInitialContextSetupResponseHeader::Print (std::ostream &os) const
{
  os << "MmeUeS1apId = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  for (std::list <NgcS1apSap::ErabSetupItem>::const_iterator l_iter = m_erabSetupList.begin(); l_iter != m_erabSetupList.end(); ++l_iter) // content of m_erabSetupList
  {
    os << " ErabId " << l_iter->erabId;
    os << " enbTransportLayerAddress " << l_iter->enbTransportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcS1APInitialContextSetupResponseHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APInitialContextSetupResponseHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APInitialContextSetupResponseHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APInitialContextSetupResponseHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

std::list<NgcS1apSap::ErabSetupItem>
NgcS1APInitialContextSetupResponseHeader::GetErabSetupItem () const 
{
  return m_erabSetupList;
}

void 
NgcS1APInitialContextSetupResponseHeader::SetErabSetupItem (std::list<NgcS1apSap::ErabSetupItem> erabSetupList)
{
  m_headerLength += erabSetupList.size() * 10;
  m_erabSetupList = erabSetupList;
}

uint32_t
NgcS1APInitialContextSetupResponseHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APInitialContextSetupResponseHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APPathSwitchRequestHeader);

NgcS1APPathSwitchRequestHeader::NgcS1APPathSwitchRequestHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1),
    m_headerLength (3 + 4 + 1 +9 + 3 + 6),
    m_enbUeS1Id (0xfffa),
    m_ecgi (0xfffa),
    m_mmeUeS1Id (0xfffffffa)
{
  m_erabToBeSwitchedInDownlinkList.clear();
}

NgcS1APPathSwitchRequestHeader::~NgcS1APPathSwitchRequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeS1Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
  m_erabToBeSwitchedInDownlinkList.clear();
}

TypeId
NgcS1APPathSwitchRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APPathSwitchRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APPathSwitchRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APPathSwitchRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APPathSwitchRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}


void
NgcS1APPathSwitchRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader 
  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (0);               // criticality = REJECT

  std::list <NgcS1apSap::ErabSwitchedInDownlinkItem>::size_type sz = m_erabToBeSwitchedInDownlinkList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcS1apSap::ErabSwitchedInDownlinkItem>::const_iterator l_iter = m_erabToBeSwitchedInDownlinkList.begin(); l_iter != m_erabToBeSwitchedInDownlinkList.end(); ++l_iter) // content of ErabToBeReleasedIndication // content of m_erabToBeSwitchedInDownlinkList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->enbTransportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (0);               // criticality = REJECT each
  }
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU16 (m_ecgi);      // ecgi
  i.WriteU8 (1 << 6);       // criticality = IGNORE

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  // TODO add 9.2.1.40

}

uint32_t
NgcS1APPathSwitchRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeS1Id = i.ReadNtohU16 ();     // m_enbUeS1Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcS1apSap::ErabSwitchedInDownlinkItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.enbTransportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabToBeSwitchedInDownlinkList.push_back(erabItem);
    m_headerLength += 10;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;

  m_mmeUeS1Id = i.ReadU64 ();         // mmeUeS1Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_ecgi = i.ReadU16 ();      // ecgi
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();               // criticality = IGNORE

  m_headerLength += 6;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcS1APPathSwitchRequestHeader::Print (std::ostream &os) const
{
  os << "MmeUeS1apId = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  os << " ECGI = " << m_ecgi; 
  for (std::list <NgcS1apSap::ErabSwitchedInDownlinkItem>::const_iterator l_iter = m_erabToBeSwitchedInDownlinkList.begin(); l_iter != m_erabToBeSwitchedInDownlinkList.end(); ++l_iter) // content of ErabToBeReleasedIndication // content of m_erabToBeSwitchedInDownlinkList
  {
    os << " ErabId " << l_iter->erabId;
    os << " enbTransportLayerAddress " << l_iter->enbTransportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcS1APPathSwitchRequestHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APPathSwitchRequestHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APPathSwitchRequestHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APPathSwitchRequestHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

std::list<NgcS1apSap::ErabSwitchedInDownlinkItem>
NgcS1APPathSwitchRequestHeader::GetErabSwitchedInDownlinkItemList () const 
{
  return m_erabToBeSwitchedInDownlinkList;
}

void 
NgcS1APPathSwitchRequestHeader::SetErabSwitchedInDownlinkItemList (std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> erabSetupList)
{
	m_headerLength += erabSetupList.size()*10;
  m_erabToBeSwitchedInDownlinkList = erabSetupList;
}

uint16_t
NgcS1APPathSwitchRequestHeader::GetEcgi() const
{
  return m_ecgi;
}

void
NgcS1APPathSwitchRequestHeader::SetEcgi(uint16_t ecgi) 
{
  m_ecgi = ecgi;
}

uint32_t
NgcS1APPathSwitchRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APPathSwitchRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APInitialContextSetupRequestHeader);

NgcS1APInitialContextSetupRequestHeader::NgcS1APInitialContextSetupRequestHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 9 + 4 + 1),
    m_enbUeS1Id (0xfffa),
    m_mmeUeS1Id (0xfffffffa)
{
  m_erabsToBeSetupList.clear();
}

NgcS1APInitialContextSetupRequestHeader::~NgcS1APInitialContextSetupRequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeS1Id = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
  m_erabsToBeSetupList.clear();
}

TypeId
NgcS1APInitialContextSetupRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APInitialContextSetupRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APInitialContextSetupRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APInitialContextSetupRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APInitialContextSetupRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcS1APInitialContextSetupRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader
  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU64 (0);               // aggregate maximum bitrate, not implemented
  i.WriteU8 (0);

  std::list <NgcS1apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcS1apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
    {
      i.WriteU8 (l_iter->erabId);
      i.WriteHtonU16 (l_iter->erabLevelQosParameters.qci);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrUl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrUl);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.priorityLevel);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionCapability);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionVulnerability);
      i.WriteHtonU32 (l_iter->transportLayerAddress.Get ());
      i.WriteHtonU32 (l_iter->smfTeid);

      i.WriteU8(0); // a criticaloty each, REJECT
    }
  i.WriteU8 (0);               // criticality = REJECT

  //TODO 9.2.140, 9.2.1.41

}

uint32_t
NgcS1APInitialContextSetupRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_mmeUeS1Id = i.ReadU64 ();         // mmeUeS1Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeS1Id = i.ReadNtohU16 ();     // m_enbUeS1Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU64 ();               // aggregate maximum bitrate, not implemented
  i.ReadU8 ();
  m_headerLength += 9;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of m_erabToBeSetupList
  {
    NgcS1apSap::ErabToBeSetupItem erabItem;
    erabItem.erabId = i.ReadU8 ();
 
    erabItem.erabLevelQosParameters = EpsBearer ((EpsBearer::Qci) i.ReadNtohU16 ());
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.arp.priorityLevel = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionCapability = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionVulnerability = i.ReadU8 ();

    erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.smfTeid = i.ReadNtohU32 ();

    i.ReadU8 ();

    m_erabsToBeSetupList.push_back (erabItem);
    m_headerLength += 46;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcS1APInitialContextSetupRequestHeader::Print (std::ostream &os) const
{
  os << " MmeUeS1Id = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  os << " NumOfBearers = " << m_erabsToBeSetupList.size ();

  std::list <NgcS1apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  int j = 0;  
  for (std::list <NgcS1apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    os << l_iter->erabId;
    if (j < (int) sz - 1)
      {
        os << ", ";
      }
    else
      {
        os << "]";
      }
    j++;  
  }
}

uint64_t 
NgcS1APInitialContextSetupRequestHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APInitialContextSetupRequestHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APInitialContextSetupRequestHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APInitialContextSetupRequestHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

std::list<NgcS1apSap::ErabToBeSetupItem>
NgcS1APInitialContextSetupRequestHeader::GetErabToBeSetupItem () const 
{
  return m_erabsToBeSetupList;
}

void 
NgcS1APInitialContextSetupRequestHeader::SetErabToBeSetupItem (std::list<NgcS1apSap::ErabToBeSetupItem> erabSetupList)
{
  m_headerLength += erabSetupList.size()*47;
  m_erabsToBeSetupList = erabSetupList;
}

uint32_t
NgcS1APInitialContextSetupRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APInitialContextSetupRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcS1APPathSwitchRequestAcknowledgeHeader);

NgcS1APPathSwitchRequestAcknowledgeHeader::NgcS1APPathSwitchRequestAcknowledgeHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1),
    m_headerLength (3 + 4 + 1 + 9 + 3 + 6),
    m_enbUeS1Id (0xfffa),
    m_ecgi (0xfffa),
    m_mmeUeS1Id (0xfffffffa)
{
  m_erabToBeSwitchedInUplinkList.clear();
}

NgcS1APPathSwitchRequestAcknowledgeHeader::~NgcS1APPathSwitchRequestAcknowledgeHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeS1Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_mmeUeS1Id = 0xfffffffb;
  m_erabToBeSwitchedInUplinkList.clear();
}

TypeId
NgcS1APPathSwitchRequestAcknowledgeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcS1APPathSwitchRequestAcknowledgeHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<NgcS1APPathSwitchRequestAcknowledgeHeader> ()
  ;
  return tid;
}

TypeId
NgcS1APPathSwitchRequestAcknowledgeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcS1APPathSwitchRequestAcknowledgeHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}


void
NgcS1APPathSwitchRequestAcknowledgeHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcS1APHeader 
  i.WriteHtonU16 (m_enbUeS1Id);     // m_enbUeS1Id
  i.WriteU8 (0);               // criticality = REJECT

  std::vector <NgcS1apSap::ErabSwitchedInUplinkItem>::size_type sz = m_erabToBeSwitchedInUplinkList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcS1apSap::ErabSwitchedInUplinkItem>::const_iterator l_iter = m_erabToBeSwitchedInUplinkList.begin(); l_iter != m_erabToBeSwitchedInUplinkList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->transportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (0);               // criticality = REJECT each
  }
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU64 (m_mmeUeS1Id);         // mmeUeS1Id
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU16 (m_ecgi);      // ecgi
  i.WriteU8 (1 << 6);       // criticality = IGNORE

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  // TODO add 9.2.1.40

}

uint32_t
NgcS1APPathSwitchRequestAcknowledgeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeS1Id = i.ReadNtohU16 ();     // m_enbUeS1Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcS1apSap::ErabSwitchedInUplinkItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabToBeSwitchedInUplinkList.push_back(erabItem);
    m_headerLength += 9;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;

  m_mmeUeS1Id = i.ReadU64 ();         // mmeUeS1Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_ecgi = i.ReadU16 ();      // ecgi
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();               // criticality = IGNORE

  m_headerLength += 6;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcS1APPathSwitchRequestAcknowledgeHeader::Print (std::ostream &os) const
{
  os << "MmeUeS1apId = " << m_mmeUeS1Id;
  os << " EnbUeS1Id = " << m_enbUeS1Id;
  os << " ECGI = " << m_ecgi; 
  for (std::list <NgcS1apSap::ErabSwitchedInUplinkItem>::const_iterator l_iter = m_erabToBeSwitchedInUplinkList.begin(); l_iter != m_erabToBeSwitchedInUplinkList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    os << " ErabId " << l_iter->erabId;
    os << " TransportLayerAddress " << l_iter->transportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcS1APPathSwitchRequestAcknowledgeHeader::GetMmeUeS1Id () const 
{
  return m_mmeUeS1Id;
}

void 
NgcS1APPathSwitchRequestAcknowledgeHeader::SetMmeUeS1Id (uint64_t mmeUeS1Id) 
{
  m_mmeUeS1Id = mmeUeS1Id;
}

uint16_t 
NgcS1APPathSwitchRequestAcknowledgeHeader::GetEnbUeS1Id () const
{
  return m_enbUeS1Id;
}

void 
NgcS1APPathSwitchRequestAcknowledgeHeader::SetEnbUeS1Id (uint16_t enbUeS1Id)
{
  m_enbUeS1Id = enbUeS1Id;
}

std::list<NgcS1apSap::ErabSwitchedInUplinkItem>
NgcS1APPathSwitchRequestAcknowledgeHeader::GetErabSwitchedInUplinkItemList () const 
{
  return m_erabToBeSwitchedInUplinkList;
}

void 
NgcS1APPathSwitchRequestAcknowledgeHeader::SetErabSwitchedInUplinkItemList (std::list<NgcS1apSap::ErabSwitchedInUplinkItem> erabSetupList)
{
  m_headerLength += erabSetupList.size() * 10;
  m_erabToBeSwitchedInUplinkList = erabSetupList;
}

uint16_t
NgcS1APPathSwitchRequestAcknowledgeHeader::GetEcgi() const
{
  return m_ecgi;
}

void
NgcS1APPathSwitchRequestAcknowledgeHeader::SetEcgi(uint16_t ecgi) 
{
  m_ecgi = ecgi;
}

uint32_t
NgcS1APPathSwitchRequestAcknowledgeHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcS1APPathSwitchRequestAcknowledgeHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

}; // end of namespace ns3
