build_lib_example(
  NAME builds
  SOURCE_FILES main.cc custrep.cc oran-report-sinr.cc enbReporterAvgSinr.cc oran-report-sinr-enb.cc oran-report-sinr-ue.cc
  LIBRARIES_TO_LINK ${libcore} ${liblte} ${libmobility} ${libbuilds} ${liboran}
  )
