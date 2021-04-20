/*************************************************************************/ /*!
@File
@Title          Server bridge for srvcore
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements the server side of the bridge for srvcore
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#include <stddef.h>
#include <asm/uaccess.h>

#include "img_defs.h"

#include "srvcore.h"
#include "pvrsrv.h"


#include "common_srvcore_bridge.h"

#include "allocmem.h"
#include "pvr_debug.h"
#include "connection_server.h"
#include "pvr_bridge.h"
#include "rgx_bridge.h"
#include "srvcore.h"
#include "handle.h"

#include <linux/slab.h>




/* ***************************************************************************
 * Server-side bridge entry points
 */
 
static IMG_INT
PVRSRVBridgeConnect(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_CONNECT *psConnectIN,
					  PVRSRV_BRIDGE_OUT_CONNECT *psConnectOUT,
					 CONNECTION_DATA *psConnection)
{







	psConnectOUT->eError =
		PVRSRVConnectKM(psConnection, OSGetDevData(psConnection),
					psConnectIN->ui32Flags,
					psConnectIN->ui32ClientBuildOptions,
					psConnectIN->ui32ClientDDKVersion,
					psConnectIN->ui32ClientDDKBuild,
					&psConnectOUT->ui8KernelArch,
					&psConnectOUT->ui32Log2PageSize);





	return 0;
}

static IMG_INT
PVRSRVBridgeDisconnect(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_DISCONNECT *psDisconnectIN,
					  PVRSRV_BRIDGE_OUT_DISCONNECT *psDisconnectOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psDisconnectIN);






	psDisconnectOUT->eError =
		PVRSRVDisconnectKM(
					);





	return 0;
}

static IMG_INT
PVRSRVBridgeInitSrvDisconnect(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_INITSRVDISCONNECT *psInitSrvDisconnectIN,
					  PVRSRV_BRIDGE_OUT_INITSRVDISCONNECT *psInitSrvDisconnectOUT,
					 CONNECTION_DATA *psConnection)
{





	PMRLock();


	psInitSrvDisconnectOUT->eError =
		PVRSRVInitSrvDisconnectKM(psConnection, OSGetDevData(psConnection),
					psInitSrvDisconnectIN->bInitSuccesful,
					psInitSrvDisconnectIN->ui32ClientBuildOptions);
	PMRUnlock();





	return 0;
}

static IMG_INT
PVRSRVBridgeAcquireGlobalEventObject(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_ACQUIREGLOBALEVENTOBJECT *psAcquireGlobalEventObjectIN,
					  PVRSRV_BRIDGE_OUT_ACQUIREGLOBALEVENTOBJECT *psAcquireGlobalEventObjectOUT,
					 CONNECTION_DATA *psConnection)
{
	IMG_HANDLE hGlobalEventObjectInt = NULL;

	PVR_UNREFERENCED_PARAMETER(psAcquireGlobalEventObjectIN);






	psAcquireGlobalEventObjectOUT->eError =
		AcquireGlobalEventObjectServer(
					&hGlobalEventObjectInt);
	/* Exit early if bridged call fails */
	if(psAcquireGlobalEventObjectOUT->eError != PVRSRV_OK)
	{
		goto AcquireGlobalEventObject_exit;
	}


	psAcquireGlobalEventObjectOUT->eError = PVRSRVAllocHandle(psConnection->psHandleBase,
							&psAcquireGlobalEventObjectOUT->hGlobalEventObject,
							(void *) hGlobalEventObjectInt,
							PVRSRV_HANDLE_TYPE_SHARED_EVENT_OBJECT,
							PVRSRV_HANDLE_ALLOC_FLAG_MULTI
							,(PFN_HANDLE_RELEASE)&ReleaseGlobalEventObjectServer);
	if (psAcquireGlobalEventObjectOUT->eError != PVRSRV_OK)
	{
		goto AcquireGlobalEventObject_exit;
	}




AcquireGlobalEventObject_exit:
	if (psAcquireGlobalEventObjectOUT->eError != PVRSRV_OK)
	{
		if (hGlobalEventObjectInt)
		{
			ReleaseGlobalEventObjectServer(hGlobalEventObjectInt);
		}
	}


	return 0;
}

