/*
 * VueceJingleClientThread.h
 *
 *  Created on: May 1, 2011
 *      Author: jingjing
 */

#ifndef _VUECE_JINGLE_CLIENT_THREAD
#define _VUECE_JINGLE_CLIENT_THREAD

#include <gcroot.h>
#include "jthread.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "VueceUI.h"
#include "VueceConstants.h"
//
namespace vuece
{
class VueceNativeInterface;
}

class VueceJingleClientThread: public JThread,  public sigslot::has_slots<>
{
public:
	VueceJingleClientThread();
	virtual ~VueceJingleClientThread();

	void* Thread();

	void SetAccount(const char* userName, const char* pwd, const int auth_type);
	void OnUserEvent(VueceEvent code);
	void OnRemoteDeviceActivity(VueceStreamingDevice* d);
	void OnRosterVCardReceived(const std::string&, const std::string&, const std::string&);

	void OnRosterSubscriptionMsgReceived(VueceRosterSubscriptionMsg* m);

	void OnFileSystemChanged();
	void TestSendFile();

private:
	bool bSignedIn;
    sigslot::signal0<> SigFileSystemChanged;

private:
    int auth_type;
    char userName[64];
    char userPwd[256];

public:
    vuece::VueceNativeInterface *client;
    sigslot::signal1<VueceEvent> SigVueceEvent;
	gcroot<VueceUI^> ui;

 };

#endif
