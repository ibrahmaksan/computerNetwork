#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/lora-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/network-module.h"
#include "ns3/network-server-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rectangle.h"
#include "ns3/string.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/periodic-sender.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "time.h"


using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");


std::vector<int> alinan; // alinan, gonderilen paket, gecikme zamanlari gibi degerleri tuttugum vektorler
std::vector<int> gonderilen;
std::vector<long long> gecikme;
long long gonderilen_zaman;
long long alinan_zaman;

auto packetsSent = std::vector<int>(6, 0);

auto packetsReceived = std::vector<int>(6, 0);



long long what_is_the_time(){ // su anki zamani bulmak icin yazdigim fonksiyon

    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long ms = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000; // su anki zamani ms cinsinden alıyorum
    return ms;
}

/**
 * Record the beginning of a transmission by an end device.
 *
 * \param packet A pointer to the packet sent.
 * \param senderNodeId Node id of the sender end device.
 */


void
OnTransmissionCallback(Ptr<const Packet> packet, uint32_t senderNodeId) 
{
    // Paket gonderiminde tetiklenecek fonksiyon. Bu sayede gonderilen paket sayısını bulabiliyorum.

    NS_LOG_FUNCTION(packet << senderNodeId);
    LoraTag tag;
    packet->PeekPacketTag(tag);
    packetsSent.at(tag.GetSpreadingFactor() - 7)++;
    gonderilen.insert(gonderilen.end(), packetsSent.begin(), packetsSent.end());
    gonderilen_zaman = what_is_the_time();
    std::cout << "gonderilen : " << gonderilen.size()/6 << " zaman : " << gonderilen_zaman << std::endl;
}

/**
 * Record the correct reception of a packet by a gateway.
 *
 * \param packet A pointer to the packet received.
 * \param receiverNodeId Node id of the receiver gateway.
 */
void
OnPacketReceptionCallback(Ptr<const Packet> packet, uint32_t receiverNodeId)
{

    // Basarili paket aliminida tetiklenecek fonksiyon. Bu sayede basariyla alinan paket sayısını bulabiliyorum.
    NS_LOG_FUNCTION(packet << receiverNodeId);
    LoraTag tag;
    packet->PeekPacketTag(tag);
    packetsReceived.at(tag.GetSpreadingFactor() - 7)++;
    alinan.insert(alinan.end(), packetsReceived.begin(), packetsReceived.end());
    alinan_zaman = what_is_the_time();
    gecikme.push_back(alinan_zaman - gonderilen_zaman); // eger basariyla bir paket alindiysa ne kadar geceikme yasadigini buluyorum.
    std::cout << "alinan : " << alinan.size()/6 << " zaman : " << alinan_zaman << std::endl;
}

int main(int argc, char* argv[])
{

    int appPeriodSeconds = 20; // Varsayılan paket gonderim periyot degeri
    int nDevices = 176; // varsayilan endevices sayisi
    std::string outputFolder = "output";  // Varsayılan dosya degeri


    // Konsoldan aldigim degerler
    CommandLine cmd;
    cmd.AddValue("nDevices", "Number of end devices", nDevices);
    cmd.AddValue("appPeriodSeconds", "Period of application packets", appPeriodSeconds);
    cmd.AddValue("OutputFolder", "Output folder", outputFolder);
    cmd.Parse(argc, argv);

    ////////////////////////////// LOG DEGERLERINI GEREKTIGINDE KULLANIYORUM ///////////////////////////
    
//     LogComponentEnable("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
//     LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
//     LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
//     LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
//     LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
//     LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
//     LogComponentEnable("LorawanMac", LOG_LEVEL_ALL);
//     LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
//     LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
//     LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
//     LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
//     LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
//     LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
//     LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
//     LogComponentEnable("LorawanMacHelper", LOG_LEVEL_ALL);
//  // LogComponentEnable("OneShotSenderHelper", LOG_LEVEL_ALL);
//  // LogComponentEnable("OneShotSender", LOG_LEVEL_ALL);
    // LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
    // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
    // LogComponentEnableAll(LOG_PREFIX_FUNC);
    // LogComponentEnableAll(LOG_PREFIX_NODE);
    // LogComponentEnableAll(LOG_PREFIX_TIME);
    // LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_ALL);
    // LogComponentEnable("PeriodicSenderHelper",LOG_LEVEL_ALL);


//////////////////////////////////////////////////////////////////////////////////////////////////////////
    // kablosuz ağ kanalı yaratıyoruz

    NS_LOG_INFO("Creating the channel...");

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76); // paket kayıp exponansiyeli, bu arttıkca basarili gonderim dusuyor
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    LorawanMacHelper macHelper = LorawanMacHelper();

    LoraHelper helper = LoraHelper();
    helper.EnablePacketTracking(); // paket takibini aktifleştirdim.