static IMG_INT
PVRSRVBridgeReleaseGlobalEventObject(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_RELEASEGLOBALEVENTOBJECT *psReleaseGlobalEventObjectIN,
					  PVRSRV_BRIDGE_OUT_RELEASEGLOBALEVENTOBJECT *psReleaseGlobalEventObjectOUT,
					 CONNECTION_DATA *psConnection)
{









	psReleaseGlobalEventObjectOUT->eError =
		PVRSRVReleaseHandle(psConnection->psHandleBase,
					(IMG_HANDLE) psReleaseGlobalEventObjectIN->hGlobalEventObject,
					PVRSRV_HANDLE_TYPE_SHARED_EVENT_OBJECT);
	if ((psReleaseGlobalEventObjectOUT->eError != PVRSRV_OK) && (psReleaseGlobalEventObjectOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_ASSERT(0);
		goto ReleaseGlobalEventObject_exit;
	}



ReleaseGlobalEventObject_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeEventObjectOpen(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_EVENTOBJECTOPEN *psEventObjectOpenIN,
					  PVRSRV_BRIDGE_OUT_EVENTOBJECTOPEN *psEventObjectOpenOUT,
					 CONNECTION_DATA *psConnection)
{
	IMG_HANDLE hEventObjectInt = NULL;
	IMG_HANDLE hOSEventInt = NULL;







				{
					/* Look up the address from the handle */
					psEventObjectOpenOUT->eError =
						PVRSRVLookupHandle(psConnection->psHandleBase,
											(void **) &hEventObjectInt,
											psEventObjectOpenIN->hEventObject,
											PVRSRV_HANDLE_TYPE_SHARED_EVENT_OBJECT);
					if(psEventObjectOpenOUT->eError != PVRSRV_OK)
					{
						goto EventObjectOpen_exit;
					}
				}


	psEventObjectOpenOUT->eError =
		OSEventObjectOpen(
					hEventObjectInt,
					&hOSEventInt);
	/* Exit early if bridged call fails */
	if(psEventObjectOpenOUT->eError != PVRSRV_OK)
	{
		goto EventObjectOpen_exit;
	}


	psEventObjectOpenOUT->eError = PVRSRVAllocHandle(psConnection->psHandleBase,
							&psEventObjectOpenOUT->hOSEvent,
							(void *) hOSEventInt,
							PVRSRV_HANDLE_TYPE_EVENT_OBJECT_CONNECT,
							PVRSRV_HANDLE_ALLOC_FLAG_MULTI
							,(PFN_HANDLE_RELEASE)&OSEventObjectClose);
	if (psEventObjectOpenOUT->eError != PVRSRV_OK)
	{
		goto EventObjectOpen_exit;
	}




EventObjectOpen_exit:
	if (psEventObjectOpenOUT->eError != PVRSRV_OK)
	{
		if (hOSEventInt)
		{
			OSEventObjectClose(hOSEventInt);
		}
	}


	return 0;
}

static IMG_INT
PVRSRVBridgeEventObjectWait(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_EVENTOBJECTWAIT *psEventObjectWaitIN,
					  PVRSRV_BRIDGE_OUT_EVENTOBJECTWAIT *psEventObjectWaitOUT,
					 CONNECTION_DATA *psConnection)
{
	IMG_HANDLE hOSEventKMInt = NULL;







				{
					/* Look up the address from the handle */
					psEventObjectWaitOUT->eError =
						PVRSRVLookupHandle(psConnection->psHandleBase,
											(void **) &hOSEventKMInt,
											psEventObjectWaitIN->hOSEventKM,
											PVRSRV_HANDLE_TYPE_EVENT_OBJECT_CONNECT);
					if(psEventObjectWaitOUT->eError != PVRSRV_OK)
					{
						goto EventObjectWait_exit;
					}
				}


	psEventObjectWaitOUT->eError =
		OSEventObjectWait(
					hOSEventKMInt);




EventObjectWait_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeEventObjectClose(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_EVENTOBJECTCLOSE *psEventObjectCloseIN,
					  PVRSRV_BRIDGE_OUT_EVENTOBJECTCLOSE *psEventObjectCloseOUT,
					 CONNECTION_DATA *psConnection)
{









	psEventObjectCloseOUT->eError =
		PVRSRVReleaseHandle(psConnection->psHandleBase,
					(IMG_HANDLE) psEventObjectCloseIN->hOSEventKM,
					PVRSRV_HANDLE_TYPE_EVENT_OBJECT_CONNECT);
	if ((psEventObjectCloseOUT->eError != PVRSRV_OK) && (psEventObjectCloseOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_ASSERT(0);
		goto EventObjectClose_exit;
	}



EventObjectClose_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDumpDebugInfo(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_DUMPDEBUGINFO *psDumpDebugInfoIN,
					  PVRSRV_BRIDGE_OUT_DUMPDEBUGINFO *psDumpDebugInfoOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psConnection);






	psDumpDebugInfoOUT->eError =
		PVRSRVDumpDebugInfoKM(
					psDumpDebugInfoIN->ui32ui32VerbLevel);





	return 0;
}

static IMG_INT
PVRSRVBridgeGetDevClockSpeed(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_GETDEVCLOCKSPEED *psGetDevClockSpeedIN,
					  PVRSRV_BRIDGE_OUT_GETDEVCLOCKSPEED *psGetDevClockSpeedOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psGetDevClockSpeedIN);






	psGetDevClockSpeedOUT->eError =
		PVRSRVGetDevClockSpeedKM(psConnection, OSGetDevData(psConnection),
					&psGetDevClockSpeedOUT->ui32ui32ClockSpeed);





	return 0;
}

static IMG_INT
PVRSRVBridgeHWOpTimeout(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_HWOPTIMEOUT *psHWOpTimeoutIN,
					  PVRSRV_BRIDGE_OUT_HWOPTIMEOUT *psHWOpTimeoutOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psHWOpTimeoutIN);






	psHWOpTimeoutOUT->eError =
		PVRSRVHWOpTimeoutKM(
					);





	return 0;
}

static IMG_INT
PVRSRVBridgeKickDevices(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_KICKDEVICES *psKickDevicesIN,
					  PVRSRV_BRIDGE_OUT_KICKDEVICES *psKickDevicesOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psKickDevicesIN);






	psKickDevicesOUT->eError =
		PVRSRVKickDevicesKM(
					);





	return 0;
}

static IMG_INT
PVRSRVBridgeResetHWRLogs(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_RESETHWRLOGS *psResetHWRLogsIN,
					  PVRSRV_BRIDGE_OUT_RESETHWRLOGS *psResetHWRLogsOUT,
					 CONNECTION_DATA *psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psResetHWRLogsIN);






	psResetHWRLogsOUT->eError =
		PVRSRVResetHWRLogsKM(psConnection, OSGetDevData(psConnection)
					);





	return 0;
}

