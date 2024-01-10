#include "oran-report-sinr.h"


#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/string.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("OranReportSinrSql");

NS_OBJECT_ENSURE_REGISTERED(OranReportSinrSql);

TypeId
OranReportSinrSql::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OranReportSinrSql")
                            .SetParent<OranReportSql>()
                            .AddConstructor<OranReportSinrSql>()
                            .AddAttribute("SINR",
                                          "Actual SINR",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportSinrSql::m_sinr),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("RSRP",
                                          "Actual RSRP",
                                          DoubleValue(),
                                          MakeDoubleAccessor(&OranReportSinrSql::m_rsrp),
                                          MakeDoubleChecker<double>());

    return tid;
}

OranReportSinrSql::OranReportSinrSql()
    : OranReportSql()
{
    m_tableInfo.emplace_back("RSRP", "REAL");
    m_tableInfo.emplace_back("SINR", "REAL");
    NS_LOG_FUNCTION(this);
}

OranReportSinrSql::~OranReportSinrSql()
{
    NS_LOG_FUNCTION(this);
}

std::string
OranReportSinrSql::ToString() const
{
    NS_LOG_FUNCTION(this);

    std::stringstream ss;
    Time time = GetTime();

    ss << "OranReportSinr("
       << "nodeid=" << GetReporterE2NodeId() << ";time=" << time.GetTimeStep()
       << ";sinr=" << m_sinr << ";rsrp=" << m_rsrp << ")";

    return ss.str();
}

double
OranReportSinrSql::GetSinr() const
{
    NS_LOG_FUNCTION(this);

    return m_sinr;
}

double
OranReportSinrSql::GetRsrp() const
{
    NS_LOG_FUNCTION(this);

    return m_rsrp;
}

std::vector<std::tuple<std::string, std::string>>
OranReportSinrSql::GetTableInfo()
{
    NS_LOG_FUNCTION(this);
    return m_tableInfo;
}
std::string
OranReportSinrSql::GetTableName()
{
    NS_LOG_FUNCTION(this);
    return "sinr";
}
} // namespace ns3
