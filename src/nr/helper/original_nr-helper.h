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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef ORIGINAL_NR_HELPER_H
#define ORIGINAL_NR_HELPER_H

#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/names.h>
#include <ns3/original_net-device.h>
#include <ns3/original_net-device-container.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/eps-bearer.h>
#include <ns3/nr-phy-stats-calculator.h>
#include <ns3/nr-phy-tx-stats-calculator.h>
#include <ns3/nr-phy-rx-stats-calculator.h>
#include <ns3/nr-mac-stats-calculator.h>
#include <ns3/nr-radio-bearer-stats-calculator.h>
#include <ns3/nr-radio-bearer-stats-connector.h>
#include <ns3/ngc-tft.h>
#include <ns3/mobility-model.h>

namespace ns3 {


class NrUePhy;
class NrEnbPhy;
class SpectrumChannel;
class NgcHelper1;
class PropagationLossModel;
class SpectrumPropagationLossModel;

/**
 * \ingroup nr
 *
 * Creation and configuration of NR entities. One NrHelper1 instance is
 * typically enough for an NR simulation. To create it:
 *
 *     Ptr<NrHelper1> nrHelper = CreateObject<NrHelper1> ();
 *
 * The general responsibility of the helper is to create various NR objects
 * and arrange them together to make the whole NR system. The overall
 * arrangement would look like the following:
 * - Downlink spectrum channel
 *   + Path loss model
 *   + Fading model
 * - Uplink spectrum channel
 *   + Path loss model
 *   + Fading model
 * - eNodeB node(s)
 *   + Mobility model
 *   + eNodeB device(s)
 *     * Antenna model
 *     * eNodeB PHY (includes spectrum PHY, interference model, HARQ model)
 *     * eNodeB MAC
 *     * eNodeB RRC (includes RRC protocol)
 *     * Scheduler
 *     * Handover algorithm
 *     * FFR (frequency reuse) algorithm
 *     * ANR (automatic neighbour relation)
 *   + NGC related models (NGC application, Internet stack, X2 interface)
 * - UE node(s)
 *   + Mobility model
 *   + UE device(s)
 *     * Antenna model
 *     * UE PHY (includes spectrum PHY, interference model, HARQ model)
 *     * UE MAC
 *     * UE RRC (includes RRC protocol)
 *     * NAS
 * - NGC helper
 * - Various statistics calculator objects
 *
 * Spetrum channels are created automatically: one for DL, and one for UL.
 * eNodeB devices are created by calling InstallEnbDevice(), while UE devices
 * are created by calling InstallUeDevice(). NGC helper can be set by using
 * SetNgcHelper().
 */
class NrHelper1 : public Object
{
public:
  NrHelper1 (void);
  virtual ~NrHelper1 (void);

  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);

  /** 
   * Set the NgcHelper1 to be used to setup the NGC network in
   * conjunction with the setup of the NR radio access network.
   *
   * \note if no NgcHelper1 is ever set, then NrHelper1 will default
   * to creating an NR-only simulation with no NGC, using NrRlcSm as
   * the RLC model, and without supporting any IP networking. In other
   * words, it will be a radio-level simulation involving only NR PHY
   * and MAC and the FF Scheduler, with a saturation traffic model for
   * the RLC.
   * 
   * \param h a pointer to the NgcHelper1 to be used
   */
  void SetNgcHelper (Ptr<NgcHelper1> h);

  /** 
   * Set the type of path loss model to be used for both DL and UL channels.
   * 
   * \param type type of path loss model, must be a type name of any class
   *             inheriting from ns3::PropagationLossModel, for example:
   *             "ns3::FriisPropagationLossModel"
   */
  void SetPathlossModelType (std::string type);

  /**
   * Set an attribute for the path loss models to be created.
   * 
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetPathlossModelAttribute (std::string n, const AttributeValue &v);

  /** 
   * Set the type of scheduler to be used by eNodeB devices.
   * 
   * \param type type of scheduler, must be a type name of any class
   *             inheriting from ns3::NrFfMacScheduler, for example:
   *             "ns3::NrPfFfMacScheduler"
   *
   * Equivalent with setting the `Scheduler` attribute.
   */
  void SetSchedulerType (std::string type);

