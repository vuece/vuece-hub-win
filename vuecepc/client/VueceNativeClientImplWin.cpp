#include "VueceNativeClientImplWin.h"

#include <cstdio>
#include <iostream>
#include <time.h>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "talk/xmpp/constants.h"
#include "talk/base/logging.h"
#include "talk/base/flags.h"
#include "talk/base/pathutils.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/helpers.h"


#include "talk/base/base64.h"
#include "talk/base/stringdigest.h"
#include "talk/session/fileshare/VueceMediaStream.h"


#include "VueceCoreClient.h"

#include "VueceConstants.h"
#include "VueceGlobalSetting.h"
#include "VueceMediaDBManager.h"
#include "VueceMediaItem.h"
#include "VueceLogger.h"

#include "libjson.h"
#include "json/json.h"

#include "tinyxml2.h"

#include "DebugLog.h"

using namespace tinyxml2;
using namespace vuece;

static const int DEFAULT_PORT = 5222;

VueceNativeClientImplWin::VueceNativeClientImplWin(vuece::InitData* init_data) {

	LOG(INFO) << "VueceNativeClientImplWin()";
	client = NULL;
	exitReason = VueceEvent_Client_SignedOut;

//	std::string path_s("C:\\Users\\jj\\Pictures\\XXX.jpg");
//
//	talk_base::Pathname path(path_s);
//
//	LOG(LS_VERBOSE) << "TEST - pathname is: " << path.pathname();
//	LOG(LS_VERBOSE) << "TEST - url is: " << path.url();
//	LOG(LS_VERBOSE) << "TEST - folder is: " << path.folder();
//	LOG(LS_VERBOSE) << "TEST - folder_name is: " << path.folder_name();
//	LOG(LS_VERBOSE) << "TEST - parent_folder is: " << path.parent_folder();
//	LOG(LS_VERBOSE) << "TEST - basename is: " << path.basename();
//	LOG(LS_VERBOSE) << "TEST - extension is: " << path.extension();
//	LOG(LS_VERBOSE) << "TEST - filename is: " << path.filename();

	//TestMusicStreaming();

}

VueceNativeClientImplWin::~VueceNativeClientImplWin() {

	LOG(INFO) << "VueceNativeClientImplWin - De-constructor called, exit reason: " << exitReason;

	//NOTE: DONT TRY TO DELETE ANY THING HERE!!!!
}

RosterMap* VueceNativeClientImplWin::GetRosterMap() {
	if (client == NULL) {
		return NULL;
	}

	return client->GetRosterMap();
}

void VueceNativeClientImplWin::AddBuddy(const std::string& jid) {
	if (client == NULL) {
		return;
	}

	client->InviteFriend(jid);
}

int VueceNativeClientImplWin::InitiateMediaStream(
		const std::string& share_id,
		const std::string& jid,
		const std::string& targetUri,
		const std::string& width,
		const std::string& height,
		const std::string& preview_file_path,
		const std::string& start_pos,
		const std::string& need_preview,
		VueceStreamType stream_type,
		VueceMediaItem* mediaItem
		)
{
	int errCode = VueceStreamSessionInitErrType_None;

	LOG(INFO) << "---------------------------------------------------------------------------------------------------------------";
	LOG(INFO)
		<< "VueceNativeClientImplWin::InitiateMediaStream: VueceShareId=" << share_id << ", Jid=" << jid << ", TargetUri=" << targetUri;
	LOG(INFO) << "---------------------------------------------------------------------------------------------------------------";

	//here we use ascii code FS to separate param1 and param2 and param3 in one string
	switch(stream_type)
	{
	/*
	 * 		const std::string& share_id,
		const std::string& jid,
		const std::string& targetUri,
		const std::string& width,
		const std::string& height,
		const std::string& preview_file_path,
		const std::string& start_pos,
		const std::string& need_preview,
	 */
		case VueceStreamType_File:
		{
			errCode = client->SendFile(share_id, jid, targetUri, width, height, preview_file_path, start_pos, need_preview);
			break;
		}

		case VueceStreamType_Music:
		{
			errCode = client->InitiateMusicStreamDB(share_id, jid, targetUri, width, height, preview_file_path, start_pos, need_preview, mediaItem);
			break;
		}

		default:
		{
			break;
		}
	}

	return errCode;
}

