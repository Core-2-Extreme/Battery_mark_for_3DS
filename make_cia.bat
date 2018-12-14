cxitool 3DS_Battery_Checker.3dsx 3DS_Battery_Checker.cxi -b resource/banner.bnr -t 000400000BC9300F -c CTR-B-Checker
makerom -f cia -o 3DS_Battery_Checker.cia -ver 1024 -target t -i 3DS_Battery_Checker.cxi:0:0
pause