#include "oran-report-sinr-enb.h"


#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/string.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportSinrEnbSql");

NS_OBJECT_ENSURE_REGISTERED(OranReportSinrEnbSql);

TypeId
OranReportSinrEnbSql::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReportSinrEnbSql")
                            .SetParent<OranReportSql>()
                            .AddConstructor<OranReportSinrEnbSql>()
                            .AddAttribute("CellId",
                                          "Cell ID of the eNB",
                                          DoubleValue(),
                                          MakeUintegerAccessor(&OranReportSinrEnbSql::m_cellId),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("SINR",
                                          "Avg SINR",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportSinrEnbSql::m_sinr),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("RSRP",
                                          "Avg RSRP",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportSinrEnbSql::m_rsrp),
                                          MakeDoubleChecker<double>());

    return tid;
}

OranReportSinrEnbSql::OranReportSinrEnbSql()
    : OranReportSql()
{
    m_tableInfo.emplace_back("CellId", "INTEGER");
    m_tableInfo.emplace_back("RSRP", "REAL");
    m_tableInfo.emplace_back("SINR", "REAL");
    NS_LOG_FUNCTION(this);
}

OranReportSinrEnbSql::~OranReportSinrEnbSql()
{
    NS_LOG_FUNCTION(this);
}

std::string
OranReportSinrEnbSql::ToString() const
{
    NS_LOG_FUNCTION(this);

    std::stringstream ss;
    Time time = GetTime();

    ss << "OranReportSinrEnb("
       << "nodeid=" << GetReporterE2NodeId() << ";time=" << time.GetTimeStep() << ";cellid=" << m_cellId
       << ";sinr=" << m_sinr << ";rsrp=" << m_rsrp << ")";

    return ss.str();
}

double
OranReportSinrEnbSql::GetSinr() const
{
    NS_LOG_FUNCTION(this);

    return m_sinr;
}

double
OranReportSinrEnbSql::GetRsrp() const
{
    NS_LOG_FUNCTION(this);

    return m_rsrp;
}

uint64_t OranReportSinrEnbSql::GetCellId() const
{
    NS_LOG_FUNCTION(this);

    return m_cellId;
}

std::vector<std::tuple<std::string, std::string>>
OranReportSinrEnbSql::GetTableInfo()
{
    NS_LOG_FUNCTION(this);
    return m_tableInfo;
}
std::string
OranReportSinrEnbSql::GetTableName()
{
    NS_LOG_FUNCTION(this);
    return "sinrEnb";
}
} // namespace ns3