void VueceNativeClientImplWin::TestSendFile()
{
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin::TestSendFile - Start";

	std::string previewImagePath("C:\\Users\\jj\\Pictures\\1.JPG");
	std::string start_position = "0";
	std::string need_preview = "0";

	LOG(LS_INFO) << "Preview image path is: " << previewImagePath ;
}


void VueceNativeClientImplWin::CancelFileShare(const std::string& share_id){
	LOG(INFO) << "VueceNativeClientImplWin::CancelFileShare: " << share_id;
	SigVueuceCommandMessage(VUECE_CMD_CANCEL_FILE, share_id.c_str());
}

void VueceNativeClientImplWin::AcceptFileShare(const std::string& share_id,const std::string& target_folder, const std::string& target_file_name){
	//dummy value for PC impl
	std::string sample_rate("0");
	LOG(INFO) << "VueceNativeClientImplWin::AcceptFileShare: " << share_id << "; sample rate: " << sample_rate;

	int i;
	char buf[128];

	//here we use ascii code FS to separate param1 and param2 and param3 in one string
	memset(buf, '\0', sizeof(buf));
	strcpy(buf, share_id.c_str());

	i = strlen(buf);
	buf[i] = 0x1C;
	strcat(buf, sample_rate.c_str());

	i = strlen(buf);
	buf[i] = 0x1C;
	strcat(buf, target_folder.c_str());

	i = strlen(buf);
	buf[i] = 0x1C;
	strcat(buf, target_file_name.c_str());

	SigVueuceCommandMessage(VUECE_CMD_ACCEPT_FILE, buf);

}

void VueceNativeClientImplWin::DeclineFileShare(const std::string& share_id){
	LOG(INFO) << "VueceNativeClientImplWin::DeclineFileShare: " << share_id;
	SigVueuceCommandMessage(VUECE_CMD_DECLINE_FILE, share_id.c_str());
}

void VueceNativeClientImplWin::SendSubscriptionMessage(const std::string& jid, int type) {
	if (client == NULL) {
		return;
	}
	client->SendSubscriptionMessage(jid, type);
}

void VueceNativeClientImplWin::SendPresence(const std::string& status)
{
	if (client == NULL) {
		return;
	}

	client->SendPresence(status);
}

void VueceNativeClientImplWin::SendPresence(const std::string& status, const std::string& signature) {
	if (client == NULL) {
		return;
	}

	client->SendPresence(status, signature);
}



void VueceNativeClientImplWin::SendSignature(const std::string& sig) {
	if (client == NULL) {
		return;
	}

	client->SendSignature(sig);
}

vuece::ClientState VueceNativeClientImplWin::GetClientState(void)
{
	return client->GetClientState();
}


int VueceNativeClientImplWin::Start(const char* name, const char* pwd,  const int auth_type) {

	bool debug = true;
	bool bIsLoginFailed = false;
	std::string protocol = "hybrid";
	std::string secure = "disable";

	LOG(INFO) << "VueceNativeClientImplWin::SignIn - Start";

	if(client != NULL)
	{
		VueceLogger::Fatal("VueceNativeClientImplWin::Start - VueceCoreClient instance should be null, abort now!");
		return RESULT_FUNC_NOT_ALLOWED;
	}

	//bbbbbbbbbbbbbbbbbbbbbbbb
	client = new VueceCoreClient(this, name);

    SigVueuceCommandMessage.connect(client, &VueceCoreClient::OnVueceCommandReceived);

    client->SignalVueceEvent.connect(this, &VueceNativeClientImplWin::OnVueceEvent);

	//a blocking call
	client->Start(name, pwd,  auth_type);

	delete client;

	client = NULL;

	LOG(INFO) << "====================================================";
	LOG(INFO) << "VueceNativeClientImplWin::Start:Call client finalized!!!!!";
	LOG(INFO) << "====================================================";

	if(bIsLoginFailed)
	{
		return RESULT_UNAUTHORIZED;
	}

	return RESULT_OK;
}

