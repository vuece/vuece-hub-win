//NOTE this file is not used in ANDROID implementation

#include "talk/base/logging.h"
#include "VueceConnectionMonitorThread.h"
#include "VueceConstants.h"

#include <Sensapi.h>

#define VUECE_CONNECTION_CHECK_INTERVAL_SECOND 5
#define VUECE_RE_CONNECT_TIMER 15

VueceConnectionMonitorThread::VueceConnectionMonitorThread() :
	worker_thread_(new talk_base::Thread())
{
	LOG(INFO) << "VueceConnectionMonitorThread constructor called.";

	bSignedIn = false;

	LOG(INFO) << "Instance created.";
}

VueceConnectionMonitorThread::~VueceConnectionMonitorThread() {
	LOG(LS_VERBOSE) << "VueceConnectionMonitorThread::De-constructor called, stop now";
	Stop();
}

void VueceConnectionMonitorThread::Start(VueceNetworkMonitorState initialState) {
	LOG(INFO) << "VueceConnectionKeeper::Start with initial state: " << initialState;
	iState = initialState;
	worker_thread_->Start();
	worker_thread_->PostDelayed(VUECE_CONNECTION_CHECK_INTERVAL_SECOND * 1000 / 2, this, 0);
	bStarted = true;
}

void VueceConnectionMonitorThread::Stop() {
	LOG(LS_VERBOSE) << "VueceConnectionMonitorThread::Stop";
	if(bStarted){
		LOG(LS_VERBOSE) << "VueceConnectionMonitorThread::Already started, stop it now";
		worker_thread_->Stop();

		bStarted = false;
	}
}

bool VueceConnectionMonitorThread::CheckConnection()
{

	DWORD   flags;
	bool   m_bOnline = true;

	LOG(INFO) << "VueceConnectionMonitorThread::CheckConnection";

	m_bOnline = IsNetworkAlive(&flags);
	if(m_bOnline)
	{
	   if ((flags & NETWORK_ALIVE_LAN) == NETWORK_ALIVE_LAN)
	   {
		   LOG(INFO) <<"VueceConnectionMonitorThread - Online:NETWORK_ALIVE_LAN";
	   }

	   if ((flags & NETWORK_ALIVE_WAN) == NETWORK_ALIVE_WAN)
	   {
		   LOG(INFO) <<"VueceConnectionMonitorThread - Online:NETWORK_ALIVE_WAN";
	   }

	   if ((flags & NETWORK_ALIVE_AOL) == NETWORK_ALIVE_AOL)
	   {
		   LOG(INFO) <<"VueceConnectionMonitorThread - Online:NETWORK_ALIVE_AOL";
	   }

	}
	else
	{
		LOG(INFO) <<"VueceConnectionMonitorThread - Offline";
	}

	return m_bOnline;
}

void VueceConnectionMonitorThread::ForceOffline()
{
	LOG(INFO) << "VueceConnectionMonitorThread:ForceOffline";
	iState = VueceNetworkMonitorState_Offline;
}

void VueceConnectionMonitorThread::WaitForSignOut()
{
	LOG(INFO) << "VueceConnectionMonitorThread:WaitForSignOut";
	iState = VueceNetworkMonitorState_WaitingForSignOut;
}

void VueceConnectionMonitorThread::OnMessage(talk_base::Message *msg)
{
	//Note the only message id in this class is 0, so there is no need to use switch - case block
	//to check the message id
	bool bOnline = false;

	if(msg->message_id != 0)
	{
		LOG(WARNING) << "VueceConnectionMonitorThread:OnMessage - message_id is not 0, sth is wrong.";
		ASSERT(msg->message_id == 0);
	}

	if(!worker_thread_->IsCurrent())
	{
		LOG(WARNING) << "VueceConnectionMonitorThread:OnMessage - current thread is not the worker thread, sth is wrong.";
		ASSERT(worker_thread_->IsCurrent());
	}

	bOnline = CheckConnection();

	switch (iState) {
		case VueceNetworkMonitorState_OnlineSignedIn:
		{
			LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - Current state: VueceNetworkMonitorState_OnlineSignedIn";

			if(bOnline)
			{
				//still online
				iState = VueceNetworkMonitorState_OnlineSignedIn;
				break;
			}
			else
			{
				iState = VueceNetworkMonitorState_Offline;

				LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - Network disconnected";

				ui->OnUserEvent(VueceEvent_Connection_FailedWithAutoReconnect);

				break;
			}

			break;
		}
		case VueceNetworkMonitorState_Offline:
		{
			LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - Current state: VueceNetworkMonitorState_Offline";

			if(!bOnline)
			{
				LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - Still offline, do nothing";
				iState = VueceNetworkMonitorState_Offline;
				break;
			}
			else
			{
//				iReconnectTimer = VUECE_RE_CONNECT_TIMER;
//				LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - Online now, start a re-connect timer: " << iReconnectTimer;
//				iState = VueceNetworkMonitorState_OnlineWaitingForSignIn;

				iState = VueceNetworkMonitorState_Stopped;

				LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_Offline: back online";

				//Notify UI that we can sign in now and stop the monitor thread
				ui->OnUserEvent(VueceEvent_Client_BackOnLine);

				break;
			}

			break;
		}
//		case VueceNetworkMonitorState_OnlineWaitingForSignIn:
//		{
//			iReconnectTimer -= VUECE_CONNECTION_CHECK_INTERVAL_SECOND;
//
//			LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_OnlineWaitingForSignIn with timer: " << iReconnectTimer;
//
//			if(!bOnline)
//			{
//				LOG(INFO) << "Disconnected, no need to re-connect.";
//				iReconnectTimer = 0;
//				iState = VueceNetworkMonitorState_Offline;
//				break;
//			}
//			else
//			{
//				if( iReconnectTimer <= 0)
//				{
//					LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_OnlineWaitingForSignIn, timed out, start sign in now ";
//
//					LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_OnlineWaitingForSignIn - DEBUG 1";
//
////					Stop();
//
//					iState = VueceNetworkMonitorState_Stopped;
//
//					LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_OnlineWaitingForSignIn - DEBUG 2";
//
//					//Notify UI that we can sign in now and stop the monitor thread
//					ui->OnUserEvent(VueceEvent_Client_BackOnLine);
//
//					LOG(INFO) << "VueceConnectionMonitorThread:OnMessage - VueceNetworkMonitorState_OnlineWaitingForSignIn - DEBUG 3";
//
//					return;
//
//				}
//				else
//				{
//					//not timedout yet
//					iState = VueceNetworkMonitorState_OnlineWaitingForSignIn;
//					break;
//				}
//			}
//
//			break;
//		}
		case VueceNetworkMonitorState_WaitingForSignOut:
		{
			break;
		}

		case VueceNetworkMonitorState_Stopped:
		{
			break;
		}

		default:
		{
			break;
		}
	}

	worker_thread_->PostDelayed(VUECE_CONNECTION_CHECK_INTERVAL_SECOND * 1000, this, 0);
}
