# For this example, have 'blackmagic' running already. No command line params
# are needed.
!orbtrace -p vtref,3.3 -e vtref,on -Tm
file ofiles/firmware.elf
target extended-remote localhost:2000
set mem inaccessible-by-default off
monitor swdp_scan
attach 1
load

source /usr/local/bin/orbcode/gdbtrace.init

enableSTM32SWO
#          CPU Speed bitrate T M
prepareSWO 72000000 36000000 0 0

dwtSamplePC 1
dwtSyncTap 3
dwtPostTap 1
dwtPostInit 1
dwtPostReset 15
dwtCycEna 1
dwtTraceException 0

ITMId 1
ITMGTSFreq 0
ITMTSPrescale 3
ITMTXEna 1
ITMSYNCEna 1
ITMEna 1


ITMTER 0 0xFFFFFFFF
ITMTPR 0xFFFFFFFF