//Note - Make sure this method call is the ONLY entry point of logging out precedure.
int VueceNativeClientImplWin::LogOut()
{
	LOG(INFO) << "VueceNativeClientImplWin::LogOut";

	return client->LogOut();
}

void VueceNativeClientImplWin::OnClientStateChanged(vuece::ClientEvent event, vuece::ClientState state)
{
	LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - event = " << event << ", state = " << state ;

	VueceNativeInterface::LogClientEvent(event);
	VueceNativeInterface::LogClientState(state);

	switch(event)
	{
	case CLIENT_EVENT_LOGGING_IN:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - LOGGING_IN";

		break;
	}
	case CLIENT_EVENT_LOGIN_OK:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - LOGGING_OK";
		SigUserEvent(VueceEvent_Client_SignedIn);

		break;
	}
	case CLIENT_EVENT_AUTH_ERR:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - AUTH_ERR";
		exitReason = VueceEvent_Client_AuthFailed;
		SigUserEvent(VueceEvent_Client_AuthFailed);
		break;
	}
	case CLIENT_EVENT_NETWORK_ERR:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - NETWORK_ERR";
		exitReason = VueceEvent_Connection_Failed;
		//Note - This call will eventually make the UI layer trigger a call to LogOut on VueceCoreClient instance
		SigUserEvent(VueceEvent_Connection_Failed);
		break;
	}
	case CLIENT_EVENT_LOGGING_OUT:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - LOGGING_OUT";

		break;
	}

	case CLIENT_EVENT_LOGOUT_OK:
	{
		LOG(INFO) << "VueceNativeClientImplWin::OnClientStateChanged - LOGGING_OK";
		SigUserEvent(VueceEvent_Client_SignedOut);

		break;
	}

	}
}

void VueceNativeClientImplWin::OnVueceEvent(int code, const char* jid, const char* other)
{
	std::string jid_s (jid);

	LOG(INFO) << "VueceNativeClientImplWin::OnVueceEvent, code = " << code << ", jid = " << jid << ", other value = " << other;

	switch(code)
	{

	//TODO - Use a separate signal to handle this event, don't mix it with FSM
	case VueceEvent_FileAccess_Denied:
	{
		std::ostringstream os;

		os <<  "{'action':'play', 'reply':'error','category':'music','control':'start','reason':'Access denied.', 'uri':'";
		os << other;
		os << "'}";

		LOG(INFO) << "VueceNativeClientImplWin::OnVueceEvent - VueceEvent_FileAccess_Denied";

		client->SendVHubMessage(jid_s, "result", os.str());
		break;
	}

	default:
	{
		VueceLogger::Fatal("VueceNativeClientImplWin::OnVueceEvent - Unexpected event notification, code: %d, abort now.", code);
		break;
	}
	}
}

int VueceNativeClientImplWin::SendVHubMessage(const std::string& to, const std::string& type,const std::string& message)
{
	return RESULT_GENERAL_ERR;
}

void VueceNativeClientImplWin::OnRosterStatusUpdate(const buzz::Status& status) {

//	LOG(INFO) << "VueceNativeClientImplWin: OnRosterStatusUpdate";
//	LOG(INFO) << "VueceNativeClientImplWin:OnRosterStatusUpdate:Jid:[" << status.jid().Str().c_str() << "]";
//	LOG(INFO) << "VueceNativeClientImplWin:OnRosterStatusUpdate:Status:[" << status.status().c_str() << "]";
//
//#ifdef VCARD_ENABLED
//	SendVCardRequest(status.jid().Str());
//#endif
}


#ifdef CHAT_ENABLED
void VueceNativeClientImplWin::SendChat(const std::string& to, const std::string& msg) {
	if (client == NULL) {
		return;
	}
	client->SendChat(to, msg);
}
void VueceNativeClientImplWin::OnChatMessageReceived(const buzz::Jid& from, const std::string& msg) {
	LOG(LS_VERBOSE) << "OnChatMessageReceived:Received a msg from: " << from.node();
	LOG(LS_VERBOSE) << "OnChatMessageReceived:Message content:\n-------------------\n" <<
		msg << "\n-------------------\n";
}
#endif

