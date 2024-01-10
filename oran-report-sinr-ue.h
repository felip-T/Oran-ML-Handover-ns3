#ifndef ORAN_REPORT_SINR_UE_SQL_H
#define ORAN_REPORT_SINR_UE_SQL_H

#include <ns3/oran-report-sql.h>

#include <ns3/object.h>
#include <ns3/vector.h>

#include <string>

namespace ns3
{
class OranReportSinrUeSql : public OranReportSql
{
  public:
    static TypeId GetTypeId();
    OranReportSinrUeSql();
    ~OranReportSinrUeSql() override;
    std::string ToString() const override;
    std::vector<std::tuple<std::string, std::string>> GetTableInfo() override;
    std::string GetTableName() override;

  private:
    uint64_t m_ueId;
    double m_sinr;
    double m_rsrp;

  public:
    double GetSinr() const;
    double GetRsrp() const;
};

} // namespace ns3

#endif
