#ifndef _VUECE_JINGLE_TESTER
#define _VUECE_JINGLE_TESTER

#include "vuececonstants.h"
#include "VueceNativeInterface.h"

#include "talk/xmpp/xmppengineimpl.h"
#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "VueceCoreClient.h"
#include "sqlite3.h"

class VueceConsole;
class vuece::VueceMediaItem;

class VueceNativeClientImplWin : public vuece::VueceNativeInterface{

public:
	VueceNativeClientImplWin(vuece::InitData* init_data);
	virtual ~VueceNativeClientImplWin();

	virtual void TestMusicStreaming();

	//log in
	virtual int Start(const char* name, const char* pwd, const int auth_type);
	virtual int LogOut(void);

	//log in state
	virtual vuece::ClientState GetClientState(void);
	virtual void OnClientStateChanged(vuece::ClientEvent event, vuece::ClientState state);


	//hub messages
	virtual void OnVHubGetMessageReceived(const buzz::Jid& jid, const std::string& message);
	virtual void OnVHubResultMessageReceived(const buzz::Jid& jid, const std::string& message);
	virtual int SendVHubMessage(const std::string& to, const std::string& type,const std::string& message);
	virtual int 	SendVHubPlayRequest(const std::string& to, const std::string& type,const std::string& message, const std::string& uri);

	//roster management
	virtual void OnRosterStatusUpdate(const buzz::Status& status);
	virtual void OnRosterReceived(const buzz::XmlElement* stanza);
	virtual void OnRosterSubRespReceived(VueceRosterSubscriptionMsg* m);
	virtual void SendSubscriptionMessage(const std::string& to, int type);
	virtual void AddBuddy(const std::string& jid);
	virtual void SendPresence(const std::string& status, const std::string& signature);
	virtual void SendPresence(const std::string& status);
	virtual void SendSignature(const std::string& sig);
	virtual vuece::RosterMap* GetRosterMap();

	//music playing
	virtual void OnStreamPlayerStateChanged(const std::string& share_id, int state);
	virtual void StopStreamPlayer(const std::string& pos);
	virtual void ResumeStreamPlayer(const std::string& pos);
	virtual vuece::NetworkPlayerState  GetNetworkPlayerState(void);
	virtual void OnNetworkPlayerStateChanged(vuece::NetworkPlayerEvent e, vuece::NetworkPlayerState s);
	virtual bool IsMusicStreaming(void);
	virtual int  GetCurrentPlayingProgress(void);

	//network player
	virtual int Play(const std::string &jid, const std::string &song_uuid);
	virtual int Pause();
	virtual int Resume();
	virtual int Seek(const int position);

	//file share/streaming
	virtual void AcceptFileShare(const std::string& share_id,const std::string& target_folder, const std::string& target_file_name);
	virtual void DeclineFileShare(const std::string& share_id);
	virtual void CancelFileShare(const std::string& share_id);

	virtual void TestSendFile();

	//share events
	virtual void OnFileShareRequestReceived(const std::string& share_id, const buzz::Jid& target, int type,  const std::string& fileName, int size, bool needPreview);
	virtual void OnFileShareProgressUpdated(const std::string& share_id, int percent);
	virtual void OnFileSharePreviewReceived(const std::string& share_id, const std::string& path, int w, int h);
	virtual void OnFileShareStateChanged(const std::string& remote_jid,  const std::string& share_id, int state);
	virtual void OnMusicStreamingProgressUpdated(const std::string& share_id, int progress);
	virtual int  GetCurrentMusicStreamingProgress(const std::string& share_id);
	//call
	virtual cricket::CallOptions* GetCurrentCallOption(void);
	virtual buzz::Jid GetCurrentJidInCall(void);

	//file system
	virtual void OnFileSystemChanged(void);

	virtual void OnRemoteDeviceActivity(VueceStreamingDevice* d);

	void OnVueceEvent(int code, const char* jid, const char* other);

#ifdef VCARD_ENABLED
	virtual void SendVCardRequest(const std::string& to);
	virtual void OnRosterVCardReceived(const std::string& jid, const std::string& fn, const std::string& img_b64);
#endif

#ifdef CHAT_ENABLED
	void SendChat(const std::string& to, const std::string& msg);
	void OnChatMessageReceived(const buzz::Jid&, const std::string&);
#endif

#ifdef MUC_ENABLED
	void SendVCardRequest(const std::string& to);
#endif


	int InitiateMediaStream(
			const std::string& share_id,
			const std::string& jid,
			const std::string& targetUri,
			const std::string& width,
			const std::string& height,
			const std::string& preview_file_path,
			const std::string& start_pos,
			const std::string& need_preview,
			VueceStreamType stream_type,
			vuece::VueceMediaItem* targetItem // NULL if this is a normal file share
			);

private:
	void ShareMusicDBFile(const buzz::Jid& jid);
	void LogClientState(void);

	sigslot::signal2<int, const char*> SigVueuceCommandMessage;

	VueceCoreClient* client;
	VueceConsole* console;
	VueceEvent exitReason;
	sqlite3 *mediaDB;
	bool bTestFlag;
};

#endif