#ifdef VCARD_ENABLED
void VueceNativeClientImplWin::OnRosterVCardReceived(const std::string& jid, const std::string& fn, const std::string& img_data)
{
	LOG(LS_VERBOSE) << "OnRosterVCardReceived:Received a vcard query result for: " << jid;
	LOG(LS_VERBOSE) << "OnRosterVCardReceived:Full name:" << fn;
	SigRosterVCardReceived(jid, fn, img_data);
}

void VueceNativeClientImplWin::SendVCardRequest(const std::string& to) {
	if (client == NULL) {
		return;
	}
	client->SendVCardRequest(to);
}
#endif

void VueceNativeClientImplWin::ShareMusicDBFile(const buzz::Jid& jid)
{

	std::string db_path = VueceMediaDBManager::GetFullDBFilePath();

	LOG(LS_VERBOSE) << "VueceNativeClientImplWin:ShareMusicDBFile - Full db file path: " << db_path;

	InitiateMediaStream(
			talk_base::ToString(talk_base::CreateRandomId()), //session id
			jid.Str(), //jid
			db_path.c_str() , //file path
			"0", //image width
			"0", //image height
			"", //preview image path
			"0", //start position in seconds
			"0",
			VueceStreamType_File,
			NULL
			);
}

void VueceNativeClientImplWin::OnVHubGetMessageReceived(const buzz::Jid& jid, const std::string& message)
{
	LOG(LS_INFO)
		<< "VueceNativeClientImplWin::VHub Msg Received - jid: " << jid.Str().c_str() << ", message: " << message;

	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	std::string msg(message);
	std::replace( msg.begin(), msg.end(), '\'', '"');
	bool parsingSuccessful = reader.parse( msg.c_str(), root );
	VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();
	const char* accountUserName = VueceGlobalContext::GetAccountUserName();
	char remote_bare_jid[128+1];
	bool access_allowed = false;
	bool config_allowFriendsAccess = (bool)VueceGlobalContext::GetAllowFriendAccess();

	memset(remote_bare_jid, 0, sizeof(remote_bare_jid));
	strcpy(remote_bare_jid, jid.BareJid().Str().c_str());

	if ( !parsingSuccessful )
	{
		// report to the user the failure and their locations in the document.
		LOG(LS_ERROR)  << "VueceNativeClientImplWin::VHub Msg Received - Failed to parse configuration\n"
				   << reader.getFormattedErrorMessages();
		return;
	}

	std::string action = root.get("action","").asString();

	LOG(LS_VERBOSE) << "VueceNativeClientImplWin::VHub Msg Received - action: " << action << ", checking access now...";
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Performing access control - Owner's hub jid: " << accountUserName << ", bare_remote_jid: " << remote_bare_jid;

	if(strcmp(accountUserName, remote_bare_jid) != 0)
	{
		LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Remote client is not owner's device: " << remote_bare_jid;

		if(!config_allowFriendsAccess)
		{
			LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Remote client is not owner's device and access is NOT allowed";

			access_allowed = false;
		}
		else
		{
			LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Remote client is not owner's device but access is allowed";

			access_allowed = true;
		}
	}
	else
	{
		LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Remote client is running on owner's device";

		access_allowed = true;
	}

	if(access_allowed)
	{
		LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Access is allowed";
	}
	else
	{
		char err_resp [128+1];

		memset(err_resp, 0, sizeof(err_resp));

		LOG(LS_VERBOSE) << "VueceNativeClientImplWin::Access is NOT allowed, sending error response";

		sprintf(err_resp, "{'action':'%s', 'reply':'noaccess','reason':'You are not allowed to access this hub.'}", action.c_str());

		client->SendVHubMessage(jid.Str().c_str(), "result",  err_resp);

		return;
	}

	if (action.compare("probe")==0)
	{
		    //get all categories, etc. music, video, camera
			std::string db_md5	 = VueceMediaDBManager::RetrieveDBFileChecksum(false);

			std::string message="{'action':'probe', 'reply':'ok','categories':['music'],'db-cache-chksum':'";
			message.append(db_md5);
			message.append("'}");

			client->SendVHubMessage(jid.Str().c_str(), "result", message);

		//TODO - TEST CODE REMOVE LAER
		//Test normal file transfer
//		InitiateMediaStream(
//				talk_base::ToString(talk_base::CreateRandomId()), //session id
//				jid.Str(), //jid
//				"P:\\Dropbox\\Photos\\IMG_0085.JPG", //file path
//				"0", //image width
//				"0", //image height
//				"", //preview image path
//				"0", //start position in seconds
//				"0",
//				VueceStreamType_File
//				);

		//DB transfer test code - remove later
//		ShareMusicDBFile(jid);
	}
	else if (action.compare("browse")==0)
	{
		// browse resources
		std::string category = root.get("category","").asString();
		std::string uri = root.get("uri","").asString();

		LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - category: " << category;

		if (category.compare("music")==0)
		{
			std::ostringstream resp;

			//TODO - Since we are using md5 string now, we don't need to base64 coded item id.
//----------------------------------------------------------------------------------------------
//			Test code - keep it for now
//			std::string samplemsg = "{'action':'browse','reply':'ok','category':'music','list':[";
//			samplemsg.append("{'name':'The Man Comes Around','bitrate':192,'artist':'Johnny Cash','album':'Johnny Cash','length':267,'size':5049287,'uri':'The Man Comes Around.flv'},");
//			samplemsg.append("{'name':'Adele - Someone Like You','bitrate':128,'artist':'Adele','album':'Adele','length':284,'size':4550819,'uri':'Adele - Someone Like You.mp3'},");
//			samplemsg.append("{'name':'GLAY  I LOVE YOU','bitrate':128,'artist':'GLAY','album':'GLAY','length':317,'size':32134370,'uri':'GLAY  I LOVE YOU.flv'},");
//			samplemsg.append("{'name':'PSY - GANGNAM STYLE MV','bitrate':128,'artist':'PSY','album':'PSY','length':252,'size':183190475 ,'uri':'PSY - GANGNAM STYLE MV.mp4'},");
//			samplemsg.append("{'name':'Coldplay - Paradise','bitrate':192,'artist':'Coldplay','album':'Coldplay','length':260,'size':9162070,'uri':'Coldplay - Paradise.mp3'}]}");
//			client->SendVHubMessage(jid.Str().c_str(), "result", samplemsg);
//----------------------------------------------------------------------------------------------

			//DB version:
			LOG(INFO) << "VueceNativeClientImplWin::VHub Msg Received  - Start browsing target uri: " << uri;

			client->BrowseMediaItem(uri, resp);

			LOG(INFO) << "VueceNativeClientImplWin::VHub Msg Received  - Shared media info string len: " << resp.str().length();
			LOG(INFO) << "VueceNativeClientImplWin::VHub Msg Received  - Shared media info string: " << resp.str();

			client->SendVHubMessage(jid.Str().c_str(), "result", resp.str());
		}
	}
	else if (action.compare("play")==0)
	{
		// control
		std::string category = root.get("category","").asString();
		std::string control = root.get("control","").asString();
		std::string uri = root.get("uri","").asString();

		LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - category: " << category  << ", control: " << control << ", uri: " << uri;

       if (uri.compare("")==0)
		{
				std::string message="{'action':'play', 'reply':'error','category':'music','control':'"+control+"', 'reason':'URI cannot be empty'}";
				client->SendVHubMessage(jid.Str().c_str(), "result", message);
		}
		else
		{
			if (category.compare("music")==0)
			{
					LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - This message belongs to MUSIC category.";

					if (control.compare("start")==0)
					{
							//TODO - Implement error handling here
							//If we are currently streaming file and meanwhile we receive another play request from remote client
							//this is unexpected because remote client is supposed to send a STOP before sending a new START, but
							//we don't trust remote client so we need to protect ourselves from crashing, so in this case, we abort
							//current streaming and return an ACCESS DENIED response to remove client, the we go back to idle state

							std::string previewImagePath("");

							LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - Received a START request from remote client, preview image path is: "  << previewImagePath ;

							std::string start_position = root.get("start","0").asString();
							std::string need_preview = root.get("need_preview","0").asString();

							LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - start_position: " << start_position << ";need preview?" << need_preview;

							LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - Enter media streaming mode.";

							//query the actual file path according to the file URL
							//if target item cannot be found, a vuece event VueceEvent_FileAccess_Denied
							//will be fired and error response will be sent back
//							std::string absFilePath = client->LocateMediaItemInDB(jid.Str().c_str(), uri);

							VueceMediaItem* targetItem = client->LocateMediaItemInDB(jid.Str().c_str(), uri);

							if(targetItem != NULL)
							{
									int errCode = VueceStreamSessionInitErrType_None;

									if(targetItem->IsFolder())
									{
										LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - This is folder, cannot be played, return an error. ";
										errCode = VueceStreamSessionInitErrType_NotAFile;
									}
									else
									{
										std::string absFilePath = talk_base::Base64::Decode(targetItem->Path(), talk_base::Base64::DO_STRICT);

										errCode = InitiateMediaStream(
												talk_base::ToString(talk_base::CreateRandomId()), //session id
												jid.Str().c_str(), 	//jid
												absFilePath,//uri, 						//target uri
												"128", 					//image width
												"128",					 //image height
												previewImagePath.c_str(),		//preview image path
												start_position,							//start position in seconds
												need_preview,							//need preview or not,
												VueceStreamType_Music,
												targetItem
												);

										LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - InitiateMediaStream() returned: " << errCode;
									}

									if(errCode != VueceStreamSessionInitErrType_OK)
									{
										LOG(LS_INFO) << "VueceNativeClientImplWin::VHub Msg Received - Target cannot be found or played, return an error msg now.";

										std::string message = "";

										if(errCode == VueceStreamSessionInitErrType_NodeBusy)
										{
											message="{'action':'play', 'reply':'error','category':'music','control':'"+control+"', 'reason':'" + VHUB_MSG_BUSY_NOW +"'}";
										}
										else
										{
											  message="{'action':'play', 'reply':'error','category':'music','control':'"+control+"', 'reason':'" + VHUB_MSG_STREAMING_TARGET_INVALID +"'}";
										}

										client->SendVHubMessage(jid.Str().c_str(), "result", message);
									}

							}
					}
			}
			else
			{
				std::string message="{'action':'play', 'reply':'error','category':'unknown','control':'"+control+"', 'reason':'Invalid request'}";
				client->SendVHubMessage(jid.Str().c_str(), "result", message);
			}
		}
	}
	else if (action.compare("sync-cache")==0)
	{
		client->SendVHubMessage(jid.Str().c_str(), "result", "{'action':'sync-cache', 'reply':'ok'}");
		ShareMusicDBFile(jid);
	}
}


