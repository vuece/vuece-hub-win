//NOTE this file is not used in ANDROID implementation

#include "talk/base/logging.h"
#include "VueceJingleClientThread.h"
#include "VueceConstants.h"
#include "VueceNativeInterface.h"
#include "VueceLogger.h"
#include "VueceCommon.h"

VueceJingleClientThread::VueceJingleClientThread()
{
	LOG(INFO) << "VueceJingleClientThread constructor called.";

	client = NULL; 

	memset(userName, 0, sizeof(userName));
	memset(userPwd, 0, sizeof(userPwd));

	auth_type = -1;

	LOG(INFO) << "Creating VueceJingleTester.";

	client = vuece::VueceNativeInterface::CreateInstance(NULL);

	ASSERT( client != NULL );

	client->SigUserEvent.connect(this, &VueceJingleClientThread::OnUserEvent);
	client->SigRemoteDeviceActivity.connect(this, &VueceJingleClientThread::OnRemoteDeviceActivity);
	client->SigRosterVCardReceived.connect(this, &VueceJingleClientThread::OnRosterVCardReceived);
	client->SigRosterSubscriptionMsgReceived.connect(this, &VueceJingleClientThread::OnRosterSubscriptionMsgReceived);

	SigFileSystemChanged.connect(client, &vuece::VueceNativeInterface::OnFileSystemChanged);

	LOG(INFO) << "Instance created.";
}

void VueceJingleClientThread::OnRosterSubscriptionMsgReceived(VueceRosterSubscriptionMsg* m)
{
	ui->OnRosterSubscriptionMsgReceived(m);
}

void VueceJingleClientThread::SetAccount(const char* _userName, const char* pwd, const int auth_type_)
{
	memset(userName, 0, sizeof(userName));
	memset(userPwd, 0, sizeof(userPwd));

	/////////////////////////////////////////////////////
	//OATH2 - START
	//enable follwoing code to use real user name and oath2 token
	strcpy(userName, _userName);
	strcpy(userPwd, pwd);
	auth_type = auth_type_;
	//OATH2 - END

	//////////////////////
	//PLAIN AUTH - START (hard code your account here)
	//test code - use following code to overwrite user name and use plain auth
	//strcpy(userName, "");
	//strcpy(userPwd, "");
	//auth_type = 1;
	//PLAIN AUTH - END
}

void* VueceJingleClientThread::Thread()
{

	LOG(INFO) << "Call ThreadStarted()";

	ThreadStarted();

	LOG(INFO) << "VueceJingleClientThread - Thread() started";

	// this is a blocked call.
	client->Start((const char*)userName, (const char*)userPwd, auth_type);

	LOG(INFO) << "in method Thread(): jingle client command loop exited, thread finished";

	if(client != NULL)
	{
		LOG(INFO) << "VueceJingleClientThread::Thread - Deleting client tester...";
		delete client;
		client = NULL;

		LOG(INFO) << "VueceJingleClientThread::Thread - Client tester deleted.";
	}

	//notify UI layer
	LOG(INFO) << "VueceJingleClientThread:Notify UI layer that the client tester instance has been finalized.";
	ui->OnUserEvent(VueceEvent_Client_Destroyed);

	return NULL;
}

void VueceJingleClientThread::TestSendFile()
{
	client->TestSendFile();
}


VueceJingleClientThread::~VueceJingleClientThread()
{
	LOG(INFO) << "VueceJingleClientThread:De-constructor called.";

	if(client != NULL)
	{
		LOG(INFO) << "VueceJingleClientThread:Deleting client tester...";
		delete client;
		client = NULL;

		LOG(INFO) << "VueceJingleClientThread:Client tester deleted.";
	}
}

void VueceJingleClientThread::OnUserEvent(VueceEvent code)
{
	VueceLogger::Debug("VueceJingleClientThread::OnUserEvent - Received an event: %d, forward event to UI layer", code);

	VueceCommon::LogVueceEvent((VueceEvent)code);

	//forward event to UI layer
	ui->OnUserEvent(code);
}

void VueceJingleClientThread::OnRemoteDeviceActivity(VueceStreamingDevice* d)
{
	VueceLogger::Debug("VueceJingleClientThread::OnRemoteDeviceActivity");

	ui->OnRemoteDeviceActivity(d);
}



void VueceJingleClientThread::OnRosterVCardReceived(const std::string& jid, const std::string& fn, const std::string& img_b64)
{
	VueceLogger::Debug("VueceJingleClientThread::OnRosterVCardReceived");

	ui->OnRosterVCardReceived(jid, fn, img_b64);
}

void VueceJingleClientThread::OnFileSystemChanged()
{
	LOG(INFO) << "VueceJingleClientThread::OnFileSystemChanged";
	if(bSignedIn)
	{
		LOG(INFO) << "VueceJingleClientThread::OnFileSystemChanged - Client signed in, notify now.";
		SigFileSystemChanged();
	}
	else
	{
		LOG(INFO) << "VueceJingleClientThread::OnFileSystemChanged - Client has not signed in yet, do nothing.";
	}
}