////////////////////////////////////////////////////////////////////////////////////////////////////

    NS_LOG_INFO("Setting up helpers...");

    // Gateway i sabit ayarlıyoruz. Hareket etmeyecek.
    MobilityHelper mobilityGw;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(2000,2000,20));
    mobilityGw.SetPositionAllocator(allocator);
    mobilityGw.SetMobilityModel("ns3::ConstantPositionMobilityModel"); // Bu sayede gatewayi sabit olarak ayarlamış olduk.

    // gateway'i oluşturuyoruz.
    NS_LOG_INFO("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create(1);// tek bir tane gatewaye ihtiyacımız var.
    mobilityGw.Install(gateways);
    phyHelper.SetDeviceType(LoraPhyHelper::GW);// gatewayin helperları
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // end device yarattık.
    NS_LOG_INFO("Creating the end device...");  // Burada end devicelerin hareketi sumodan gelen ns2mobility.tcl dosyasina gore belirlen
    // mektedir.
    std::string traceFile = "/home/ibrahim/İndirilenler/ns-allinone-3.41/ns-3.41/scratch/ns2mobility.tcl";
    Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile); // Hareketlilik ns2MobilityHelper kütüphanesi ile saglandi.
    NodeContainer endDevices;
    endDevices.Create(nDevices); // end device sayısı
    ns2.Install();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

     // addres çözümleyici ayarlama
    uint8_t nwkId = 54;
    uint32_t nwkAddr = 1864;
    Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator>(nwkId, nwkAddr);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // helperlari end devicelara yukleme
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    macHelper.SetAddressGenerator(addrGen);
    helper.Install(phyHelper, macHelper, endDevices);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7
// end devicelara paket gonderme ozelligini ekleme
    
    PeriodicSenderHelper appHelper = PeriodicSenderHelper(); // periodic olarak paket göndermeye yarıyor.
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    ApplicationContainer appContainer = appHelper.Install(endDevices);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// Bu callbackler sayesinde paket gonderiminde ve paket aliminda bu fonksiyonlar cagrilacaktir.
    // Install trace sources
    for (auto node = gateways.Begin(); node != gateways.End(); node++)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "ReceivedPacket",
            MakeCallback(&OnPacketReceptionCallback));
    }

    // Install trace sources
    for (auto node = endDevices.Begin(); node != endDevices.End(); node++)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "StartSending",
            MakeCallback(&OnTransmissionCallback));
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<int> sfQuantity(6);
    sfQuantity = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);


    Simulator::Stop(Seconds(480)); // sumoda simulasyonun surdugu zamani yaziyoruz.
    Simulator::Run();
    Simulator::Destroy();

    // delayi hesapladigim yer
    long long delayTime = 0;

    for (auto it = gecikme.begin(); it != gecikme.end(); ++it) { // her bir gecikmeyi atip basarili paket sayisina bolersem ortalama
    // gecikmeyi elde etmis olurum.
        delayTime += *it;
    }

    std::cout<<"Paket gonderim basarim orani : %" << 100 * (static_cast<float>(alinan.size())/static_cast<float>(gonderilen.size()))<<std::endl;
    std::cout <<"Gecikme ortalamasi : paket basina " << delayTime/(static_cast<float>(gecikme.size())) << " milliseconds gecikme yasanmaktadir." <<std::endl; 

    std::ofstream outputFile;
    outputFile.open(outputFolder + "/results.txt");
    outputFile << "Paket gonderim basarim orani : %" << 100 * (static_cast<float>(alinan.size()) / static_cast<float>(gonderilen.size())) << std::endl;
    outputFile << "Gecikme ortalamasi : paket basina " << delayTime / (static_cast<float>(gecikme.size())) << " milliseconds gecikme yasanmaktadir." << std::endl;
    outputFile.close();

    return 0;
}