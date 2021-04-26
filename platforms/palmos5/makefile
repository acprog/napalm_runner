all:
	@"C:\Program Files\Metrowerks\CodeWarrior\Bin\ide.exe" runner.mcp /b /r /q /t Application
	@makeprc arm_db.info
#	@"C:\Palm\Desktop\QuickInstall.exe" arm_db.prc
	@make -C "c:/work/MPP/" -s backup PLATFORM=palmos5
	@cp bins/armc.bin c:/work/mpp/projects/runner/armc.bin
	@cp "C:/Work/MPP/tools/MPP Launcher/bins/palmos5.prc" c:/work/mpp/projects/runner/palmos5.prc