void VueceNativeClientImplWin::OnVHubResultMessageReceived(const buzz::Jid& jid, const std::string& message)
{
	LOG(LS_INFO)
		<< "VueceNativeClientImplWin::OnVHubResultMessageReceived:jid: " << jid.Str().c_str() << ", message: " << message << " - Empty impl";
}


void VueceNativeClientImplWin::OnRosterSubRespReceived(VueceRosterSubscriptionMsg* m) {
	LOG(LS_VERBOSE) << "OnRosterSubRespReceived";
//	VueceRosterSubscriptionType type = VueceRosterSubscriptionType_NA;
//
//	std::string messageType = stanza->Attr(buzz::QN_TYPE);
//	std::string from = stanza->Attr(buzz::QN_FROM);
//
//	LOG(LS_VERBOSE) << "OnRosterSubRespReceive::Source: " << from << ", type: " << messageType;
//	VueceLogger::Warn("OnRosterSubRespReceive::Source: %s, type: %s", from.c_str(), messageType.c_str());
//
//	if (messageType == buzz::STR_SUBSCRIBED) {
//		LOG(LS_VERBOSE) << "OnRosterSubRespReceived:Message type: subscribed";
//		type = VueceRosterSubscriptionType_Subscribed;
//	} else 	if (messageType == buzz::STR_SUBSCRIBE) {
//		LOG(LS_VERBOSE) << "OnRosterSubRespReceived:Message type: subscribe";
//		type = VueceRosterSubscriptionType_Subscribe;
//	} else 	if (messageType == buzz::STR_UNSUBSCRIBE) {
//		LOG(LS_VERBOSE) << "OnRosterSubRespReceived:Message type: unsubscribe";
//		type = VueceRosterSubscriptionType_Unsubscribe;
//	} else 	if (messageType == buzz::STR_UNSUBSCRIBED) {
//		LOG(LS_VERBOSE) << "OnRosterSubRespReceived:Message type: unsubscribed";
//		type = VueceRosterSubscriptionType_Unsubscribed;
//	} else 	if (messageType == buzz::STR_UNAVAILABLE) {
//		LOG(LS_VERBOSE) << "OnRosterSubRespReceived:Message type: unavailable";
//		type = VueceRosterSubscriptionType_Unavailable;
//	}
//
//	VueceRosterSubscriptionMsg m;
//	memset(&m, 0, sizeof(m));
//
//	strcpy(m.user_id, from.c_str());
//	m.subscribe_type = type;

	SigRosterSubscriptionMsgReceived(m);
}		