  /**
   *
   * \return the scheduler type
   */
  std::string GetSchedulerType () const; 

  /**
   * Set an attribute for the scheduler to be created.
   * 
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetSchedulerAttribute (std::string n, const AttributeValue &v);

  /**
   * Set the type of FFR algorithm to be used by eNodeB devices.
   *
   * \param type type of FFR algorithm, must be a type name of any class
   *             inheriting from ns3::NrFfrAlgorithm, for example:
   *             "ns3::NrFrNoOpAlgorithm"
   *
   * Equivalent with setting the `FfrAlgorithm` attribute.
   */
  void SetFfrAlgorithmType (std::string type);

  /**
   *
   * \return the FFR algorithm type
   */
  std::string GetFfrAlgorithmType () const;

  /**
   * Set an attribute for the FFR algorithm to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetFfrAlgorithmAttribute (std::string n, const AttributeValue &v);

  /**
   * Set the type of handover algorithm to be used by eNodeB devices.
   *
   * \param type type of handover algorithm, must be a type name of any class
   *             inheriting from ns3::NrHandoverAlgorithm, for example:
   *             "ns3::NrNoOpHandoverAlgorithm"
   *
   * Equivalent with setting the `HandoverAlgorithm` attribute.
   */
  void SetHandoverAlgorithmType (std::string type);

  /**
   *
   * \return the handover algorithm type
   */
  std::string GetHandoverAlgorithmType () const;

  /**
   * Set an attribute for the handover algorithm to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetHandoverAlgorithmAttribute (std::string n, const AttributeValue &v);

  /**
   * Set an attribute for the eNodeB devices (NrEnbNetDevice) to be created.
   * 
   * \param n the name of the attribute.
   * \param v the value of the attribute
   */
  void SetEnbDeviceAttribute (std::string n, const AttributeValue &v);

  /** 
   * Set the type of antenna model to be used by eNodeB devices.
   * 
   * \param type type of antenna model, must be a type name of any class
   *             inheriting from ns3::AntennaModel, for example:
   *             "ns3::IsotropicAntennaModel"
   */
  void SetEnbAntennaModelType (std::string type);

  /**
   * Set an attribute for the eNodeB antenna model to be created.
   * 
   * \param n the name of the attribute.
   * \param v the value of the attribute
   */
  void SetEnbAntennaModelAttribute (std::string n, const AttributeValue &v);

  /**
   * Set an attribute for the UE devices (NrUeNetDevice) to be created.
   *
   * \param n the name of the attribute.
   * \param v the value of the attribute
   */
  void SetUeDeviceAttribute (std::string n, const AttributeValue &v);

  /** 
   * Set the type of antenna model to be used by UE devices.
   * 
   * \param type type of antenna model, must be a type name of any class
   *             inheriting from ns3::AntennaModel, for example:
   *             "ns3::IsotropicAntennaModel"
   */
  void SetUeAntennaModelType (std::string type);

  /**
   * Set an attribute for the UE antenna model to be created.
   * 
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeAntennaModelAttribute (std::string n, const AttributeValue &v);

  /** 
   * Set the type of spectrum channel to be used in both DL and UL.
   *
   * \param type type of spectrum channel model, must be a type name of any
   *             class inheriting from ns3::SpectrumChannel, for example:
   *             "ns3::MultiModelSpectrumChannel"
   */
  void SetSpectrumChannelType (std::string type);

  /**
   * Set an attribute for the spectrum channel to be created (both DL and UL).
   * 
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetSpectrumChannelAttribute (std::string n, const AttributeValue &v);

  /**
   * Create a set of eNodeB devices.
   *
   * \param c the node container where the devices are to be installed
   * \return the NetDeviceContainer1 with the newly created devices
   */
  NetDeviceContainer1 InstallEnbDevice (NodeContainer c);

  /**
   * Create a set of UE devices.
   *
   * \param c the node container where the devices are to be installed
   * \return the NetDeviceContainer1 with the newly created devices
   */
  NetDeviceContainer1 InstallUeDevice (NodeContainer c);

