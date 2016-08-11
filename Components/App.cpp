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
#include "App.h"

// Class Property Implementation //
Status App::WM_MENU_REPORT_BUG		= Status::registerState(_T("WM_MENU_REPORT_BUG"));
Status App::WM_MENU_DONATE			= Status::registerState(_T("WM_MENU_DONATE"));
Status App::WM_MENU_EXIT			= Status::registerState(_T("WM_MENU_EXIT"));


// Static Function Implementation //


// Function Implementation //
App::App() 
	: Win32App(), 
	subjInfoLVUid(Status::registerState(_T("Subject Info Listview Uid")).state),
	usageLVUid(Status::registerState(_T("Usage Listview Uid")).state),
	pingUid(Status::registerState(_T("Ping Uid")).state)
{
	wndDimensions	= RECT{ -400, -400, 400, 600 };
	bkColour		= CreateSolidBrush(RGB(50, 50, 50));
	wndTitle		= _T("Application Usage Viewer");
	wndFlags		= WS_EX_TOOLWINDOW | WS_POPUP;
	iconID			= IDI_APP;
	smallIconID		= IDI_SMALL;

	TCHAR szPath[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath) == S_OK) {
		usageLogFileDirectory = tstring(szPath) + _T("\\AppUsageViewer\\usageLogs");
		SHCreateDirectoryEx(NULL, usageLogFileDirectory.c_str(), NULL);
	}
}

App::~App()
{

}

Status App::init(const IEventArgs& evtArgs)
{
	const WinEventArgs& args = static_cast<const WinEventArgs&>(evtArgs);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	addComponent<ScheduleAppComponent>(app, _T("AppUsageViewerCBA"));
	addComponent<DPIAwareComponent>(app);
	addComponent<PingUsageLogsComponent>(app, usageLogFileDirectory, pingUid);
	addComponent<BorderWindowComponent>(app, RECT{ 6, 6, 388, 588 });
	sysTrayCmp		= addComponent<SystemTrayComponent>(app, _T("images/small.ico"), _T("App Usage Viewer"));
	auto tskDkCmp	= addComponent<TaskBarDockComponent>(app, SIZE{ 20, 20 });

	Win32App::init(evtArgs);
	
	#pragma message("Todo: make win32app have flag for dpiaware scale")
	DPIAwareComponent::scaleRect(wndDimensions);
	SetWindowPos(hwnd, 0, wndDimensions.left, wndDimensions.top, wndDimensions.right, wndDimensions.bottom, 0);
	ShowWindow(hwnd, SW_HIDE); // Hide window till downloads are complete

	initMenu();
	addSubjectInfoLV(args);
	addUsageLV(args);

	registerEvent(SystemTrayComponent::WM_TRAY_ICON.state, &App::onTrayIconInteraction);
	registerEvent(WM_ACTIVATEAPP, &App::onKillFocus);
	registerEvent(DispatchWindowComponent::translateMessage(pingUid, PingUsageLogsComponent::WM_PING_COMPLETE), &App::parseUsageFiles);

	const WinEventArgs wArgs(hinstance, hwnd, 0, 0);
	parseUsageFiles(wArgs);

	return S_SUCCESS;
}

Status App::terminate(const IEventArgs& evtArgs)
{
	Win32App::terminate(evtArgs);

	return S_SUCCESS;
}

