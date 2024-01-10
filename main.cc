#include "custrep.h"
#include "enbReporterAvgSinr.h"

#include <ns3/buildings-module.h>
#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/oran-module.h>

#include <string>

using namespace ns3;

void
NotifyUeReport(std::vector<Ptr<OranE2NodeTerminator>> reps,
               Ptr<OranModSqlite> db,
               uint16_t cellId,
               uint16_t rnti,
               double rsrp,
               double sinr,
               uint8_t componentCarrierId)
{
    uint64_t id = db->GetLteUeE2NodeIdFromCellInfo(cellId, rnti);
    auto it = std::find_if(reps.begin(), reps.end(), [id](Ptr<OranE2NodeTerminator> r) {
        return r->GetE2NodeId() == id;
    });
    if (id != 0)
    {
        std::vector<Ptr<OranReporter>> reporters = (*it)->GetReporters();
        Ptr<custRep> rep = DynamicCast<custRep>(
            *std::find_if(reporters.begin(), reporters.end(), [](Ptr<OranReporter> r) {
                return r->GetInstanceTypeId().GetName() == "ns3::custRep";
            }));
        rep->UpdateData(sinr, rsrp);
    }
}

void
NotifyEnbReport(std::vector<Ptr<OranE2NodeTerminator>> reps,
                Ptr<OranModSqlite> db,
                uint16_t cellId,
                uint16_t rnti,
                double rsrp,
                double sinr,
                uint8_t componentCarrierId)
{
    // for (auto& i : reps)
    // {
    //     std::cout << "----->" << i->GetE2NodeId() << std::endl;
    // }
    auto it = std::find_if(reps.begin(), reps.end(), [cellId](Ptr<OranE2NodeTerminator> r) {
        return r->GetE2NodeId() == cellId;
    });
    if (it != reps.end())
    {
        std::vector<Ptr<OranReporter>> reporters = (*it)->GetReporters();
        Ptr<EnbReporterAvgSinr> rep = DynamicCast<EnbReporterAvgSinr>(
            *std::find_if(reporters.begin(), reporters.end(), [](Ptr<OranReporter> r) {
                return r->GetInstanceTypeId().GetName() == "ns3::EnbReporterAvgSinr";
            }));
        rep->UpdateData(sinr, rsrp, rnti);
    }
    // std::cout << "Enb Report" << std::endl;
    // std::cout << "cellId: " << cellId << std::endl;
    // std::cout << "rnti: " << rnti << std::endl;
    // std::cout << "rsrp: " << rsrp << std::endl;
    // std::cout << "sinr: " << sinr << std::endl;
    // std::cout << "componentCarrierId: " << componentCarrierId << std::endl;
    // std::cout << Simulator::Now().GetSeconds() <<'\t' << std::endl;
}

void
readReport(std::vector<std::tuple<std::string, std::string>> report)
{
    for (auto& i : report)
    {
        std::cout << std::get<0>(i) << ' ' << std::get<1>(i) << std::endl;
    }
}

std::vector<std::tuple<uint64_t, uint64_t>>
GetAllRegisteredUeIds(uint64_t cellId, Ptr<OranModSqlite> db)
{
    uint64_t cellIdUe;
    uint64_t rnti;
    std::vector<std::tuple<uint64_t, uint64_t>> ids;
    for (auto ueId : db->GetLteUeE2NodeIds())
    {
        bool found;
        std::tie(found, cellIdUe, rnti) = db->GetLteUeCellInfo(ueId);
        if (found && cellIdUe == cellId)
        {
            ids.emplace_back(ueId, rnti);
        }
    }

    return ids;
}

void
showPosition(Ptr<MobilityModel> mm, int i)
{
    Vector pos = mm->GetPosition();
    std::cout << "Node " << i << " is at " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
}

void
QueryRcSink(std::string query, std::string args, int rc)
{
    std::cout << Simulator::Now().GetSeconds() << " Query "
              << ((rc == SQLITE_OK || rc == SQLITE_DONE) ? "OK" : "ERROR") << "(" << rc << "): \""
              << query << "\"";

    if (!args.empty())
    {
        std::cout << " (" << args << ")";
    }
    std::cout << std::endl;
}

