/*
 * VueceConnectionMonitorThread.h
 *
 *  Created on: May 30, 2013
 *      Author: Jingjing Sun
 */

#ifndef VUECECONNECTIONMONITORTHREAD_H_
#define VUECECONNECTIONMONITORTHREAD_H_

//extern "C"
//{
//	#include "stdint.h"
//}

#include <gcroot.h>

#include "talk/base/sigslot.h"
#include "talk/p2p/base/constants.h"
#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"
#include "talk/base/scoped_ptr.h"
#include "VueceUI.h"
//

typedef enum  VueceNetworkMonitorState{
	VueceNetworkMonitorState_OnlineSignedIn = 0,
	VueceNetworkMonitorState_Offline,
	VueceNetworkMonitorState_WaitingForSignOut,
	VueceNetworkMonitorState_Stopped
} VueceNetworkMonitorState;

class VueceConnectionMonitorThread: public talk_base::MessageHandler,  public sigslot::has_slots<>
{
public:
	VueceConnectionMonitorThread();
	virtual ~VueceConnectionMonitorThread();

	virtual void OnMessage(talk_base::Message *msg);

	void Start(VueceNetworkMonitorState initialState);

	void Stop();

	void ForceOffline();

	void WaitForSignOut();

	static bool CheckConnection();

private:

	bool bSignedIn;
    sigslot::signal0<> SigFileSystemChanged;

	  talk_base::Thread* worker_thread_;
	  bool bStarted;
	  int iState;
	  int iReconnectTimer;

public:
	gcroot<VueceUI^> ui;

 };


#endif /* VUECECONNECTIONMONITORTHREAD_H_ */
