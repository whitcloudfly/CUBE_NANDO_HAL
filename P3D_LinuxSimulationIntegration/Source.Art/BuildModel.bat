

SET XtoMdlPath=..\..\..\Modeling\3ds Max\3DSM2017_x64\Plugins
SET ModelSrcPath=
SET ModelSrc=LinuxSimulation.x
SET ModelDest=..\Content\SimObjects\LinuxSimulation\model\LinuxSimulation.mdl
SET ModelDefPath=modeldef.xml

"%XtoMdlPath%\XtoMDL.exe" /DICT:"%ModelDefPath%" /XANIM /OUT:"%ModelDest%" "%ModelSrc%"