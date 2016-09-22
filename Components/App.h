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

#ifndef APP_USAGE_VIEWER_H
#define APP_USAGE_VIEWER_H

#include "stdafx.h"

class App : public Win32App
{
public:
	friend class IApp;

	static Status WM_MENU_REPORT_BUG;
	static Status WM_MENU_DONATE;
	static Status WM_MENU_EXIT;

	virtual ~App();

	Status init(const IEventArgs& evtArgs);

	Status terminate(const IEventArgs& evtArgs);

protected:
	App();

	Status onTrayIconInteraction(const IEventArgs& evtArgs);
	Status initMenu(); 
	Status addSubjectInfoLV(const WinEventArgs& args);
	Status addUsageLV(const WinEventArgs& args);
	Status onKillFocus(const IEventArgs& evtArgs);
	Status parseUsageFiles(const IEventArgs& evtArgs);

private:
	STATE pingUid;
	STATE subjInfoLVUid;
	STATE usageLVUid;
	tstring usageLogFileDirectory;
	HMENU hMenu;
	HWND subjInfoListView;
	HWND usageListView;
	std::shared_ptr<SystemTrayComponent> sysTrayCmp;
	std::map<tstring, int> uidRowMap;
};

#endif // APP_USAGE_VIEWER_H