  /**
   * \brief Enables automatic attachment of a set of UE devices to a suitable
   *        cell using Idle mode initial cell selection procedure.
   * \param ueDevices the set of UE devices to be attached
   *
   * By calling this, the UE will start the initial cell selection procedure at
   * the beginning of simulation. In addition, the function also instructs each
   * UE to immediately enter CONNECTED mode and activates the default EPS
   * bearer.
   *
   * If this function is called when the UE is in a situation where entering
   * CONNECTED mode is not possible (e.g. before the simulation begin), then the
   * UE will attempt to connect at the earliest possible time (e.g. after it
   * camps to a suitable cell).
   *
   * Note that this function can only be used in NGC-enabled simulation.
   */
  void Attach (NetDeviceContainer1 ueDevices);

  /**
   * \brief Enables automatic attachment of a UE device to a suitable cell
   *        using Idle mode initial cell selection procedure.
   * \param ueDevice the UE device to be attached
   *
   * By calling this, the UE will start the initial cell selection procedure at
   * the beginning of simulation. In addition, the function also instructs the
   * UE to immediately enter CONNECTED mode and activates the default EPS
   * bearer.
   *
   * If this function is called when the UE is in a situation where entering
   * CONNECTED mode is not possible (e.g. before the simulation begin), then the
   * UE will attempt to connect at the earliest possible time (e.g. after it
   * camps to a suitable cell).
   *
   * Note that this function can only be used in NGC-enabled simulation.
   */
  void Attach (Ptr<NetDevice1> ueDevice);

  /**
   * \brief Manual attachment of a set of UE devices to the network via a given
   *        eNodeB.
   * \param ueDevices the set of UE devices to be attached
   * \param enbDevice the destination eNodeB device
   *
   * In addition, the function also instructs each UE to immediately enter
   * CONNECTED mode and activates the default EPS bearer.
   *
   * The function can be used in both NR-only and NGC-enabled simulations.
   * Note that this function will disable Idle mode initial cell selection
   * procedure.
   */
  void Attach (NetDeviceContainer1 ueDevices, Ptr<NetDevice1> enbDevice);

  /**
   * \brief Manual attachment of a UE device to the network via a given eNodeB.
   * \param ueDevice the UE device to be attached
   * \param enbDevice the destination eNodeB device
   *
   * In addition, the function also instructs the UE to immediately enter
   * CONNECTED mode and activates the default EPS bearer.
   *
   * The function can be used in both NR-only and NGC-enabled simulations.
   * Note that this function will disable Idle mode initial cell selection
   * procedure.
   */
  void Attach (Ptr<NetDevice1> ueDevice, Ptr<NetDevice1> enbDevice);

  /** 
   * \brief Manual attachment of a set of UE devices to the network via the
   *        closest eNodeB (with respect to distance) among those in the set.
   * \param ueDevices the set of UE devices to be attached
   * \param enbDevices the set of eNodeB devices to be considered
   * 
   * This function finds among the eNodeB set the closest eNodeB for each UE,
   * and then invokes manual attachment between the pair.
   * 
   * Users are encouraged to use automatic attachment (Idle mode cell selection)
   * instead of this function.
   * 
   * \sa NrHelper1::Attach(NetDeviceContainer1 ueDevices);
   */
  void AttachToClosestEnb (NetDeviceContainer1 ueDevices, NetDeviceContainer1 enbDevices);

  /** 
   * \brief Manual attachment of a UE device to the network via the closest
   *        eNodeB (with respect to distance) among those in the set.
   * \param ueDevice the UE device to be attached
   * \param enbDevices the set of eNodeB devices to be considered
   *
   * This function finds among the eNodeB set the closest eNodeB for the UE,
   * and then invokes manual attachment between the pair.
   * 
   * Users are encouraged to use automatic attachment (Idle mode cell selection)
   * instead of this function.
   *
   * \sa NrHelper1::Attach(Ptr<NetDevice> ueDevice);
   */
  void AttachToClosestEnb (Ptr<NetDevice1> ueDevice, NetDeviceContainer1 enbDevices);

