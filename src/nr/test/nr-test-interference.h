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
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef NR_TEST_INTERFERENCE_H
#define NR_TEST_INTERFERENCE_H

#include "ns3/test.h"


using namespace ns3;


/**
 * Test that SINR calculation and MCS selection works fine in a multi-cell interference scenario.
 */
class NrInterferenceTestSuite : public TestSuite
{
public:
  NrInterferenceTestSuite ();
};


class NrInterferenceTestCase : public TestCase
{
public:
  NrInterferenceTestCase (std::string name, double d1, double d2, double dlSinr, double ulSinr, double dlSe, double ulSe, uint16_t dlMcs, uint16_t ulMcs);
  virtual ~NrInterferenceTestCase ();

  void DlScheduling (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                     uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2);

  void UlScheduling (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                     uint8_t mcs, uint16_t sizeTb);

private:
  virtual void DoRun (void);


  double m_d1;
  double m_d2;
  double m_expectedDlSinrDb;
  double m_expectedUlSinrDb;
  uint16_t m_dlMcs;
  uint16_t m_ulMcs;
};

#endif /* NR_TEST_INTERFERENCE_H */