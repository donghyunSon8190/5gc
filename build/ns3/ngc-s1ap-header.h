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
 * Inspired by ngc-x2-header.h
 */

#ifndef NGC_S1AP_HEADER_H
#define NGC_S1AP_HEADER_H

#include "ns3/ngc-s1ap-sap.h"
#include "ns3/header.h"

#include <list>


namespace ns3 {


class NgcS1APHeader : public Header
{
public:
  NgcS1APHeader ();
  virtual ~NgcS1APHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint8_t GetProcedureCode () const;
  void SetProcedureCode (uint8_t procedureCode);

  void SetLengthOfIes (uint32_t lengthOfIes);
  void SetNumberOfIes (uint32_t numberOfIes);


  enum ProcedureCode_t {
    InitialUeMessage        = 71,
    PathSwitchRequest       = 58,
    ErabReleaseIndication   = 37,
    InitialContextSetupResponse = 43,
    InitialContextSetupRequest = 41,
    PathSwitchRequestAck = 59
  };


private:
  uint8_t m_procedureCode;

  uint32_t m_lengthOfIes;
  uint32_t m_numberOfIes;
};


// Header for initial Ue message
class NgcS1APInitialUeMessageHeader : public Header
{
public:
  NgcS1APInitialUeMessageHeader ();
  virtual ~NgcS1APInitialUeMessageHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  uint64_t GetSTmsi () const;
  void SetSTmsi (uint64_t stmsi);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint64_t          m_stmsi;
  uint64_t          m_mmeUeS1Id;
  uint16_t          m_enbUeS1Id;
  uint16_t          m_ecgi;
};



class NgcS1APErabReleaseIndicationHeader : public Header
{
public:
  NgcS1APErabReleaseIndicationHeader ();
  virtual ~NgcS1APErabReleaseIndicationHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  std::list<NgcS1apSap::ErabToBeReleasedIndication> GetErabToBeReleaseIndication () const;
  void SetErabReleaseIndication (std::list<NgcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeS1Id;
  uint64_t          m_mmeUeS1Id;
  std::list<NgcS1apSap::ErabToBeReleasedIndication> m_erabToBeReleaseIndication;
};


class NgcS1APInitialContextSetupResponseHeader : public Header
{
public:
  NgcS1APInitialContextSetupResponseHeader ();
  virtual ~NgcS1APInitialContextSetupResponseHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  std::list<NgcS1apSap::ErabSetupItem> GetErabSetupItem () const;
  void SetErabSetupItem (std::list<NgcS1apSap::ErabSetupItem> erabSetupList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeS1Id;
  uint64_t          m_mmeUeS1Id;
  std::list<NgcS1apSap::ErabSetupItem> m_erabSetupList;
};

class NgcS1APPathSwitchRequestHeader : public Header
{
public:
  NgcS1APPathSwitchRequestHeader ();
  virtual ~NgcS1APPathSwitchRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> GetErabSwitchedInDownlinkItemList () const;
  void SetErabSwitchedInDownlinkItemList (std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeS1Id;
  uint16_t          m_ecgi;
  uint64_t          m_mmeUeS1Id;
  std::list<NgcS1apSap::ErabSwitchedInDownlinkItem> m_erabToBeSwitchedInDownlinkList;
};

class NgcS1APInitialContextSetupRequestHeader : public Header
{
public:
  NgcS1APInitialContextSetupRequestHeader ();
  virtual ~NgcS1APInitialContextSetupRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  std::list<NgcS1apSap::ErabToBeSetupItem> GetErabToBeSetupItem () const;
  void SetErabToBeSetupItem (std::list<NgcS1apSap::ErabToBeSetupItem> erabToBeSetupList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeS1Id;
  uint64_t          m_mmeUeS1Id;
  std::list<NgcS1apSap::ErabToBeSetupItem> m_erabsToBeSetupList;
};

class NgcS1APPathSwitchRequestAcknowledgeHeader : public Header
{
public:
  NgcS1APPathSwitchRequestAcknowledgeHeader ();
  virtual ~NgcS1APPathSwitchRequestAcknowledgeHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetMmeUeS1Id () const;
  void SetMmeUeS1Id (uint64_t mmeUeS1Id);

  uint16_t GetEnbUeS1Id () const;
  void SetEnbUeS1Id (uint16_t enbUeS1Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  std::list<NgcS1apSap::ErabSwitchedInUplinkItem> GetErabSwitchedInUplinkItemList () const;
  void SetErabSwitchedInUplinkItemList (std::list<NgcS1apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInDownlinkList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeS1Id;
  uint16_t          m_ecgi;
  uint64_t          m_mmeUeS1Id;
  std::list<NgcS1apSap::ErabSwitchedInUplinkItem> m_erabToBeSwitchedInUplinkList;
};

}

#endif //NGC_S1AP_HEADER_H