void VueceNativeClientImplWin::OnRosterReceived(const buzz::XmlElement* stanza) {
	LOG(INFO) << "OnRosterReceived - Do nothing";
//	int rosterSize = 0;
//	int len = 0;
//	const buzz::XmlElement * query = stanza->FirstNamed(buzz::QN_ROSTER_QUERY);
//	const buzz::XmlElement* rosterItem = query->FirstNamed(buzz::QN_ROSTER_ITEM);
//	while (rosterItem) {
//		//LOG(LS_VERBOSE) << "rosterItem: " << rosterItem->Str();
//		rosterSize++;
//		rosterItem = rosterItem->NextNamed(buzz::QN_ROSTER_ITEM);
//	}

}

void VueceNativeClientImplWin::OnFileSharePreviewReceived(const std::string& share_id, const std::string& path, int w, int h)
{
	LOG(LS_VERBOSE)
		<< "VueceNativeClientImplWin:OnFileSharePreviewReceived:share_id: " << share_id << ", path: " << path << ", w: " << w << ", h: " << h<<  ", do nothing";
}

void VueceNativeClientImplWin::OnFileShareProgressUpdated(const std::string& share_id, int percent)
{
	LOG(LS_VERBOSE)
		<< "VueceNativeClientImplWin:OnFileShareProgressUpdated:share_id: " << share_id << ", progress: " << percent <<  ", do nothing";
}