  /**
   * Activate a dedicated EPS bearer on a given set of UE devices.
   *
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer
   */
  uint8_t ActivateDedicatedEpsBearer (NetDeviceContainer1 ueDevices, EpsBearer bearer, Ptr<NgcTft> tft);

  /**
   * Activate a dedicated EPS bearer on a given UE device.
   *
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer.
   */
  uint8_t ActivateDedicatedEpsBearer (Ptr<NetDevice1> ueDevice, EpsBearer bearer, Ptr<NgcTft> tft);

  /**
   *  \brief Manually trigger dedicated bearer de-activation at specific simulation time
   *  \param ueDevice the UE on which dedicated bearer to be de-activated must be of the type NrUeNetDevice
   *  \param enbDevice eNB, must be of the type NrEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  \warning Requires the use of NGC mode. See SetNgcHelper() method.
   */

  void DeActivateDedicatedEpsBearer (Ptr<NetDevice1> ueDevice, Ptr<NetDevice1> enbDevice, uint8_t bearerId);
  /**
   * Create an X2 interface between all the eNBs in a given set.
   *
   * \param enbNodes the set of eNB nodes
   */
  void AddX2Interface (NodeContainer enbNodes);

  /**
   * Create an X2 interface between two eNBs.
   *
   * \param enbNode1 one eNB of the X2 interface
   * \param enbNode2 the other eNB of the X2 interface
   */
  void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);

  /**
   * Manually trigger an X2-based handover.
   *
   * \param hoTime when the handover shall be initiated
   * \param ueDev the UE that hands off, must be of the type NrUeNetDevice
   * \param sourceEnbDev source eNB, must be of the type NrEnbNetDevice
   *                     (originally the UE is attached to this eNB)
   * \param targetEnbDev target eNB, must be of the type NrEnbNetDevice
   *                     (the UE would be connected to this eNB after the
   *                     handover)
   *
   * \warning Requires the use of NGC mode. See SetNgcHelper() method
   */
  void HandoverRequest (Time hoTime, Ptr<NetDevice1> ueDev,
                        Ptr<NetDevice1> sourceEnbDev, Ptr<NetDevice1> targetEnbDev);


  /** 
   * Activate a Data Radio Bearer on a given UE devices (for NR-only simulation).
   * 
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   */
  void ActivateDataRadioBearer (NetDeviceContainer1 ueDevices,  EpsBearer bearer);

  /** 
   * Activate a Data Radio Bearer on a UE device (for NR-only simulation).
   * This method will schedule the actual activation
   * the bearer so that it happens after the UE got connected.
   * 
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   */
  void ActivateDataRadioBearer (Ptr<NetDevice1> ueDevice,  EpsBearer bearer);

  /** 
   * Set the type of fading model to be used in both DL and UL.
   * 
   * \param type type of fading model, must be a type name of any class
   *             inheriting from ns3::SpectrumPropagationLossModel, for
   *             example: "ns3::NrTraceFadingLossModel"
   */
  void SetFadingModel (std::string type);

  /**
   * Set an attribute for the fading model to be created (both DL and UL).
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetFadingModelAttribute (std::string n, const AttributeValue &v);

  /**
   * Enables full-blown logging for major components of the LENA architecture.
   */
  void EnableLogComponents (void);

  /**
   * Enables trace sinks for PHY, MAC, RLC and PDCP. To make sure all nodes are
   * traced, traces should be enabled once all UEs and eNodeBs are in place and
   * connected, just before starting the simulation.
   */
  void EnableTraces (void);

  /**
   * Enable trace sinks for PHY layer.
   */
  void EnablePhyTraces (void);

  /**
   * Enable trace sinks for DL PHY layer.
   */
  void EnableDlPhyTraces (void);

  /**
   * Enable trace sinks for UL PHY layer.
   */
  void EnableUlPhyTraces (void);

  /**
   * Enable trace sinks for DL transmission PHY layer.
   */
  void EnableDlTxPhyTraces (void);

  /**
   * Enable trace sinks for UL transmission PHY layer.
   */
  void EnableUlTxPhyTraces (void);

  /**
   * Enable trace sinks for DL reception PHY layer.
   */
  void EnableDlRxPhyTraces (void);

  /**
   * Enable trace sinks for UL reception PHY layer.
   */
  void EnableUlRxPhyTraces (void);

  /**
   * Enable trace sinks for MAC layer.
   */
  void EnableMacTraces (void);

  /**
   * Enable trace sinks for DL MAC layer.
   */
  void EnableDlMacTraces (void);

  /**
   * Enable trace sinks for UL MAC layer.
   */
  void EnableUlMacTraces (void);

  /**
   * Enable trace sinks for RLC layer.
   */
  void EnableRlcTraces (void);

  /** 
   * 
   * \return the RLC stats calculator object
   */
  Ptr<NrRadioBearerStatsCalculator> GetRlcStats (void);

  /**
   * Enable trace sinks for PDCP layer
   */
  void EnablePdcpTraces (void);

  /** 
   * 
   * \return the PDCP stats calculator object
   */
  Ptr<NrRadioBearerStatsCalculator> GetPdcpStats (void);

  /**
   * Assign a fixed random variable stream number to the random variables used.
   *
   * The InstallEnbDevice() or InstallUeDevice method should have previously
   * been called by the user on the given devices.
   *
   * If NrTraceFadingLossModel has been set as the fading model type, this method
   * will also assign a stream number to it, if none has been assigned before.
   *
   * \param c NetDeviceContainer1 of the set of net devices for which the
   *          NrNetDevice should be modified to use a fixed stream
   * \param stream first stream index to use
   * \return the number of stream indices (possibly zero) that have been assigned
  */
  int64_t AssignStreams (NetDeviceContainer1 c, int64_t stream);