Status App::onTrayIconInteraction(const IEventArgs& evtArgs)
{
	const WinEventArgs& args = static_cast<const WinEventArgs&>(evtArgs);

	// if the tray icon message is not for this SystemTrayComponent tray icon: return
	if (args.wParam != sysTrayCmp->trayIconID.state)
		return S_SUCCESS;

	if (args.lParam == WM_LBUTTONDOWN) {
		if (IsWindowVisible(hwnd))
			ShowWindow(hwnd, SW_HIDE);
		else ShowWindow(hwnd, SW_SHOW);
	}
	else if (args.lParam == WM_RBUTTONDOWN) {
		POINT curPoint;
		GetCursorPos(&curPoint);
		SetForegroundWindow(args.hwnd);

		UINT clicked = TrackPopupMenu(hMenu,
			TPM_RETURNCMD | TPM_NONOTIFY | TPM_VERPOSANIMATION,
			curPoint.x, curPoint.y, 0, args.hwnd, NULL);

		if (clicked == WM_MENU_REPORT_BUG.state)
		{
			ShellExecute(NULL, _T("open"), _T("http://windowtiler.soribo.com.au/report-a-bug/"), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (clicked == WM_MENU_DONATE.state)
		{
			ShellExecute(NULL, _T("open"), _T("http://windowtiler.soribo.com.au/donate"), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (clicked == WM_MENU_EXIT.state)
		{
			PostMessage(args.hwnd, WM_CLOSE, 0, 0);
		}
	}

	return S_SUCCESS;
}

Status App::initMenu()
{
	hMenu = CreatePopupMenu();

	AppendMenu(hMenu, MF_STRING, WM_MENU_REPORT_BUG.state, TEXT("Report a bug"));
	AppendMenu(hMenu, MF_STRING, WM_MENU_DONATE.state, TEXT("Donate"));
	AppendMenu(hMenu, MF_STRING, WM_MENU_EXIT.state, TEXT("Exit"));
	return S_SUCCESS;
}

Status App::addSubjectInfoLV(const WinEventArgs& args)
{
	RECT rcClient;                      
	GetClientRect(hwnd, &rcClient);
	int buffer = 20;
	int x = buffer;
	int y = buffer;
	int w = rcClient.right - rcClient.left - (buffer*2);
	int h = (rcClient.bottom - rcClient.top - (buffer * 2)) * 0.25;

	subjInfoListView = CreateWindowEx(0, WC_LISTVIEW,
		0,
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
		x, y, w, h,
		hwnd,
		(HMENU)subjInfoLVUid,
		args.hinstance,
		NULL);

	// Insert LV columns
	TCHAR* columnHeaders[6] = { _T("Uid"), _T("Monitors"), _T("Windows Version"), _T("Timezone Standard Name")};
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int iCol = 0; iCol < 4; iCol++) {
		lvc.iSubItem = iCol;
		lvc.pszText = columnHeaders[iCol];
		lvc.cx = 130;

		if (iCol < 2)
			lvc.fmt = LVCFMT_LEFT;
		else
			lvc.fmt = LVCFMT_RIGHT;

		if (ListView_InsertColumn(subjInfoListView, iCol, &lvc) == -1) {
			output(_T("Error ListView_InsertColumn\n"));
			return S_UNDEFINED_ERROR;
		}
	}

	return S_SUCCESS;
}

Status App::addUsageLV(const WinEventArgs& args)
{
	RECT rcClient;
	GetClientRect(hwnd, &rcClient);
	int buffer = 20;
	int x = buffer;
	int y = (rcClient.bottom - rcClient.top - (buffer * 2)) * 0.25 + (buffer * 2);
	int w = rcClient.right - rcClient.left - (buffer * 2);
	int h = ((rcClient.bottom - rcClient.top - (buffer*2)) * 0.75) - buffer;

	usageListView = CreateWindowEx(0, WC_LISTVIEW,
		0,
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
		x, y, w, h,
		hwnd,
		(HMENU)usageLVUid,
		args.hinstance,
		NULL);

	// Insert LV columns
	TCHAR* columnHeaders[6] = { _T("Uid Index"), _T("Time Stamp"), _T("Interaction"), _T("Type") };
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int iCol = 0; iCol < 4; iCol++) {
		lvc.iSubItem = iCol;
		lvc.pszText = columnHeaders[iCol];
		lvc.cx = 130;

		if (iCol < 2)
			lvc.fmt = LVCFMT_LEFT;
		else
			lvc.fmt = LVCFMT_RIGHT;

		if (ListView_InsertColumn(usageListView, iCol, &lvc) == -1) {
			output(_T("Error ListView_InsertColumn\n"));
			return S_UNDEFINED_ERROR;
		}
	}

	return S_SUCCESS;
}

Status App::onKillFocus(const IEventArgs& evtArgs)
{
	const WinEventArgs& args = static_cast<const WinEventArgs&>(evtArgs);

	if (args.wParam == FALSE)
		ShowWindow(hwnd, SW_HIDE);

	return S_SUCCESS;
}

Status App::parseUsageFiles(const IEventArgs& evtArgs)
{
	const WinEventArgs& args = static_cast<const WinEventArgs&>(evtArgs);
	int index = (int)args.wParam;
	TCHAR fPath[MAX_PATH];

	do {
		index++;
		_stprintf(fPath, _T("%s\\%d"), usageLogFileDirectory.c_str(), index);
		int rowIndex = ListView_GetItemCount(subjInfoListView);

		SubjectInformation sInfo;

		if (!GetPrivateProfileStruct(_T("SubjectInformation"), _T("Information"), &sInfo, sizeof(SubjectInformation), fPath)) {
			output(_T("Failed to read struct\n"));
			continue;
		}

		if (uidRowMap.find(sInfo.uid) != uidRowMap.end()) {
			ListView_DeleteItem(subjInfoListView, uidRowMap[sInfo.uid]);
			rowIndex--;
		}

		uidRowMap[sInfo.uid] = rowIndex;

		LVITEM lvi = { 0 };
		lvi.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvi.iItem = rowIndex;
		ListView_InsertItem(subjInfoListView, &lvi);

		ListView_SetItemText(subjInfoListView, rowIndex, 0, sInfo.uid);
		output(_T("uid: %s, "), sInfo.uid);

		tstringstream ss;
		ss << sInfo.nMonitors;
		ListView_SetItemText(subjInfoListView, rowIndex, 1, (LPTSTR)ss.str().c_str());
		output(_T("%s, "), ss.str().c_str());

		ss.clear();
		ss << sInfo.versionInfo.dwMajorVersion << _T("-") << sInfo.versionInfo.dwMinorVersion;
		ListView_SetItemText(subjInfoListView, rowIndex, 2, (LPTSTR)ss.str().c_str());
		output(_T("%s, "), ss.str().c_str());

		ListView_SetItemText(subjInfoListView, rowIndex, 3, sInfo.timeZoneInfo.StandardName);
		output(_T("%s\n"), sInfo.timeZoneInfo.StandardName);

		// Populate interactions listview
		int interIndex = 0;
		UsageInformation uInfo;
		TCHAR interactionKey[MAX_PATH];
		_stprintf(interactionKey, _T("%d"), interIndex);
		if (!GetPrivateProfileStruct(interactionKey, _T("Interaction"), &uInfo, sizeof(UsageInformation), fPath)) {

			// Bug sometimes interIndex starts at 1 not 0
			interIndex++;
			_stprintf(interactionKey, _T("%d"), interIndex);
			if (!GetPrivateProfileStruct(interactionKey, _T("Interaction"), &uInfo, sizeof(UsageInformation), fPath))
				continue;
		}

		do {
			int interRow = ListView_GetItemCount(usageListView);

			LVITEM lvi = { 0 };
			lvi.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvi.cColumns = 4;
			lvi.iItem = interRow;
			ListView_InsertItem(usageListView, &lvi);

			TCHAR buf[10];
			_stprintf(buf, _T("%d"), rowIndex + 1 /*index*/); 
			ListView_SetItemText(usageListView, interRow, 0, buf);

			TCHAR timeStmp[MAX_PATH];
			_tcsftime(timeStmp, MAX_PATH, _T("%Y-%m-%d %H:%M:%S"), gmtime(&uInfo.timeStamp));
			ListView_SetItemText(usageListView, interRow, 1, timeStmp);

			ListView_SetItemText(usageListView, interRow, 2, uInfo.annotation);

			ListView_SetItemText(usageListView, interRow, 3, (uInfo.msg == -1) ? _T("Default") : _T("Event Based"));
			
			interIndex++;
			_stprintf(interactionKey, _T("%d"), interIndex);

		} while (GetPrivateProfileStruct(interactionKey, _T("Interaction"), &uInfo, sizeof(UsageInformation), fPath));

	} while (WinUtilityComponent::fileExists(tstring(fPath)) == S_SUCCESS);

	ShowWindow(hwnd, SW_SHOW);

	return S_SUCCESS;
}