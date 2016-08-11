/*
Copyright (c) 2016 Sam Zielke-Ryner All rights reserved.

For job opportunities or to work together on projects please contact
myself via Github:   https://github.com/sazr

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. The source code, API or snippets cannot be used for commercial
purposes without written consent from the author.

THIS SOFTWARE IS PROVIDED ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include "stdafx.h"
#include "PingUsageLogsComponent.h"

// Class Property Implementation //
Status PingUsageLogsComponent::WM_PING_COMPLETE = Status::registerState(_T("WM_PING_COMPLETE"));

// Static Function Implementation //


// Function Implementation //
PingUsageLogsComponent::PingUsageLogsComponent(const std::weak_ptr<IApp>& app, const tstring& usageLogFileDirectory, STATE uid)
	: Component(app), 
	strtLogIndex(0), endLogIndex(0), origLogIndex(0),
	uid(uid), usageLogFileDirectory(usageLogFileDirectory),
	usageLogFileDirectoryA(WinUtilityComponent::wstrtostr(usageLogFileDirectory)),
	pingUid(Status::registerState(_T("Ping unique id")).state),
	logDwnldUid(Status::registerState(_T("Download log unique id")).state),
	isDownloading(false)
{
	dldCmp = addComponent<DownloadComponent>(app);
	registerEvents();
}

PingUsageLogsComponent::~PingUsageLogsComponent()
{

}

Status PingUsageLogsComponent::init(const IEventArgs& evtArgs)
{
	const WinEventArgs& args = static_cast<const WinEventArgs&>(evtArgs);
	mainHwnd = args.hwnd;

	int index = 0;
	TCHAR fPath[MAX_PATH];

	do {
		index++;
		_stprintf(fPath, _T("%s\\%d"), usageLogFileDirectory.c_str(), index);
	} while (WinUtilityComponent::fileExists(tstring(fPath)) == S_SUCCESS);

	endLogIndex = --index;
	strtLogIndex = endLogIndex;
	origLogIndex = strtLogIndex;

	SetTimer(mainHwnd, pingUid, 600000, NULL); // ping every 10 minutes
	onTimer(NULL_ARGS); // run first time
	
	return S_SUCCESS;
}

Status PingUsageLogsComponent::terminate(const IEventArgs& evtArgs)
{
	
	return S_SUCCESS;
}

Status PingUsageLogsComponent::registerEvents()
{
	registerEvent(WM_CREATE, &PingUsageLogsComponent::init);
	registerEvent(WM_CLOSE, &PingUsageLogsComponent::terminate);
	registerEvent(DispatchWindowComponent::translateMessage(pingUid, DownloadComponent::WM_DOWNLOAD_COMPLETE), &PingUsageLogsComponent::onPingResponse);
	//registerEvent(DispatchWindowComponent::translateMessage(logDwnldUid, DownloadComponent::WM_DOWNLOAD_COMPLETE), &PingUsageLogsComponent::onDownloadLog);
	registerEvent(DispatchWindowComponent::translateMessage(pingUid, WM_TIMER), &PingUsageLogsComponent::onTimer);
	registerEvent(DispatchWindowComponent::translateMessage(logDwnldUid, WM_TIMER), &PingUsageLogsComponent::onDownloadLogTimer);
	return S_SUCCESS;
}

Status PingUsageLogsComponent::onTimer(const IEventArgs& evtArgs)
{
	if (isDownloading)
		return S_UNDEFINED_ERROR;

	dldCmp->download("http://windowtiler.soribo.com.au/pingUsage.php?application=window-tiler", pingUid);
	return S_SUCCESS;
}

Status PingUsageLogsComponent::onDownloadLogTimer(const IEventArgs& evtArgs)
{
	if (isDownloading)
		return S_UNDEFINED_ERROR;

	if (strtLogIndex >= endLogIndex) {
		KillTimer(mainHwnd, logDwnldUid);

		const WinEventArgs wArgs(NULL, NULL, origLogIndex, 0);
		Win32App::eventHandler(DispatchWindowComponent::translateMessage(uid, WM_PING_COMPLETE), wArgs);
		
		origLogIndex = strtLogIndex;
		return S_SUCCESS;
	}

	isDownloading = true;
	char url[100], outputFilePath[100];
	sprintf(url, "http://windowtiler.soribo.com.au/logs/%d", strtLogIndex+1);
	sprintf(outputFilePath, "%s\\%d", usageLogFileDirectoryA.c_str(), strtLogIndex+1);
	
	if (dldCmp->downloadFile(url, outputFilePath, logDwnldUid) == S_SUCCESS)
		strtLogIndex++;

	isDownloading = false;

	return S_SUCCESS;
}

Status PingUsageLogsComponent::onPingResponse(const IEventArgs& evtArgs)
{
	const DownloadEvtArgs& args = static_cast<const DownloadEvtArgs&>(evtArgs);
	
	int newEndLogIndex = atoi(args.data.c_str());

	if (newEndLogIndex <= endLogIndex)
		return S_SUCCESS;

	strtLogIndex = endLogIndex;
	endLogIndex = newEndLogIndex;
	ShowWindow(mainHwnd, SW_HIDE); // hide window until download is complete
	SetTimer(mainHwnd, logDwnldUid, 100, NULL);
	
	return S_SUCCESS;
}

//Status PingUsageLogsComponent::onDownloadLog(const IEventArgs& evtArgs)
//{
//	const DownloadEvtArgs& dldArgs = static_cast<const DownloadEvtArgs&>(evtArgs);
//
//	return S_SUCCESS;
//}

