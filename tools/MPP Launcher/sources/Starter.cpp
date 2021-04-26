/******************************************************************************
 *
 * File: Starter.cpp
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <PCENativeCall.h>
#include <PNOLoader.h>
#include "StarterRsc.h"
#include <MemGlue.h>
#include <VfsMgr.h>
#include <PenInputMgr.h>
#include <STDLIB.H>

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define CREATOR					'ACln'
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

#define	SPLIT_SIZE				0x8000

// Define the minimum OS version we support (5.0 for now).
#define kOurMinVersion	sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static	void LOG(const char *str, int num)
{
	static	char buff[100];
	StrCopy(buff, str);
	StrIToA(buff+StrLen(str), num);
	FrmCustomAlert(InfoMessage, buff, NULL, NULL);
}


#define	ERROR(str) \
{ \
	FrmCustomAlert(InfoMessage, str, NULL, NULL); \
	return -1; \
}



/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags that indicate if the application 
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to 
			// another safe one.
			if (romVersion <= kPalmOS10Version)
				{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
				}
			}
		
		return sysErrRomIncompatible;
		}

	return errNone;
}


//=============================================================================
static Boolean	PalmSGIsTungstenT()
{
	Boolean result 		= false;
	UInt32 	companyFtr 	= 0;
	UInt32	deviceFtr	= 0;
	
	// Check for Tungsten T
	FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &deviceFtr);
	FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyFtr);
	if( companyFtr == 'Palm' && deviceFtr == 'Frg1' )
		result = true;
		
	return result;
}



//=============================================================================
static Boolean PalmSGHasSampledSound()
{
	UInt32	version			= 0;
	Boolean hasSampledSound = false;
	
	// Check if the sound feature is set
	if( FtrGet( sysFileCSoundMgr, sndFtrIDVersion, &version ) == errNone ) {
		hasSampledSound = true;
		goto Done;
	}
	
	if( PalmSGIsTungstenT() )
		hasSampledSound = true;
	
Done:
	return hasSampledSound;
}



//---------------------------------------------------------
// get screen info
struct si
{
	UInt32	width,
			height,
			depths;
	char	*path;
	void	*message_box;
	UInt32	no_sound;
	void	*abort_programm;
}screen_info;	

UInt32	penmgr=0;



//====================================================================================
static FileRef load_from_card(char *path)
{
	char	name[1024];
	StrPrintF(name, "%s/armc.bin", path);

	FileRef f=NULL; 
    UInt16	n=0;
	UInt32 volIterator = vfsIteratorStart; 
	do
	{
		if (errNone!=VFSVolumeEnumerate(&n, &volIterator))
			return NULL;
	}
	while (errNone!=VFSFileOpen(n, name, vfsModeRead, &f));

	if (f)
	{
		screen_info.path=path;
	}
	return f;
}


//====================================================================================
static void message_box(int type, const char *str, char *result)
{
	*result=FrmCustomAlert(type==0 ? InfoMessage : ConfirmAlert, str, NULL, NULL);
	if (penmgr)
	    StatHide();                               
}


//====================================================================================
static void abort_programm()
{
	SysReset();
//	exit(1);
}

/***********************************************************************
 *
 * FUNCTION:    StarterPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err		error;

	error = RomVersionCompatible (kOurMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
		{
		case sysAppLaunchCmdNormalLaunch:
			//---------------------------------------------------------------
			// убираем с экрана все лишнее
			if (errNone==FtrGet(pinCreator, pinFtrAPIVersion, &penmgr))
				penmgr=1;
			else
				penmgr=0;
				
			if (penmgr)
			{
	          	PINSetInputAreaState(pinInputAreaClosed);
				PINSetInputTriggerState(pinInputTriggerNone);
	            SysSetOrientation(sysOrientationPortrait);
	            SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
	            StatHide();            
			}

			screen_info.no_sound=!PalmSGHasSampledSound();
			
			screen_info.message_box=message_box;
			screen_info.abort_programm=abort_programm;

			//--------------------------------------------------------------
			// параметры экрана
			if (	errNone!=WinScreenGetAttribute(winScreenWidth,  &screen_info.width)
				||	errNone!=WinScreenGetAttribute(winScreenHeight, &screen_info.height))
				ERROR("Invalid screen info");
 
			Boolean enableColor;
			WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &screen_info.depths, &enableColor);
			
			//---------------------------------------------------------
			// call the PACE Native Object
			UInt32 processor;
			FtrGet(sysFtrCreator, sysFtrNumProcessorID, &processor);

			if (sysFtrNumProcessorIsARM(processor))
			{
			    //------------------------------------------------------------------------------
				// загружаем код в storage
				MemPtr ppno;
   
				DmOpenRef db=DmOpenDatabaseByTypeCreator('ARMc', CREATOR, dmModeReadOnly);
				if (!db)
				{   // can't find arm database
					// загружаем с карты памяти
                    FileRef f; 
 						f=load_from_card("/Palm/Programs/naPalm Runner");
					if (!f)
						f=load_from_card("/Palm/Programs/naPalmRunner");
					if (!f)
						f=load_from_card("/Palm/Programs/Runner");
					if (!f)
						f=load_from_card("/Palm/Programs/Runer");
					if (!f)
						f=load_from_card("/Palm/Programs/ Runner");
					if (!f)
						f=load_from_card("/Palm/Programs/Runner ");
					if (!f)
						f=load_from_card("/Palm/Programs/ Runner ");
					if (!f)
						f=load_from_card("/Palm/naPalm Runner");
					if (!f)
						f=load_from_card("/Palm/naPalmRunner");
					if (!f)
						f=load_from_card("/Palm/Runner");
					if (!f)
						f=load_from_card("/Palm/Launcher/naPalm Runner");
					if (!f)
						f=load_from_card("/Palm/Launcher/naPalmRunner");
					if (!f)
						f=load_from_card("/Palm/Launcher/Runner");
					if (!f)
						f=load_from_card("/Palm/Launcher");
					if (!f)
						f=load_from_card("/naPalm Runner");
					if (!f)
						f=load_from_card("/naPalmRunner");
					if (!f)
						f=load_from_card("/Runner");
					if (!f)
						ERROR("can't find armc.bin on cards");

					// выделение памяти
					UInt32 size;
                    VFSFileSize(f, &size);
					if (0!=FtrPtrNew(CREATOR, 0, size, &ppno))
						ERROR("no free memory");
					UInt32	read;
					if (errNone!=VFSFileReadData(f, size, ppno, 0, &read))
						ERROR("can't read arm code");
					if (read!=size)
						ERROR("corrupt arm code");
					VFSFileClose(f);
				}
				else
				{
					// загружаем из базы
                    screen_info.path="/runner";
					// размер кода
					MemHandle h = DmGetResource('tint', 1000);
					int	size=*(int*)MemHandleLock(h);
					MemHandleUnlock(h);
					DmReleaseResource(h);

					// выделение памяти
					if (0!=FtrPtrNew(CREATOR, 0, size, &ppno))
						ERROR("no free memory");

					// склейка кода	
					int		id=1000,
							offset=0;
					while (size>0)
					{
						h = DmGetResource('ARMc', id);

						if (!h)
							ERROR("can't find arm resource");

						if (size>SPLIT_SIZE)
							DmWrite(ppno, offset, MemHandleLock(h), SPLIT_SIZE);
						else
						{
							DmWrite(ppno, offset, MemHandleLock(h), size);
							break;
						}

						MemHandleUnlock(h);
						DmReleaseResource(h);

						size-=SPLIT_SIZE;
		                id++;
		                offset+=SPLIT_SIZE;
					}
	                DmCloseDatabase(db);
				}
    
                //--------------------------------------------------------------------------------
				// запуск PNO
				PnoDescriptor pno;
				PnoLoad(&pno, ppno);

				UInt32 result = PnoCall(&pno, &screen_info);
				PnoUnload(&pno);

       			FtrPtrFree(CREATOR, 0);

				if (penmgr)
				{
					PINSetInputTriggerState(pinInputTriggerEnabled);
		            SysSetOrientationTriggerState(sysOrientationTriggerEnabled);
	    	        StatShow();
	    	   	}
                    
                return result;
			}
			return -1;

		default:
			break;

		}
	
	return errNone;
}