int
main(int argc, char* argv[])
{
    unsigned numberOfBuildings = 9;
    std::string dbFileName = "oran-repository.db";

    // Create nodes
    NodeContainer ueNodes;
    NodeContainer enbNodes;

    // Create buildings
    Ptr<GridBuildingAllocator> buildings = CreateObject<GridBuildingAllocator>();
    buildings->SetAttribute("GridWidth", UintegerValue(3));
    buildings->SetAttribute("LengthX", DoubleValue(100));
    buildings->SetAttribute("LengthY", DoubleValue(100));
    buildings->SetAttribute("DeltaX", DoubleValue(7));
    buildings->SetAttribute("DeltaY", DoubleValue(7));
    buildings->SetAttribute("Height", DoubleValue(6));
    buildings->SetBuildingAttribute("NRoomsX", UintegerValue(2));
    buildings->SetBuildingAttribute("NRoomsY", UintegerValue(4));
    buildings->SetBuildingAttribute("NFloors", UintegerValue(2));
    buildings->SetAttribute("MinX", DoubleValue(0));
    buildings->SetAttribute("MinY", DoubleValue(0));

    BuildingContainer bContainer = buildings->Create(numberOfBuildings);
    double bounds = 100 * (numberOfBuildings / 3) + 7 * (numberOfBuildings / (3 - 2));

    // Create mobility models
    MobilityHelper mobilityUe;
    mobilityUe.SetMobilityModel("ns3::RandomWalk2dOutdoorMobilityModel",
                                "Bounds",
                                RectangleValue(Rectangle(0, bounds, 0, bounds)));
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(101, 101, 1.5));
    positionAlloc->Add(Vector(101, 201, 1.5));
    positionAlloc->Add(Vector(205, 101, 1.5));
    mobilityUe.SetPositionAllocator(positionAlloc);
    ueNodes.Create(3);
    mobilityUe.Install(ueNodes);

    MobilityHelper mobilityEnb;
    ns3::Ptr<ns3::ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator>();
    positionAllocEnb->Add(Vector(90, 90, 4));
    positionAllocEnb->Add(Vector(120, 200, 4));
    mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityEnb.SetPositionAllocator(positionAllocEnb);
    enbNodes.Create(2);
    mobilityEnb.Install(enbNodes);

    // Install LTE
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm");
    lteHelper->SetAttribute("PathlossModel",
                            StringValue("ns3::HybridBuildingsPropagationLossModel"));
    lteHelper->SetPathlossModelAttribute("ShadowSigmaExtWalls", DoubleValue(5));
    lteHelper->SetPathlossModelAttribute("ShadowSigmaOutdoor", DoubleValue(7));
    lteHelper->SetPathlossModelAttribute("ShadowSigmaIndoor", DoubleValue(7));

    showPosition(ueNodes.Get(0)->GetObject<MobilityModel>(), 0);
    showPosition(enbNodes.Get(0)->GetObject<MobilityModel>(), 1);

    BuildingsHelper::Install(ueNodes);
    BuildingsHelper::Install(enbNodes);

    // setup oran
    Ptr<OranHelper> oranHelper = CreateObject<OranHelper>();
    oranHelper->SetAttribute("Verbose", BooleanValue(true));
    oranHelper->SetAttribute("LmQueryInterval", TimeValue(Seconds(5)));
    oranHelper->SetAttribute("E2NodeInactivityThreshold", TimeValue(Seconds(2)));
    oranHelper->SetAttribute("E2NodeInactivityIntervalRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=2]"));
    oranHelper->SetAttribute("LmQueryMaxWaitTime",
                             TimeValue(Seconds(0))); // 0 means wait for all LMs to finish
    oranHelper->SetAttribute("LmQueryLateCommandPolicy", EnumValue(OranNearRtRic::DROP));
    oranHelper->SetAttribute("RicTransmissionDelayRv",
                             StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    if (!dbFileName.empty())
    {
        std::remove(dbFileName.c_str());
    }

    oranHelper->SetDataRepository("ns3::OranModSqlite", "DatabaseFile", StringValue(dbFileName));

    Ptr<OranNearRtRic> nearRtRic = oranHelper->CreateNearRtRic();

    // ue E2 setup
    OranE2NodeTerminatorContainer e2NodeTerminatorsUe;

    oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorLteUe",
                                    "RegistrationIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "TransmissionDelayRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    oranHelper->AddReporter("ns3::OranReporterLteUeCellInfo",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerLteUeHandover[InitialReport=true]"));

    // oranHelper->AddReporter("ns3::custRep",
    //                         "Trigger",
    //                         StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminatorsUe.Add(oranHelper->DeployTerminators(nearRtRic, ueNodes));

    // enb E2 setup
    OranE2NodeTerminatorContainer e2NodeTerminatorsEnbs;
    oranHelper->SetE2NodeTerminator("ns3::OranE2NodeTerminatorLteEnb",
                                    "RegistrationIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "SendIntervalRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                                    "TransmissionDelayRv",
                                    StringValue("ns3::ConstantRandomVariable[Constant=0.001]"));

    oranHelper->AddReporter("ns3::EnbReporterAvgSinr",
                            "Trigger",
                            StringValue("ns3::OranReportTriggerPeriodic"));

    e2NodeTerminatorsEnbs.Add(oranHelper->DeployTerminators(nearRtRic, enbNodes));

    // Create Devices and install them in the Nodes (eNB and UE)
    NetDeviceContainer enbDevs;
    NetDeviceContainer ueDevs;
    enbDevs = lteHelper->InstallEnbDevice(enbNodes);
    ueDevs = lteHelper->InstallUeDevice(ueNodes);

    // Attach a UE to a eNB
    lteHelper->AttachToClosestEnb(ueDevs, enbDevs);

    // Activate an EPS bearer
    EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

    lteHelper->EnablePhyTraces();

    nearRtRic->Data()->TraceConnectWithoutContext("QueryRc", MakeCallback(&QueryRcSink));

    std::vector<Ptr<OranE2NodeTerminator>> ueTerminators;
    for (size_t i = 0; i < e2NodeTerminatorsUe.GetN(); i++)
    {
        ueTerminators.emplace_back(e2NodeTerminatorsUe.Get(i));
    }

    std::vector<Ptr<OranE2NodeTerminator>> enbTerminators;
    for (size_t i = 0; i < e2NodeTerminatorsEnbs.GetN(); i++)
    {
        enbTerminators.emplace_back(e2NodeTerminatorsEnbs.Get(i));
    }

    Ptr<OranModSqlite> db = DynamicCast<OranModSqlite>(nearRtRic->Data());

    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/"
                                  "ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr",
                                  MakeBoundCallback(&NotifyEnbReport, enbTerminators, db));

    Simulator::Schedule(Seconds(1), &OranHelper::ActivateAndStartNearRtRic, oranHelper, nearRtRic);
    Simulator::Schedule(Seconds(1.5),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsEnbs);
    Simulator::Schedule(Seconds(2),
                        &OranHelper::ActivateE2NodeTerminators,
                        oranHelper,
                        e2NodeTerminatorsUe);
    for (int i = 0; i < 10; i++)
    {
        Simulator::Schedule(Seconds(i),
                            &showPosition,
                            ueNodes.Get(0)->GetObject<MobilityModel>(),
                            0);
        Simulator::Schedule(Seconds(i),
                            &showPosition,
                            ueNodes.Get(1)->GetObject<MobilityModel>(),
                            0);
    }

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    if(db->IsActive()) std::cout << "DB is active" << std::endl;
    else std::cout << "DB is not active" << std::endl;
    auto a = db->GetLastReport("sinrEnb");
    readReport(a);
    // for (auto t : GetAllRegisteredUeIds(2, db)){
    //     std::cout << std::get<0>(t) << ' ' << std::get<1>(t) << std::endl;
    // }
}