static IMG_INT
PVRSRVBridgeSoftReset(IMG_UINT32 ui32DispatchTableEntry,
					  PVRSRV_BRIDGE_IN_SOFTRESET *psSoftResetIN,
					  PVRSRV_BRIDGE_OUT_SOFTRESET *psSoftResetOUT,
					 CONNECTION_DATA *psConnection)
{







	psSoftResetOUT->eError =
		PVRSRVSoftResetKM(psConnection, OSGetDevData(psConnection),
					psSoftResetIN->ui64ResetValue1,
					psSoftResetIN->ui64ResetValue2);





	return 0;
}



/* *************************************************************************** 
 * Server bridge dispatch related glue 
 */

static IMG_BOOL bUseLock = IMG_TRUE;

PVRSRV_ERROR InitSRVCOREBridge(void);
PVRSRV_ERROR DeinitSRVCOREBridge(void);

/*
 * Register all SRVCORE functions with services
 */
PVRSRV_ERROR InitSRVCOREBridge(void)
{

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_CONNECT, PVRSRVBridgeConnect,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_DISCONNECT, PVRSRVBridgeDisconnect,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_INITSRVDISCONNECT, PVRSRVBridgeInitSrvDisconnect,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_ACQUIREGLOBALEVENTOBJECT, PVRSRVBridgeAcquireGlobalEventObject,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_RELEASEGLOBALEVENTOBJECT, PVRSRVBridgeReleaseGlobalEventObject,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_EVENTOBJECTOPEN, PVRSRVBridgeEventObjectOpen,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_EVENTOBJECTWAIT, PVRSRVBridgeEventObjectWait,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_EVENTOBJECTCLOSE, PVRSRVBridgeEventObjectClose,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_DUMPDEBUGINFO, PVRSRVBridgeDumpDebugInfo,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_GETDEVCLOCKSPEED, PVRSRVBridgeGetDevClockSpeed,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_HWOPTIMEOUT, PVRSRVBridgeHWOpTimeout,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_KICKDEVICES, PVRSRVBridgeKickDevices,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_RESETHWRLOGS, PVRSRVBridgeResetHWRLogs,
					NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_SRVCORE, PVRSRV_BRIDGE_SRVCORE_SOFTRESET, PVRSRVBridgeSoftReset,
					NULL, bUseLock);


	return PVRSRV_OK;
}

/*
 * Unregister all srvcore functions with services
 */
PVRSRV_ERROR DeinitSRVCOREBridge(void)
{
	return PVRSRV_OK;
}