void VueceNativeClientImplWin::OnFileShareStateChanged(const std::string& remote_jid,  const std::string& sid, int state)
{
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin:OnFileShareStateChanged:state: " << state << ", remote_jid: " << remote_jid << ", session id: " << sid;

	if(state == (int)cricket::FS_RESOURCE_RELEASED)
	{
		std::ostringstream os;

		LOG(LS_VERBOSE) << "VueceNativeClientImplWin:OnFileShareStateChanged:state - FS_RESOURCE_RELEASED";

		os <<  "{'action':'notification', 'category':'music', 'type':'streaming-resource-released', 'sid':'";
		os << sid;
		os << "'}";

		client->RemoveStreamingNode(remote_jid);

		client->SendVHubMessage(remote_jid, "get", os.str());
	}
	else
	{
		//do nothing for now if the notification state is not FS_RESOURCE_RELEASED
		LOG(LS_VERBOSE) << "VueceNativeClientImplWin:OnFileShareStateChanged:state - do nothing for now because the notification state is not FS_RESOURCE_RELEASED";
	}
}

void VueceNativeClientImplWin::OnFileShareRequestReceived(const std::string& share_id, const buzz::Jid& target, int type, const std::string& fileName, int size, bool needPreview)
{
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin:OnFileShareRequestReceived:fileName: " << fileName;
}

void VueceNativeClientImplWin::OnFileSystemChanged()
{
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin::OnFileSystemChanged";
}

void VueceNativeClientImplWin::OnRemoteDeviceActivity(VueceStreamingDevice* d)
{
	LOG(LS_VERBOSE) << "VueceNativeClientImplWin::OnRemoteDeviceActivity - User id: " << d->user_id
			<< ", Activity type: " << d->activity << ", File url: " << d->file_url;

	SigRemoteDeviceActivity(d);
}

