
bin_PROGRAMS = LT_PMBusApp
LT_PMBusApp_SOURCES = LT_PMBusApp.cpp LT_PMBus.cpp LT_SMBus.cpp LT_SMBusBase.cpp LT_SMBusPec.cpp LT_SMBusNoPec.cpp LT_SMBusGroup.cpp LT_PMBusSpeedTest.cpp LT_PMBusMath.cpp LT_Exception.cpp LT_FaultLog.cpp LT_3880FaultLog.cpp LT_3882FaultLog.cpp LT_3883FaultLog.cpp LT_3884FaultLog.cpp LT_3886FaultLog.cpp LT_3887FaultLog.cpp LT_3889FaultLog.cpp LT_7880FaultLog.cpp LT_2972FaultLog.cpp LT_2974FaultLog.cpp LT_2975FaultLog.cpp LT_2977FaultLog.cpp LT_2978FaultLog.cpp main_record_processor.cpp LT_Nvm.cpp nvm_data_helpers.cpp hex_file_parser.cpp httoi.cpp LT_PMBusDetect.cpp LT_PMBusDevice.cpp LT_PMBusDeviceLTC2972.cpp LT_PMBusDeviceLTC2974.cpp LT_PMBusDeviceLTC2975.cpp LT_PMBusDeviceLTC2977.cpp LT_PMBusDeviceLTC2978.cpp LT_PMBusRail.cpp LT_PMBusDeviceLTC2980.cpp LT_PMBusDeviceLTC3880.cpp LT_PMBusDeviceLTC3882.cpp LT_PMBusDeviceLTC3883.cpp LT_PMBusDeviceLTC3884.cpp LT_PMBusDeviceLTC3886.cpp LT_PMBusDeviceLTC3887.cpp LT_PMBusDeviceLTC3889.cpp LT_PMBusDeviceLTC7880.cpp LT_PMBusDeviceLTM2987.cpp  LT_PMBusDeviceLTM4664.cpp LT_PMBusDeviceLTM4675.cpp LT_PMBusDeviceLTM4676.cpp LT_PMBusDeviceLTM4677.cpp LT_PMBusDeviceLTM4678.cpp LT_PMBusDeviceLTM4680.cpp LT_PMBusDeviceLTM4686.cpp LT_PMBusDeviceLTM4700.cpp

# Add this for dmalloc
# -I../dmalloc-5.5.2 -DDMALLOC
AM_CXXFLAGS = @LT_PMBusApp_CFLAGS@
# ../dmalloc-5.5.2/libdmallocxx.a
AM_LDFLAGS = @LT_PMBusApp_LIBS@ -li2c



CLEANFILES = *~