protected:
  // inherited from Object
  virtual void DoInitialize (void);

private:
  /**
   * Create an eNodeB device (NrEnbNetDevice) on the given node.
   * \param n the node where the device is to be installed
   * \return pointer to the created device
   */
  Ptr<NetDevice1> InstallSingleEnbDevice (Ptr<Node> n);

  /**
   * Create a UE device (NrUeNetDevice) on the given node
   * \param n the node where the device is to be installed
   * \return pointer to the created device
   */
  Ptr<NetDevice1> InstallSingleUeDevice (Ptr<Node> n);

  /**
   * The actual function to trigger a manual handover.
   * \param ueDev the UE that hands off, must be of the type NrUeNetDevice
   * \param sourceEnbDev source eNB, must be of the type NrEnbNetDevice
   *                     (originally the UE is attached to this eNB)
   * \param targetEnbDev target eNB, must be of the type NrEnbNetDevice
   *                     (the UE would be connected to this eNB after the
   *                     handover)
   *
   * This method is normally scheduled by HandoverRequest() to run at a specific
   * time where a manual handover is desired by the simulation user.
   */
  void DoHandoverRequest (Ptr<NetDevice1> ueDev,
                          Ptr<NetDevice1> sourceEnbDev,
                          Ptr<NetDevice1> targetEnbDev);


  /**
   *  \brief The actual function to trigger a manual bearer de-activation
   *  \param ueDevice the UE on which bearer to be de-activated must be of the type NrUeNetDevice
   *  \param enbDevice eNB, must be of the type NrEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  This method is normally scheduled by DeActivateDedicatedEpsBearer() to run at a specific
   *  time when a manual bearer de-activation is desired by the simulation user.
   */
  void DoDeActivateDedicatedEpsBearer (Ptr<NetDevice1> ueDevice, Ptr<NetDevice1> enbDevice, uint8_t bearerId);


  /// The downlink NR channel used in the simulation.
  Ptr<SpectrumChannel> m_downlinkChannel;
  /// The uplink NR channel used in the simulation.
  Ptr<SpectrumChannel> m_uplinkChannel;
  /// The path loss model used in the downlink channel.
  Ptr<Object> m_downlinkPathlossModel;
  /// The path loss model used in the uplink channel.
  Ptr<Object> m_uplinkPathlossModel;

  /// Factory of MAC scheduler object.
  ObjectFactory m_schedulerFactory;
  /// Factory of FFR (frequency reuse) algorithm object.
  ObjectFactory m_ffrAlgorithmFactory;
  /// Factory of handover algorithm object.
  ObjectFactory m_handoverAlgorithmFactory;
  /// Factory of NrEnbNetDevice objects.
  ObjectFactory m_enbNetDeviceFactory;
  /// Factory of antenna object for eNodeB.
  ObjectFactory m_enbAntennaModelFactory;
  /// Factory for NrUeNetDevice objects.
  ObjectFactory m_ueNetDeviceFactory;
  /// Factory of antenna object for UE.
  ObjectFactory m_ueAntennaModelFactory;
  /// Factory of path loss model object for the downlink channel.
  ObjectFactory m_dlPathlossModelFactory;
  /// Factory of path loss model object for the uplink channel.
  ObjectFactory m_ulPathlossModelFactory;
  /// Factory of both the downlink and uplink NR channels.
  ObjectFactory m_channelFactory;

  /// Name of fading model type, e.g., "ns3::NrTraceFadingLossModel".
  std::string m_fadingModelType;
  /// Factory of fading model object for both the downlink and uplink channels.
  ObjectFactory m_fadingModelFactory;
  /// The fading model used in both the downlink and uplink channels.
  Ptr<SpectrumPropagationLossModel> m_fadingModule;
  /**
   * True if a random variable stream number has been assigned for the fading
   * model. Used to prevent such assignment to be done more than once.
   */
  bool m_fadingStreamsAssigned;

  /// Container of PHY layer statistics.
  Ptr<NrPhyStatsCalculator> m_phyStats;
  /// Container of PHY layer statistics related to transmission.
  Ptr<NrPhyTxStatsCalculator> m_phyTxStats;
  /// Container of PHY layer statistics related to reception.
  Ptr<NrPhyRxStatsCalculator> m_phyRxStats;
  /// Container of MAC layer statistics.
  Ptr<NrMacStatsCalculator> m_macStats;
  /// Container of RLC layer statistics.
  Ptr<NrRadioBearerStatsCalculator> m_rlcStats;
  /// Container of PDCP layer statistics.
  Ptr<NrRadioBearerStatsCalculator> m_pdcpStats;
  /// Connects RLC and PDCP statistics containers to appropriate trace sources
  NrRadioBearerStatsConnector m_radioBearerStatsConnector;

  /**
   * Helper which provides implementation of core network. Initially empty
   * (i.e., NR-only simulation without any core network) and then might be
   * set using SetNgcHelper().
   */
  Ptr<NgcHelper1> m_ngcHelper;

  /**
   * Keep track of the number of IMSI allocated. Increases by one every time a
   * new UE is installed (by InstallSingleUeDevice()). The first UE will have
   * an IMSI of 1. The maximum number of UE is 2^64 (~4.2e9).
   */
  uint64_t m_imsiCounter;
  /**
   * Keep track of the number of cell ID allocated. Increases by one every time
   * a new eNodeB is installed (by InstallSingleEnbDevice()). The first eNodeB
   * will have a cell ID of 1. The maximum number of eNodeB is 65535.
   */
  uint16_t m_cellIdCounter;

  /**
   * The `UseIdealRrc` attribute. If true, NrRrcProtocolIdeal will be used for
   * RRC signaling. If false, NrRrcProtocolReal will be used.
   */
  bool m_useIdealRrc;
  /**
   * The `AnrEnabled` attribute. Activate or deactivate Automatic Neighbour
   * Relation function.
   */
  bool m_isAnrEnabled;
  /**
   * The `UsePdschForCqiGeneration` attribute. If true, DL-CQI will be
   * calculated from PDCCH as signal and PDSCH as interference. If false,
   * DL-CQI will be calculated from PDCCH as signal and PDCCH as interference.
   */
  bool m_usePdschForCqiGeneration;

}; // end of `class NrHelper1`


} // namespace ns3



#endif // NR_HELPER_H