//music playing
void VueceNativeClientImplWin::OnStreamPlayerStateChanged(const std::string& share_id, int state)
{
	LOG(LS_INFO) << "VueceNativeClientImplWin::OnStreamPlayerStateChanged:share_id: " << share_id << ", state: " << state<<  ", do nothing";

}

void VueceNativeClientImplWin::StopStreamPlayer(const std::string& pos)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - StopStreamPlayer is not supported, abort!");
}
void VueceNativeClientImplWin::ResumeStreamPlayer(const std::string& pos)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - ResumeStreamPlayer is not supported, abort!");
}

vuece::NetworkPlayerState VueceNativeClientImplWin::GetNetworkPlayerState(void)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - GetNetworkPlayerState is not supported, abort!");
	return vuece::NetworkPlayerState_Idle;
}

void VueceNativeClientImplWin::OnNetworkPlayerStateChanged(vuece::NetworkPlayerEvent e, vuece::NetworkPlayerState s)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - OnNetworkPlayerStateChanged is not supported, abort!");
}

bool VueceNativeClientImplWin::IsMusicStreaming(void)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - IsMusicStreaming is not supported, abort!");
	return false;
}

int  VueceNativeClientImplWin::GetCurrentPlayingProgress(void)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - GetCurrentPlayingProgress is not supported, abort!");
	return -1;
}

int VueceNativeClientImplWin::Play(const std::string &jid, const std::string &song_uuid)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - Play is not supported, abort!");
	return -1;
}
int VueceNativeClientImplWin::Pause()
{
	VueceLogger::Fatal("VueceNativeClientImplWin - Pause is not supported, abort!");
	return -1;
}
int VueceNativeClientImplWin::Resume()
{
	VueceLogger::Fatal("VueceNativeClientImplWin - Resume is not supported, abort!");
	return -1;
}
int VueceNativeClientImplWin::Seek(const int position)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - Seek is not supported, abort!");
	return -1;
}

void VueceNativeClientImplWin::OnMusicStreamingProgressUpdated(const std::string& share_id, int progress)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - OnMusicStreamingProgressUpdated is not supported, abort!");
}

int  VueceNativeClientImplWin::GetCurrentMusicStreamingProgress(const std::string& share_id)
{
	VueceLogger::Fatal("VueceNativeClientImplWin - GetCurrentMusicStreamingProgress is not supported, abort!");
	return -1;
}

cricket::CallOptions* VueceNativeClientImplWin::GetCurrentCallOption(void)
{
	return NULL;
}

buzz::Jid VueceNativeClientImplWin::GetCurrentJidInCall(void)
{
	buzz::Jid jid;

	return jid;
}

void VueceNativeClientImplWin::TestMusicStreaming()
{
	//input not used for sender
	int sample_rate = -1;
	int bit_rate = -1;
	int nchannels = -1;
	int duration = -1;

	bool fOpend = 0;

	char readBuf[2048];
	size_t read_len = 0;
	int read_err;

	talk_base::StreamResult sr =talk_base:: SR_ERROR;

	LOG(LS_VERBOSE) << "VueceNativeClientImplWin:TestMusicStreaming - Start";

	talk_base::VueceMediaStream* file = new talk_base::VueceMediaStream("", sample_rate, bit_rate, nchannels, duration);

	fOpend = (file->Open("D:\\STREAM_SAMPLES\\SHORT.mp3", "rb", 0));

	if ( !fOpend )
	{
		LOG(LS_ERROR) << "VueceNativeClientImplWin:OnHttpRequest:Meida hub - File cannot be opened";
		return;
	}

	while(true)
	{
		sr = file->Read(readBuf, 2048, &read_len, &read_err);

		if(sr == talk_base::SR_ERROR || sr == talk_base::SR_EOS)
		{
			break;
		}
	}

	LOG(LS_VERBOSE) << "VueceNativeClientImplWin:TestMusicStreaming - Done";
}

int  VueceNativeClientImplWin::SendVHubPlayRequest(const std::string& to, const std::string& type,const std::string& message, const std::string& uri)
{
	return 0;
}

