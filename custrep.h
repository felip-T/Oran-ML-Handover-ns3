#ifndef CUSTREP_H
#define CUSTREP_H
#include <ns3/oran-module.h>
#include "oran-report-sinr.h"
#include <ns3/log.h>
#include <ns3/double.h>

namespace ns3 {
class custRep : public OranReporter
{
    public:
    static TypeId GetTypeId();

    custRep();
    ~custRep() override;

    void UpdateData(double sinr, double rsrp);
    std::vector<Ptr<OranReport>> GenerateReports() override;

    private:
    double m_sinr;
    double m_rsrp;
};
}
#endif
