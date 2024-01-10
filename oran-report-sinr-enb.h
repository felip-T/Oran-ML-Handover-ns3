#ifndef ORAN_REPORT_SINR_ENB_SQL_H
#define ORAN_REPORT_SINR_ENB_SQL_H

#include <ns3/oran-report-sql.h>

#include <ns3/object.h>
#include <ns3/vector.h>

#include <string>

namespace ns3
{
class OranReportSinrEnbSql : public OranReportSql
{
  public:
    static TypeId GetTypeId();
    OranReportSinrEnbSql();
    ~OranReportSinrEnbSql() override;
    std::string ToString() const override;
    std::vector<std::tuple<std::string, std::string>> GetTableInfo() override;
    std::string GetTableName() override;
    uint64_t GetCellId() const;

  private:
    uint64_t m_cellId;
    double m_sinr;
    double m_rsrp;

  public:
    double GetSinr() const;
    double GetRsrp() const;
}; // class OranReportLocation

} // namespace ns3

#endif
