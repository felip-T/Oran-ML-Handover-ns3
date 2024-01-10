#ifndef ENBREPORTERAVGSINR_H
#define ENBREPORTERAVGSINR_H
#include <ns3/oran-module.h>
#include "oran-report-sinr-enb.h"
#include "oran-report-sinr-ue.h"
#include <ns3/log.h>
#include <ns3/double.h>

namespace ns3 {
class EnbReporterAvgSinr : public OranReporter
{
    public:
    static TypeId GetTypeId();

    EnbReporterAvgSinr();
    ~EnbReporterAvgSinr() override;

    void UpdateData(double sinr, double rsrp, uint64_t rnti);
    std::vector<Ptr<OranReport>> GenerateReports() override;

    private:
    std::vector<std::tuple<double, uint64_t>>  m_sinr;
    std::vector<std::tuple<double, uint64_t>>  m_rsrp;
    DoubleValue m_sinrAvg;
    DoubleValue m_rsrpAvg;

    protected:
    std::vector<std::tuple<uint64_t, uint64_t>> GetAllRegisteredUeIds(uint64_t, Ptr<OranModSqlite>);

};
}
#endif
