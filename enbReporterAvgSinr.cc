#include "enbReporterAvgSinr.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("EnbReporterAvgSinr");
NS_OBJECT_ENSURE_REGISTERED(EnbReporterAvgSinr);

TypeId
EnbReporterAvgSinr::GetTypeId()
{
    static TypeId tid = TypeId("ns3::EnbReporterAvgSinr")
                            .SetParent<OranReporter>()
                            .AddConstructor<EnbReporterAvgSinr>();

    return tid;
}

EnbReporterAvgSinr::EnbReporterAvgSinr()
    : OranReporter()
{
}

EnbReporterAvgSinr::~EnbReporterAvgSinr()
{
}

void
EnbReporterAvgSinr::UpdateData(double sinr, double rsrp, uint64_t rnti)
{
    NS_LOG_FUNCTION(this << rsrp << sinr);
    auto it =
        std::find_if(m_sinr.begin(), m_sinr.end(), [rnti](const std::tuple<double, uint64_t>& t) {
            return std::get<1>(t) == rnti;
        });
    double dbSinr = 10 * std::log10(sinr);
    double dbRsrp = 10 * std::log10(rsrp);
    if (it == m_sinr.end())
    {
        m_sinr.emplace_back(dbSinr, rnti);
        m_rsrp.emplace_back(dbRsrp, rnti);
    }
    else
    {
        std::get<0>(*it) = dbSinr;
        std::get<0>(*(m_rsrp.begin() + std::distance(m_sinr.begin(), it))) = dbRsrp;
    }
    // std::cout << "RNTI: " << rnti << " SINR: " << dbSinr << " RSRP: " << dbRsrp << std::endl;
}

std::vector<Ptr<OranReport>>
EnbReporterAvgSinr::GenerateReports()
{
    std::vector<Ptr<OranReport>> reports;
    if (m_active)
    {
        NS_ABORT_MSG_IF(m_terminator == nullptr,
                        "Attempting to generate reports in reporter with NULL E2 Terminator");

        Ptr<OranModSqlite> data = DynamicCast<OranModSqlite>(m_terminator->GetNearRtRic()->Data());
        m_sinrAvg = 0;
        m_rsrpAvg = 0;
        uint16_t cellId = std::get<1>(data->GetLteEnbCellInfo(m_terminator->GetE2NodeId()));
        std::vector<std::tuple<uint64_t, uint64_t>> connectedUes =
            GetAllRegisteredUeIds(cellId, data);
        {
            for (auto ue : connectedUes)
            {
                std::cout << "---> CellId " << cellId << ' ' << std::get<0>(ue) << ' '
                          << std::get<1>(ue) << std::endl;

                double ueSinr = double(std::get<0>(
                    *(m_sinr.begin() +
                      std::distance(connectedUes.begin(),
                                    std::find_if(connectedUes.begin(),
                                                 connectedUes.end(),
                                                 [ue](const std::tuple<uint64_t, uint64_t>& t) {
                                                     return std::get<1>(t) == std::get<1>(ue);
                                                 })))));
                double ueRsrp = double(std::get<0>(
                    *(m_rsrp.begin() +
                      std::distance(connectedUes.begin(),
                                    std::find_if(connectedUes.begin(),
                                                 connectedUes.end(),
                                                 [ue](const std::tuple<uint64_t, uint64_t>& t) {
                                                     return std::get<1>(t) == std::get<1>(ue);
                                                 })))));

                uint64_t rnti = std::get<1>(ue);
                uint64_t ueNodeId = data->GetLteUeE2NodeIdFromCellInfo(cellId, rnti);
                Ptr<OranReportSinrUeSql> sinrUeReport = CreateObject<OranReportSinrUeSql>();
                sinrUeReport->SetAttribute("ReporterE2NodeId", UintegerValue(m_terminator->GetE2NodeId()));
                sinrUeReport->SetAttribute("ueid", UintegerValue(ueNodeId));
                sinrUeReport->SetAttribute("SINR", DoubleValue(ueSinr));
                sinrUeReport->SetAttribute("RSRP", DoubleValue(ueRsrp));
                sinrUeReport->SetAttribute("Time", TimeValue(Simulator::Now()));

                reports.emplace_back(sinrUeReport);

                m_sinrAvg = m_sinrAvg.Get() + ueSinr;
                m_rsrpAvg = m_rsrpAvg.Get() + ueRsrp;
            }
        }
        std::cout << "Avg SINR: " << m_sinrAvg.Get() << std::endl;
        std::cout << "Avg RSRP: " << m_rsrpAvg.Get() << std::endl;
        std::cout << "Avg RSRP: " << cellId << std::endl;

        Ptr<OranReportSinrEnbSql> sinrReport = CreateObject<OranReportSinrEnbSql>();
        sinrReport->SetAttribute("ReporterE2NodeId", UintegerValue(m_terminator->GetE2NodeId()));
        sinrReport->SetAttribute("CellId", UintegerValue(cellId));
        sinrReport->SetAttribute("SINR", m_sinrAvg);
        sinrReport->SetAttribute("RSRP", m_rsrpAvg);
        sinrReport->SetAttribute("Time", TimeValue(Simulator::Now()));

        reports.emplace_back(sinrReport);
    }
    return reports;
}

std::vector<std::tuple<uint64_t, uint64_t>>
EnbReporterAvgSinr::GetAllRegisteredUeIds(uint64_t cellId, Ptr<OranModSqlite> db)
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
} // namespace ns3
