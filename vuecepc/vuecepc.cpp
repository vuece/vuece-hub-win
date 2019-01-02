// vuecepc.cpp : main project file.

#include "stdafx.h"
#include "VuecePCMainForm.h"
#include "VueceCommon.h"
#include "VueceGlobalSetting.h"

using namespace vuecepc;

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Diagnostics;
using namespace System::Resources;

static Process ^ RunningInstance() {

	Process ^ current = Process::GetCurrentProcess();

	array<Process^>^ processes = Process::GetProcessesByName (current->ProcessName);

	//Loop through the running processes in with the same name

	for (int i = 0; i < processes->Length; i++) {

		Process ^ process = processes[i];

		//Ignore the current process
		if (process->Id != current->Id)
		{
			current->MainModule->FileName;

			Assembly ^ assem = Assembly::GetExecutingAssembly();

			//Make sure that the process is running from the exe file.
			if (assem->Location->Replace("/", "\\") == current->MainModule->FileName)
			{
				//Return the other process instance.
				return process;
			}
		}
	}

	//No other instance was found, return null.
	return nullptr;
}

static bool CheckAndShowIfRunning() {

	HWND formhwnd;
	bool bFound = false;

	Process ^ current = Process::GetCurrentProcess();

	array<Process^>^ processes = Process::GetProcessesByName (current->ProcessName);

	if (processes->Length > 1) {
		bFound = true;
	}else{
		return false;
	}

	Thread::CurrentThread->CurrentUICulture = CultureInfo::CurrentCulture;//Thread::CurrentThread->CurrentCulture;
	ResourceManager^ resMgr = gcnew ResourceManager("VuecePC.VueceStrings", Assembly::GetExecutingAssembly());

	//Loop through the running processes in with the same name
	for (int i = 0; i < processes->Length; i++) {
		Process ^ process = processes[i];

		//Ignore the current process
		if (process->Id != current->Id) {

			HWND handle = static_cast<HWND>(process->MainWindowHandle.ToPointer());

			if (process->MainWindowHandle.ToInt32() == 0) {
				pin_ptr<const wchar_t> wname = PtrToStringChars(resMgr->GetString(L"String23_MainWinTitle"));

				formhwnd = FindWindow(nullptr,  wname);

				ShowWindow(formhwnd, SW_RESTORE);
//				AnimateWindow(formhwnd, 200, AW_ACTIVATE);
				SwitchToThisWindow(formhwnd, true);
			} else {

				SwitchToThisWindow(handle, true);
			}
		}
	}

	//No other instance was found, return null.
	return bFound;
}

static void HandleRunningInstance(Process^ instance)
{
	HWND handle = static_cast<HWND>(instance->MainWindowHandle.ToPointer());

	ShowWindow(handle, SW_RESTORE);
	SetForegroundWindow(handle);
}


static void  CreateGlobalSettingInstance()
{
	//ONE TIME init
		LOG(INFO) << "CreateGlobalSettingInstance";
		VueceGlobalContext::Init(VueceAppRole_Media_Hub);
}

static void  CreateUserDataFolder()
{
	char buf[1024];

	//ONE TIME init
		LOG(INFO) << "CreateUserDataFolder";

		memset(buf, 0, sizeof(buf));

		if (SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, buf ) != S_OK)
		{
			LOG(LS_ERROR)  << "MAIN STARTUP - Could not retrieve the user's home directory!\n";
		}
		else
		{
			LOG(INFO)  << "MAIN STARTUP - User app data directory = \"" << buf << "\"";

			strcat(buf, "\\Vuece\\");

			LOG(INFO)  << "MAIN STARTUP - Vuece user data directory = \"" << buf << "\"";

			VueceGlobalContext::SetAppUserDataDir(buf);

			talk_base::Pathname userDataPath (VueceGlobalContext::GetAppUserDataDir());
			talk_base::Filesystem::CreateFolder(userDataPath);
		}
}


[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	//first load user setting and check if auto start is allowed
	char buf[1024];
	int len;
	int iJustRebooted = 0;

	CreateGlobalSettingInstance();
	VueceGlobalSetting* mVueceGlobalSetting = VueceGlobalContext::Instance();

	if(mVueceGlobalSetting == NULL)
	{
		return -1;
	}

	memset(buf, 0, sizeof(buf));

	CreateUserDataFolder();

#if  defined(DEBUG)
	VueceCommon::ConfigureLogging(VUECE_LOG_LEVEL);
#endif

	//Retrieve auto login option, at first we need to load and parse
	//saved user data
	len = VueceWinUtilities::LoadUserData(buf, sizeof(buf));

	if(len == -1)
	{
	}
	else
	{
		//parse user data
		VueceWinUtilities::ParseUserData(buf, len, mVueceGlobalSetting);

		LOG(INFO) << "MAIN - User data parsed, appUserDataDir: " << VueceGlobalContext::GetAppUserDataDir();
		LOG(INFO) << "MAIN - User data parsed, accountUserName: " << VueceGlobalContext::GetAccountUserName();
		LOG(INFO) << "MAIN - User data parsed, accountDisplayName: " << VueceGlobalContext::GetAccountDisplayName();
		LOG(INFO) << "MAIN - User data parsed, accountImgUrl: " << VueceGlobalContext::GetAccountImgUrl();
		LOG(INFO) << "MAIN - User data parsed, accountRefreshToken: " <<VueceGlobalContext::GetAccountRefreshToken();
		LOG(INFO) << "MAIN - User data parsed, hubName: " << VueceGlobalContext::GetHubName();
		LOG(INFO) << "MAIN - User data parsed, hubID: " <<VueceGlobalContext::GetHubId();
		LOG(INFO) << "MAIN - User data parsed, number of public folders: " << VueceGlobalContext::GetPublicFolderList()->size();
		LOG(INFO) << "MAIN - User data parsed, re-scan needed: " << VueceGlobalContext::IsRescanNeeded();
		LOG(INFO) << "MAIN - User data parsed, auto login at app startup: " << VueceGlobalContext::GetAutoLoginAtAppStartup();
		LOG(INFO) << "MAIN - User data parsed, auto login at system startup: " << VueceGlobalContext::GetAutoLoginAtSysStartup();
		LOG(INFO) << "MAIN - User data parsed, maximum concurrent streaming: " << VueceGlobalContext::GetMaxConcurrentStreaming();
		LOG(INFO) << "MAIN - User data parsed, allow friend access: " << VueceGlobalContext::GetAllowFriendAccess();


	}

	if(args->Length >= 1)
	{
		//if started during Windows start
		if(args[0] == L"auto"){

			LOG(INFO) << "MAIN - This is an auto-start up when Windows starts.";

			iJustRebooted = 1;

			if(mVueceGlobalSetting->iAutoLoginAtSysStartup != 1)
				{
					LOG(INFO) << "MAIN - but user disabled auto start of this app, return now.";
					return 0;
				}
		}
		else
		{
			LOG(INFO) << "MAIN - This is a normal start up,";
		}
	}


	if( CheckAndShowIfRunning() )
	{
		return 0;
	}
	else
	{
		// Enabling Windows XP visual effects before any controls are created
		Application::EnableVisualStyles();
		Application::SetCompatibleTextRenderingDefault(false);
		// Create the main window and run it
		Application::Run(gcnew VuecePCMainForm(iJustRebooted));
	}

	return 0;
}
