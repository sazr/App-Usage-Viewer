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

#ifndef AUV_PINGUSAGELOGSCMP_H
#define AUV_PINGUSAGELOGSCMP_H

#include "stdafx.h"

class PingUsageLogsComponent : public Component
{
public:
	friend class Component;

	// Static Variables //
	static Status WM_PING_COMPLETE;

	// Static Methods //

	// Class Variables //

	// Class Methods //
	virtual ~PingUsageLogsComponent();

	Status init(const IEventArgs& evtArgs);
	Status terminate(const IEventArgs& evtArgs);

protected:
	// Static Variables //

	// Static Methods //

	// Class Variables //

	// Class Methods //
	PingUsageLogsComponent(const std::weak_ptr<IApp>& app, const tstring& usageLogFileDirectory, STATE uid);

	Status registerEvents();
	Status onTimer(const IEventArgs& evtArgs);
	Status onDownloadLogTimer(const IEventArgs& evtArgs);
	Status onPingResponse(const IEventArgs& evtArgs);
	//Status onDownloadLog(const IEventArgs& evtArgs);

private:
	// Static Variables //

	// Static Methods //

	// Class Variables //
	STATE uid;
	STATE pingUid;
	STATE logDwnldUid;
	HWND mainHwnd;
	int strtLogIndex;
	int endLogIndex;
	int origLogIndex;
	bool isDownloading;
	const tstring usageLogFileDirectory;
	const std::string usageLogFileDirectoryA;
	std::shared_ptr<DownloadComponent> dldCmp;

	// Class Methods //

};

#endif // AUV_PINGUSAGELOGSCMP_H