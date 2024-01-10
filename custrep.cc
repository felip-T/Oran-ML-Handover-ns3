#include "custrep.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("custRep");
    NS_OBJECT_ENSURE_REGISTERED(custRep);

    TypeId custRep::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::custRep")
                                .SetParent<OranReporter>()
                                .AddConstructor<custRep>();

        return tid;
    }

    custRep::custRep()
        : OranReporter()
    {
    }

    custRep::~custRep()
    {
    }

    void custRep::UpdateData(double sinr, double rsrp)
    {
        NS_LOG_FUNCTION(this << rsrp << sinr);
        m_sinr = sinr;
        m_rsrp = rsrp;
    }

    std::vector<Ptr<OranReport>> custRep::GenerateReports()
    {
        std::vector<Ptr<OranReport>> reports;
        if (m_active)
        {
            NS_ABORT_MSG_IF(m_terminator == nullptr,
                            "Attempting to generate reports in reporter with NULL E2 Terminator");

            Ptr<OranReportSinrSql> sinrReport = CreateObject<OranReportSinrSql>();
            sinrReport->SetAttribute("ReporterE2NodeId",
                                         UintegerValue(m_terminator->GetE2NodeId()));
            sinrReport->SetAttribute("SINR", DoubleValue(m_sinr));
            sinrReport->SetAttribute("RSRP", DoubleValue(m_rsrp));
            sinrReport->SetAttribute("Time", TimeValue(Simulator::Now()));

            reports.emplace_back(sinrReport);
        }
        return reports;
    }
}
