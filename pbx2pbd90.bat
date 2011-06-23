@rem Call pbx2pbd90 with correct errorlevel for visual studio
@rem
@rem unlike newer versions, pbx2pbd90 returns 1 for no error
pbx2pbd90.exe %1 %2
@if errorlevel 1 exit 0
