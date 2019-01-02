#pragma once

#include "VueceNativeInterface.h"
#include "VueceJingleClientThread.h"
#include "VueceConnectionMonitorThread.h"
#include "VueceMediaDBScannerThread.h"
#include "talk/base/logging.h"
#include "talk/base/pathutils.h"
#include "VueceGlobalSetting.h"
#include "VueceDirectoryMonitor.h"
#include "VueceUI.h"
#include "VueceCrypt.h"
#include "VueceProgressDialog.h"
#include "VueceAboutDialog.h"
#include "VueceUpdateDialog.h"
#include "VueceAuthDialog.h"
#include "VueceWinUtilities.h"
#include "VueceHubVersion.h"
#include "tinyxml2.h"
#include "Mmsystem.h"
#include <ShlObj.h>
#include <vcclr.h>
#include <wininet.h>
#include "resource.h"

using namespace tinyxml2;

#define IMG_IDX_USER 14
#define LOGOUT_URL "https://www.google.com/accounts/logout"

VueceJingleClientThread* jingleClientThread;
VueceMediaDBScannerThread* mediaScannerThread;
VueceConnectionMonitorThread* connectionMonitorThread;

	namespace vuecepc {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;
	using namespace System::Text;
	using namespace Microsoft::Win32;
	using namespace System::Runtime::InteropServices;
	using namespace System::Resources;
	using namespace System::Reflection;
	using namespace System::Globalization;
	using namespace System::Threading;
	using namespace System::Security::Permissions;
	using namespace System::Diagnostics;
	using namespace System::ComponentModel;
	using namespace System::Drawing;

	using namespace VueceFileSystem;

	/// <summary>
	/// Summary for VuecePCMainForm
	/// </summary>
	public ref class VuecePCMainForm : public System::Windows::Forms::Form, VueceUI
	{
	public:
		VuecePCMainForm(int iJustRebooted)
		{
			char buf[1024];
			char buf2[128];

			std::string stdTmpString ("");
			int len = 0;

			memset(buf, 0, sizeof(buf));
			memset(buf2, 0, sizeof(buf2));

			// Set the user interface to display in the
			// same culture as that set in Control Panel.

			//Thread::CurrentThread->CurrentUICulture = CultureInfo::CurrentCulture;

			//Thread::CurrentThread->CurrentUICulture = gcnew CultureInfo("en");
			//Thread::CurrentThread->CurrentUICulture = gcnew CultureInfo("zh-CN");

			String^ cultureName = CultureInfo::CurrentCulture->DisplayName;
			String^ cultureNameEn = CultureInfo::CurrentCulture->EnglishName;

			CreateMutex (NULL, FALSE, L"VueceMusicHub2015");

			//Note - this culture/language setting must be done before InitializeComponent()
			resMgr = gcnew ResourceManager("VuecePC.VueceStrings", Assembly::GetExecutingAssembly());

			InitializeComponent();

			VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

			//PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

			//
			//TODO: Add the constructor code here
			//

			//overwrite the title text with String23_MainWinTitle
			this->Text = resMgr->GetString(L"String23_MainWinTitle");

			VueceWinUtilities::MarshalString(cultureName, stdTmpString);

			LOG(LS_VERBOSE) << "MAIN STARTUP - cultureName: " << stdTmpString;

			 VueceWinUtilities::MarshalString(cultureNameEn, stdTmpString);

			LOG(LS_VERBOSE) << "MAIN STARTUP - cultureName(en): " << stdTmpString;

			bSignedIn = false;
			bSignedOutByUser = false;
			bFirstSignIn = true;

			connectionMonitorThread = NULL;
			jingleClientThread = NULL;
			mediaScannerThread = NULL;

			jingleClientThread = new VueceJingleClientThread();
			jingleClientThread->ui = this;
			
			memset(buf, 0, sizeof(buf));

			if(strlen(VueceGlobalContext::GetAccountUserName()) > 0
					&& strlen(VueceGlobalContext::GetAccountDisplayName()) > 0)
			{
				VueceGlobalContext::SetAccountLinked(true);
				LOG(INFO) << "MAIN - LoadUserData Ok - account is already linked.";
			}

			String^ s_hubname = gcnew String(VueceGlobalContext::GetHubName());

			if(VueceGlobalContext::IsAccountLinked())
			{
				//load vuece application settings
				UpdateUserInfoOnUi(true);
			}
			else
			{
				UpdateUserInfoOnUi(false);
			}

			if(s_hubname != "")
			{
				textBoxHubName->Text = s_hubname;
			}
			else
			{
				TCHAR  infoBuf[128];
				char tmpBuf[128];
				DWORD  bufCharCount = 128;

				// Get and display the name of the computer.
				if( GetComputerName( infoBuf, &bufCharCount ) )
				{
					wcstombs(tmpBuf, infoBuf, 128 );
					textBoxHubName->Text = gcnew String(infoBuf);
				}
			}

			///populate advanced settings
			if(VueceGlobalContext::GetAllowFriendAccess() == 1)
			{
				this->checkBoxAllowFriendAccess->Checked = true;
			}
			else
			{
				this->checkBoxAllowFriendAccess->Checked = false;
			}

			numericMaxConcurrentStreaming->Value = VueceGlobalContext::GetMaxConcurrentStreaming();


			if(VueceGlobalContext::GetAutoLoginAtAppStartup() == 1)
			{
				this->checkBoxAutoLoginAtAppStart->Checked = true;
			}
			else
			{
				this->checkBoxAutoLoginAtAppStart->Checked = false;
			}

			if(VueceGlobalContext::GetAutoLoginAtSysStartup() == 1)
			{
				this->checkBoxAutoLoginAtSysStart->Checked = true;
			}
			else
			{
				this->checkBoxAutoLoginAtSysStart->Checked = false;
			}

			//add existing public folders
			std::list<std::string>::iterator it = iVueceGlobalSetting->iPublicFolderList->begin();

			while (it != iVueceGlobalSetting->iPublicFolderList->end())
			{
				std::string fp = *it;
				String^ s_pfolder = gcnew String(fp.c_str()	);

				listBoxShareFolders->Items->Add(s_pfolder);

				++it;
			}

			this->notifyIcon1->Visible = true;

			bExitingApp = false;

			if(ValidateInput())
			{
				LOG(INFO) << ("Input is valid");

				//auto-login
				if(VueceGlobalContext::GetAutoLoginAtAppStartup() == 1)
				{
					LOG(INFO) << ("Auto-login at app start up is enabled, start login now.");
					HandleSignInSignOut();
				}
				else if(iJustRebooted == 1 && VueceGlobalContext::GetAutoLoginAtSysStartup() == 1)
				{
					LOG(INFO) << ("Auto-login at sys start up is enabled and this is a sys reboot, start login now.");

					minimizeAppWindow();

					HandleSignInSignOut();
				}
			}

			checkAndUpdate(true);

		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VuecePCMainForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonSignIn;
	protected: 
	private: 

	private: System::Windows::Forms::NotifyIcon^  notifyIcon1;
	private: System::Windows::Forms::TabControl^  tabControl1;
	private: System::Windows::Forms::TabPage^  tabPageAccount;
	private: System::Windows::Forms::TabPage^  tabPageSetting;
private: System::Windows::Forms::ImageList^  imageList_Icons;


	private: System::Windows::Forms::GroupBox^  groupBoxSharedFolders;

	private: System::Windows::Forms::FolderBrowserDialog^  publicFolderBrowserDialog;



	private: System::IO::FileSystemWatcher^  publicFolderWatcher;

	private: System::ComponentModel::IContainer^  components;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  menuItemExit;
	private: System::Windows::Forms::ToolStripMenuItem^  menuItemUpdate;

	private: System::Windows::Forms::ToolStripMenuItem^  menuItemSettings;
	private: VueceFileSystem::VueceDirectoryMonitor^ dirMonitor;
	private: System::Windows::Forms::Button^  buttonRemoveShareFolder;
	private: System::Windows::Forms::Button^  buttonAddShareFolder;
	private: System::Windows::Forms::ListBox^  listBoxShareFolders;
	private: bool bExitingApp;
	private: bool bSignedIn;
	private: bool bSignedOutByUser;
	private: bool bFirstSignIn;
	private: String^ authCode;
	private: delegate void DelegateTest();
	private: delegate void DelegateOnRemoteDeviceActivity(array<VueceStreamingDevice* >^myArray);
	private: delegate void DelegateAddRosterImg(String^ key, String^ data_b64);
	private: delegate void DelegateOnRosterSubscriptionMsgReceived(array<VueceRosterSubscriptionMsg* >^myArray);

	private: ResourceManager^ resMgr;

	private: System::Windows::Forms::LinkLabel^  linkLabel1;
	private: System::Windows::Forms::LinkLabel^  linkLabel2;

	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel1;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel2;
	private: System::Windows::Forms::Button^  btnScanFiles;
	private: System::Windows::Forms::Button^  btnRemoveAllFolders;


	private: System::Windows::Forms::CheckBox^  checkBoxAutoLoginAtAppStart;
	private: System::Windows::Forms::ToolStripMenuItem^  menuItemAboutVueceHub;


private: System::Windows::Forms::TextBox^  textBoxHubName;
private: System::Windows::Forms::GroupBox^  groupBox_Hub;
private: System::Windows::Forms::GroupBox^  groupBox_Account;
private: System::Windows::Forms::Label^  label_DisplayName;
private: System::Windows::Forms::Button^  button_LinkAccount;

private: System::Windows::Forms::Label^  label_AccountName;
private: System::Windows::Forms::PictureBox^  pictureBox_UserImg;
private: System::Windows::Forms::CheckBox^  checkBoxAutoLoginAtSysStart;
private: System::Windows::Forms::TabPage^  tabPageAdvanced;
private: System::Windows::Forms::GroupBox^  groupBoxHubAccess;
private: System::Windows::Forms::CheckBox^  checkBoxAllowFriendAccess;

private: System::Windows::Forms::GroupBox^  groupBoxConcurrentStreaming;
private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::NumericUpDown^  numericMaxConcurrentStreaming;
private: System::Windows::Forms::GroupBox^  groupBox1;
private: System::Windows::Forms::ListView^  listViewStreamingDevices;
private: System::Windows::Forms::ColumnHeader^  columnHeader_UserName;


private: System::Windows::Forms::ColumnHeader^  columnHeader_FileName;
private: System::Windows::Forms::ColumnHeader^  columnHeader_Device;
private: System::Windows::Forms::ImageList^  imageListRosterImg;
private: System::Windows::Forms::ToolTip^  toolTip_PublicFolder;


	private: System::Windows::Forms::Label^  labelAppEventMsg;

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(VuecePCMainForm::typeid));
			this->buttonSignIn = (gcnew System::Windows::Forms::Button());
			this->imageList_Icons = (gcnew System::Windows::Forms::ImageList(this->components));
			this->notifyIcon1 = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->menuItemSettings = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menuItemUpdate = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menuItemAboutVueceHub = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menuItemExit = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPageAccount = (gcnew System::Windows::Forms::TabPage());
			this->checkBoxAutoLoginAtSysStart = (gcnew System::Windows::Forms::CheckBox());
			this->groupBox_Hub = (gcnew System::Windows::Forms::GroupBox());
			this->textBoxHubName = (gcnew System::Windows::Forms::TextBox());
			this->groupBox_Account = (gcnew System::Windows::Forms::GroupBox());
			this->pictureBox_UserImg = (gcnew System::Windows::Forms::PictureBox());
			this->label_AccountName = (gcnew System::Windows::Forms::Label());
			this->label_DisplayName = (gcnew System::Windows::Forms::Label());
			this->button_LinkAccount = (gcnew System::Windows::Forms::Button());
			this->checkBoxAutoLoginAtAppStart = (gcnew System::Windows::Forms::CheckBox());
			this->labelAppEventMsg = (gcnew System::Windows::Forms::Label());
			this->flowLayoutPanel1 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->linkLabel2 = (gcnew System::Windows::Forms::LinkLabel());
			this->linkLabel1 = (gcnew System::Windows::Forms::LinkLabel());
			this->tabPageSetting = (gcnew System::Windows::Forms::TabPage());
			this->groupBoxSharedFolders = (gcnew System::Windows::Forms::GroupBox());
			this->flowLayoutPanel2 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->buttonAddShareFolder = (gcnew System::Windows::Forms::Button());
			this->buttonRemoveShareFolder = (gcnew System::Windows::Forms::Button());
			this->btnRemoveAllFolders = (gcnew System::Windows::Forms::Button());
			this->btnScanFiles = (gcnew System::Windows::Forms::Button());
			this->listBoxShareFolders = (gcnew System::Windows::Forms::ListBox());
			this->tabPageAdvanced = (gcnew System::Windows::Forms::TabPage());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->listViewStreamingDevices = (gcnew System::Windows::Forms::ListView());
			this->columnHeader_UserName = (gcnew System::Windows::Forms::ColumnHeader());
			this->columnHeader_Device = (gcnew System::Windows::Forms::ColumnHeader());
			this->columnHeader_FileName = (gcnew System::Windows::Forms::ColumnHeader());
			this->imageListRosterImg = (gcnew System::Windows::Forms::ImageList(this->components));
			this->groupBoxHubAccess = (gcnew System::Windows::Forms::GroupBox());
			this->checkBoxAllowFriendAccess = (gcnew System::Windows::Forms::CheckBox());
			this->groupBoxConcurrentStreaming = (gcnew System::Windows::Forms::GroupBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->numericMaxConcurrentStreaming = (gcnew System::Windows::Forms::NumericUpDown());
			this->publicFolderBrowserDialog = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->toolTip_PublicFolder = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->contextMenuStrip1->SuspendLayout();
			this->tabControl1->SuspendLayout();
			this->tabPageAccount->SuspendLayout();
			this->groupBox_Hub->SuspendLayout();
			this->groupBox_Account->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox_UserImg))->BeginInit();
			this->flowLayoutPanel1->SuspendLayout();
			this->tabPageSetting->SuspendLayout();
			this->groupBoxSharedFolders->SuspendLayout();
			this->flowLayoutPanel2->SuspendLayout();
			this->tabPageAdvanced->SuspendLayout();
			this->groupBox1->SuspendLayout();
			this->groupBoxHubAccess->SuspendLayout();
			this->groupBoxConcurrentStreaming->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericMaxConcurrentStreaming))->BeginInit();
			this->SuspendLayout();
			// 
			// buttonSignIn
			// 
			resources->ApplyResources(this->buttonSignIn, L"buttonSignIn");
			this->buttonSignIn->ImageList = this->imageList_Icons;
			this->buttonSignIn->MaximumSize = System::Drawing::Size(150, 40);
			this->buttonSignIn->MinimumSize = System::Drawing::Size(150, 50);
			this->buttonSignIn->Name = L"buttonSignIn";
			this->buttonSignIn->UseVisualStyleBackColor = true;
			this->buttonSignIn->Click += gcnew System::EventHandler(this, &VuecePCMainForm::buttonSignIn_Click);
			// 
			// imageList_Icons
			// 
			this->imageList_Icons->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"imageList_Icons.ImageStream")));
			this->imageList_Icons->TransparentColor = System::Drawing::Color::Transparent;
			this->imageList_Icons->Images->SetKeyName(0, L"ic_action_android.png");
			this->imageList_Icons->Images->SetKeyName(1, L"ic_action_exit.png");
			this->imageList_Icons->Images->SetKeyName(2, L"ic_action_export.png");
			this->imageList_Icons->Images->SetKeyName(3, L"ic_action_gear.png");
			this->imageList_Icons->Images->SetKeyName(4, L"ic_action_globe.png");
			this->imageList_Icons->Images->SetKeyName(5, L"ic_action_heart.png");
			this->imageList_Icons->Images->SetKeyName(6, L"ic_action_help.png");
			this->imageList_Icons->Images->SetKeyName(7, L"ic_action_home.png");
			this->imageList_Icons->Images->SetKeyName(8, L"ic_action_import.png");
			this->imageList_Icons->Images->SetKeyName(9, L"ic_action_info.png");
			this->imageList_Icons->Images->SetKeyName(10, L"ic_action_list.png");
			this->imageList_Icons->Images->SetKeyName(11, L"ic_action_list_2.png");
			this->imageList_Icons->Images->SetKeyName(12, L"ic_action_reload.png");
			this->imageList_Icons->Images->SetKeyName(13, L"ic_action_share_2.png");
			this->imageList_Icons->Images->SetKeyName(14, L"ic_action_user.png");
			this->imageList_Icons->Images->SetKeyName(15, L"ic_action_warning.png");
			this->imageList_Icons->Images->SetKeyName(16, L"ic_action_lock_open.png");
			this->imageList_Icons->Images->SetKeyName(17, L"ic_action_wizard.png");
			this->imageList_Icons->Images->SetKeyName(18, L"ic_action_link.png");
			// 
			// notifyIcon1
			// 
			this->notifyIcon1->BalloonTipIcon = System::Windows::Forms::ToolTipIcon::Info;
			resources->ApplyResources(this->notifyIcon1, L"notifyIcon1");
			this->notifyIcon1->ContextMenuStrip = this->contextMenuStrip1;
			this->notifyIcon1->BalloonTipClicked += gcnew System::EventHandler(this, &VuecePCMainForm::notifyIcon1_BalloonTipClicked);
			this->notifyIcon1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &VuecePCMainForm::notifyIcon1_MouseClick);
			this->notifyIcon1->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &VuecePCMainForm::notifyIcon1_MouseDoubleClick);
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->BackColor = System::Drawing::SystemColors::Control;
			this->contextMenuStrip1->ImageScalingSize = System::Drawing::Size(30, 30);
			this->contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->menuItemSettings, 
				this->menuItemUpdate, this->menuItemAboutVueceHub, this->menuItemExit});
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
			resources->ApplyResources(this->contextMenuStrip1, L"contextMenuStrip1");
			// 
			// menuItemSettings
			// 
			resources->ApplyResources(this->menuItemSettings, L"menuItemSettings");
			this->menuItemSettings->Name = L"menuItemSettings";
			this->menuItemSettings->Click += gcnew System::EventHandler(this, &VuecePCMainForm::menuItemSettings_Click);
			// 
			// menuItemUpdate
			// 
			resources->ApplyResources(this->menuItemUpdate, L"menuItemUpdate");
			this->menuItemUpdate->Name = L"menuItemUpdate";
			this->menuItemUpdate->Click += gcnew System::EventHandler(this, &VuecePCMainForm::menuItemOpenCheckUpdate_Click);
			// 
			// menuItemAboutVueceHub
			// 
			resources->ApplyResources(this->menuItemAboutVueceHub, L"menuItemAboutVueceHub");
			this->menuItemAboutVueceHub->Name = L"menuItemAboutVueceHub";
			this->menuItemAboutVueceHub->Click += gcnew System::EventHandler(this, &VuecePCMainForm::menuItemAboutVueceHub_Click);
			// 
			// menuItemExit
			// 
			resources->ApplyResources(this->menuItemExit, L"menuItemExit");
			this->menuItemExit->Name = L"menuItemExit";
			this->menuItemExit->Click += gcnew System::EventHandler(this, &VuecePCMainForm::menuItemExit_Click);
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPageAccount);
			this->tabControl1->Controls->Add(this->tabPageSetting);
			this->tabControl1->Controls->Add(this->tabPageAdvanced);
			resources->ApplyResources(this->tabControl1, L"tabControl1");
			this->tabControl1->ImageList = this->imageList_Icons;
			this->tabControl1->MinimumSize = System::Drawing::Size(416, 363);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->SizeMode = System::Windows::Forms::TabSizeMode::FillToRight;
			// 
			// tabPageAccount
			// 
			this->tabPageAccount->BackColor = System::Drawing::Color::White;
			this->tabPageAccount->Controls->Add(this->checkBoxAutoLoginAtSysStart);
			this->tabPageAccount->Controls->Add(this->groupBox_Hub);
			this->tabPageAccount->Controls->Add(this->groupBox_Account);
			this->tabPageAccount->Controls->Add(this->checkBoxAutoLoginAtAppStart);
			this->tabPageAccount->Controls->Add(this->labelAppEventMsg);
			this->tabPageAccount->Controls->Add(this->flowLayoutPanel1);
			this->tabPageAccount->Controls->Add(this->linkLabel2);
			this->tabPageAccount->Controls->Add(this->linkLabel1);
			resources->ApplyResources(this->tabPageAccount, L"tabPageAccount");
			this->tabPageAccount->Name = L"tabPageAccount";
			// 
			// checkBoxAutoLoginAtSysStart
			// 
			resources->ApplyResources(this->checkBoxAutoLoginAtSysStart, L"checkBoxAutoLoginAtSysStart");
			this->checkBoxAutoLoginAtSysStart->Name = L"checkBoxAutoLoginAtSysStart";
			this->checkBoxAutoLoginAtSysStart->UseVisualStyleBackColor = true;
			this->checkBoxAutoLoginAtSysStart->CheckedChanged += gcnew System::EventHandler(this, &VuecePCMainForm::checkBoxAutoLoginAtSysStart_CheckedChanged);
			// 
			// groupBox_Hub
			// 
			this->groupBox_Hub->Controls->Add(this->textBoxHubName);
			resources->ApplyResources(this->groupBox_Hub, L"groupBox_Hub");
			this->groupBox_Hub->Name = L"groupBox_Hub";
			this->groupBox_Hub->TabStop = false;
			// 
			// textBoxHubName
			// 
			resources->ApplyResources(this->textBoxHubName, L"textBoxHubName");
			this->textBoxHubName->Name = L"textBoxHubName";
			this->textBoxHubName->TextChanged += gcnew System::EventHandler(this, &VuecePCMainForm::textBoxHubName_TextChanged);
			// 
			// groupBox_Account
			// 
			resources->ApplyResources(this->groupBox_Account, L"groupBox_Account");
			this->groupBox_Account->BackColor = System::Drawing::Color::Transparent;
			this->groupBox_Account->Controls->Add(this->pictureBox_UserImg);
			this->groupBox_Account->Controls->Add(this->label_AccountName);
			this->groupBox_Account->Controls->Add(this->label_DisplayName);
			this->groupBox_Account->Controls->Add(this->button_LinkAccount);
			this->groupBox_Account->Name = L"groupBox_Account";
			this->groupBox_Account->TabStop = false;
			// 
			// pictureBox_UserImg
			// 
			this->pictureBox_UserImg->BackColor = System::Drawing::Color::Transparent;
			resources->ApplyResources(this->pictureBox_UserImg, L"pictureBox_UserImg");
			this->pictureBox_UserImg->Name = L"pictureBox_UserImg";
			this->pictureBox_UserImg->TabStop = false;
			this->pictureBox_UserImg->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &VuecePCMainForm::pictureBoxUserImage_Paint);
			// 
			// label_AccountName
			// 
			resources->ApplyResources(this->label_AccountName, L"label_AccountName");
			this->label_AccountName->Name = L"label_AccountName";
			// 
			// label_DisplayName
			// 
			resources->ApplyResources(this->label_DisplayName, L"label_DisplayName");
			this->label_DisplayName->Name = L"label_DisplayName";
			// 
			// button_LinkAccount
			// 
			resources->ApplyResources(this->button_LinkAccount, L"button_LinkAccount");
			this->button_LinkAccount->AutoEllipsis = true;
			this->button_LinkAccount->ImageList = this->imageList_Icons;
			this->button_LinkAccount->Name = L"button_LinkAccount";
			this->button_LinkAccount->UseVisualStyleBackColor = true;
			this->button_LinkAccount->Click += gcnew System::EventHandler(this, &VuecePCMainForm::button_LinkAccount_Click);
			// 
			// checkBoxAutoLoginAtAppStart
			// 
			resources->ApplyResources(this->checkBoxAutoLoginAtAppStart, L"checkBoxAutoLoginAtAppStart");
			this->checkBoxAutoLoginAtAppStart->Name = L"checkBoxAutoLoginAtAppStart";
			this->checkBoxAutoLoginAtAppStart->UseVisualStyleBackColor = true;
			this->checkBoxAutoLoginAtAppStart->CheckedChanged += gcnew System::EventHandler(this, &VuecePCMainForm::checkBoxAutoLoginAtAppStart_CheckedChanged);
			// 
			// labelAppEventMsg
			// 
			this->labelAppEventMsg->BackColor = System::Drawing::Color::White;
			resources->ApplyResources(this->labelAppEventMsg, L"labelAppEventMsg");
			this->labelAppEventMsg->ForeColor = System::Drawing::SystemColors::HotTrack;
			this->labelAppEventMsg->Name = L"labelAppEventMsg";
			// 
			// flowLayoutPanel1
			// 
			this->flowLayoutPanel1->Controls->Add(this->buttonSignIn);
			resources->ApplyResources(this->flowLayoutPanel1, L"flowLayoutPanel1");
			this->flowLayoutPanel1->Name = L"flowLayoutPanel1";
			// 
			// linkLabel2
			// 
			resources->ApplyResources(this->linkLabel2, L"linkLabel2");
			this->linkLabel2->Name = L"linkLabel2";
			this->linkLabel2->TabStop = true;
			this->linkLabel2->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VuecePCMainForm::linkLabel2_LinkClicked);
			// 
			// linkLabel1
			// 
			resources->ApplyResources(this->linkLabel1, L"linkLabel1");
			this->linkLabel1->Name = L"linkLabel1";
			this->linkLabel1->TabStop = true;
			this->linkLabel1->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VuecePCMainForm::linkLabel1_LinkClicked);
			// 
			// tabPageSetting
			// 
			this->tabPageSetting->Controls->Add(this->groupBoxSharedFolders);
			resources->ApplyResources(this->tabPageSetting, L"tabPageSetting");
			this->tabPageSetting->Name = L"tabPageSetting";
			this->tabPageSetting->UseVisualStyleBackColor = true;
			// 
			// groupBoxSharedFolders
			// 
			this->groupBoxSharedFolders->Controls->Add(this->flowLayoutPanel2);
			this->groupBoxSharedFolders->Controls->Add(this->listBoxShareFolders);
			resources->ApplyResources(this->groupBoxSharedFolders, L"groupBoxSharedFolders");
			this->groupBoxSharedFolders->Name = L"groupBoxSharedFolders";
			this->groupBoxSharedFolders->TabStop = false;
			// 
			// flowLayoutPanel2
			// 
			this->flowLayoutPanel2->Controls->Add(this->buttonAddShareFolder);
			this->flowLayoutPanel2->Controls->Add(this->buttonRemoveShareFolder);
			this->flowLayoutPanel2->Controls->Add(this->btnRemoveAllFolders);
			this->flowLayoutPanel2->Controls->Add(this->btnScanFiles);
			resources->ApplyResources(this->flowLayoutPanel2, L"flowLayoutPanel2");
			this->flowLayoutPanel2->Name = L"flowLayoutPanel2";
			// 
			// buttonAddShareFolder
			// 
			resources->ApplyResources(this->buttonAddShareFolder, L"buttonAddShareFolder");
			this->buttonAddShareFolder->ImageList = this->imageList_Icons;
			this->buttonAddShareFolder->Name = L"buttonAddShareFolder";
			this->buttonAddShareFolder->UseVisualStyleBackColor = true;
			this->buttonAddShareFolder->Click += gcnew System::EventHandler(this, &VuecePCMainForm::buttonAddShareFolder_Click);
			// 
			// buttonRemoveShareFolder
			// 
			resources->ApplyResources(this->buttonRemoveShareFolder, L"buttonRemoveShareFolder");
			this->buttonRemoveShareFolder->ImageList = this->imageList_Icons;
			this->buttonRemoveShareFolder->Name = L"buttonRemoveShareFolder";
			this->buttonRemoveShareFolder->UseVisualStyleBackColor = true;
			this->buttonRemoveShareFolder->Click += gcnew System::EventHandler(this, &VuecePCMainForm::buttonRemoveShareFolder_Click);
			// 
			// btnRemoveAllFolders
			// 
			resources->ApplyResources(this->btnRemoveAllFolders, L"btnRemoveAllFolders");
			this->btnRemoveAllFolders->Name = L"btnRemoveAllFolders";
			this->btnRemoveAllFolders->UseVisualStyleBackColor = true;
			this->btnRemoveAllFolders->Click += gcnew System::EventHandler(this, &VuecePCMainForm::btnRemoveAllFolders_Click);
			// 
			// btnScanFiles
			// 
			resources->ApplyResources(this->btnScanFiles, L"btnScanFiles");
			this->btnScanFiles->Name = L"btnScanFiles";
			this->btnScanFiles->UseVisualStyleBackColor = true;
			this->btnScanFiles->Click += gcnew System::EventHandler(this, &VuecePCMainForm::btnScanFiles_Click);
			// 
			// listBoxShareFolders
			// 
			this->listBoxShareFolders->AllowDrop = true;
			this->listBoxShareFolders->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			resources->ApplyResources(this->listBoxShareFolders, L"listBoxShareFolders");
			this->listBoxShareFolders->FormattingEnabled = true;
			this->listBoxShareFolders->Name = L"listBoxShareFolders";
			this->listBoxShareFolders->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
			this->toolTip_PublicFolder->SetToolTip(this->listBoxShareFolders, resources->GetString(L"listBoxShareFolders.ToolTip"));
			this->listBoxShareFolders->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &VuecePCMainForm::ListBoxShareFolders_DragDrop);
			this->listBoxShareFolders->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &VuecePCMainForm::ListBoxShareFolders_DragEnter);
			// 
			// tabPageAdvanced
			// 
			this->tabPageAdvanced->Controls->Add(this->groupBox1);
			this->tabPageAdvanced->Controls->Add(this->groupBoxHubAccess);
			this->tabPageAdvanced->Controls->Add(this->groupBoxConcurrentStreaming);
			resources->ApplyResources(this->tabPageAdvanced, L"tabPageAdvanced");
			this->tabPageAdvanced->Name = L"tabPageAdvanced";
			this->tabPageAdvanced->UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->listViewStreamingDevices);
			resources->ApplyResources(this->groupBox1, L"groupBox1");
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->TabStop = false;
			// 
			// listViewStreamingDevices
			// 
			this->listViewStreamingDevices->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->columnHeader_UserName, 
				this->columnHeader_Device, this->columnHeader_FileName});
			resources->ApplyResources(this->listViewStreamingDevices, L"listViewStreamingDevices");
			this->listViewStreamingDevices->LargeImageList = this->imageListRosterImg;
			this->listViewStreamingDevices->MultiSelect = false;
			this->listViewStreamingDevices->Name = L"listViewStreamingDevices";
			this->listViewStreamingDevices->SmallImageList = this->imageListRosterImg;
			this->listViewStreamingDevices->UseCompatibleStateImageBehavior = false;
			this->listViewStreamingDevices->View = System::Windows::Forms::View::Details;
			// 
			// columnHeader_UserName
			// 
			resources->ApplyResources(this->columnHeader_UserName, L"columnHeader_UserName");
			// 
			// columnHeader_Device
			// 
			resources->ApplyResources(this->columnHeader_Device, L"columnHeader_Device");
			// 
			// columnHeader_FileName
			// 
			resources->ApplyResources(this->columnHeader_FileName, L"columnHeader_FileName");
			// 
			// imageListRosterImg
			// 
			this->imageListRosterImg->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"imageListRosterImg.ImageStream")));
			this->imageListRosterImg->TransparentColor = System::Drawing::Color::Transparent;
			this->imageListRosterImg->Images->SetKeyName(0, L"ic_action_android.png");
			// 
			// groupBoxHubAccess
			// 
			this->groupBoxHubAccess->Controls->Add(this->checkBoxAllowFriendAccess);
			resources->ApplyResources(this->groupBoxHubAccess, L"groupBoxHubAccess");
			this->groupBoxHubAccess->Name = L"groupBoxHubAccess";
			this->groupBoxHubAccess->TabStop = false;
			// 
			// checkBoxAllowFriendAccess
			// 
			resources->ApplyResources(this->checkBoxAllowFriendAccess, L"checkBoxAllowFriendAccess");
			this->checkBoxAllowFriendAccess->Name = L"checkBoxAllowFriendAccess";
			this->checkBoxAllowFriendAccess->UseVisualStyleBackColor = true;
			this->checkBoxAllowFriendAccess->CheckedChanged += gcnew System::EventHandler(this, &VuecePCMainForm::checkBoxAllowFriendAccess_CheckedChanged);
			// 
			// groupBoxConcurrentStreaming
			// 
			this->groupBoxConcurrentStreaming->Controls->Add(this->label1);
			this->groupBoxConcurrentStreaming->Controls->Add(this->numericMaxConcurrentStreaming);
			resources->ApplyResources(this->groupBoxConcurrentStreaming, L"groupBoxConcurrentStreaming");
			this->groupBoxConcurrentStreaming->Name = L"groupBoxConcurrentStreaming";
			this->groupBoxConcurrentStreaming->TabStop = false;
			// 
			// label1
			// 
			resources->ApplyResources(this->label1, L"label1");
			this->label1->Name = L"label1";
			// 
			// numericMaxConcurrentStreaming
			// 
			resources->ApplyResources(this->numericMaxConcurrentStreaming, L"numericMaxConcurrentStreaming");
			this->numericMaxConcurrentStreaming->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
			this->numericMaxConcurrentStreaming->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->numericMaxConcurrentStreaming->Name = L"numericMaxConcurrentStreaming";
			this->numericMaxConcurrentStreaming->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
			this->numericMaxConcurrentStreaming->ValueChanged += gcnew System::EventHandler(this, &VuecePCMainForm::numericMaxConcurrentStreaming_ValueChanged);
			// 
			// publicFolderBrowserDialog
			// 
			this->publicFolderBrowserDialog->RootFolder = System::Environment::SpecialFolder::MyComputer;
			// 
			// VuecePCMainForm
			// 
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
			this->Controls->Add(this->tabControl1);
			this->Name = L"VuecePCMainForm";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &VuecePCMainForm::Form1_FormClosing);
			this->FormClosed += gcnew System::Windows::Forms::FormClosedEventHandler(this, &VuecePCMainForm::Form1_FormClosed);
			this->Resize += gcnew System::EventHandler(this, &VuecePCMainForm::Form1_Resize);
			this->contextMenuStrip1->ResumeLayout(false);
			this->tabControl1->ResumeLayout(false);
			this->tabPageAccount->ResumeLayout(false);
			this->tabPageAccount->PerformLayout();
			this->groupBox_Hub->ResumeLayout(false);
			this->groupBox_Hub->PerformLayout();
			this->groupBox_Account->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox_UserImg))->EndInit();
			this->flowLayoutPanel1->ResumeLayout(false);
			this->tabPageSetting->ResumeLayout(false);
			this->groupBoxSharedFolders->ResumeLayout(false);
			this->flowLayoutPanel2->ResumeLayout(false);
			this->tabPageAdvanced->ResumeLayout(false);
			this->groupBox1->ResumeLayout(false);
			this->groupBoxHubAccess->ResumeLayout(false);
			this->groupBoxHubAccess->PerformLayout();
			this->groupBoxConcurrentStreaming->ResumeLayout(false);
			this->groupBoxConcurrentStreaming->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericMaxConcurrentStreaming))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion




	private: System::Void createGlobalSettingInstance(){
			LOG(INFO) << "createGlobalSettingInstance";

			VueceGlobalContext::Init(VueceAppRole_Media_Hub);
	}

	private: System::Void handleJingleEvent_LoginFailure() {

		if (this->buttonSignIn->InvokeRequired)
		{
			this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_LoginFailure));
			return;
		}
		else
		{

			ShowErrorDialog(resMgr->GetString("String3_LoginFailedTitle"), resMgr->GetString("String4_LoginFailedMessage"));

			handleJingleEvent_SignedOut();

		}
	}

	private: System::Void handleJingleEvent_SigningIn_UpdateUI() {

		if (this->buttonSignIn->InvokeRequired)
		{
			this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_SigningIn_UpdateUI));
			return;
		}
		else
		{
			// This is the UI thread so perform the task.
			this->UseWaitCursor = true;
			//change button state
			this->buttonSignIn->Text = resMgr->GetString("String1_SigningIn");
			this->buttonSignIn->Enabled = false;
			textBoxHubName->Enabled = false;
			listBoxShareFolders->Enabled = false;
			buttonRemoveShareFolder->Enabled = false;
			buttonAddShareFolder->Enabled = false;
			button_LinkAccount->Enabled = false;

			btnScanFiles->Enabled = false;
			btnRemoveAllFolders->Enabled = false;

			this->labelAppEventMsg->Text = L"";
		}
	}

	private: System::Void handleJingleEvent_ConnectionFailedWithAutoReconnect() {
			if (this->buttonSignIn->InvokeRequired)
			{
				this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_ConnectionFailedWithAutoReconnect));
				return;
			}
			else
			{
				LOG(INFO) << "handleJingleEvent_ConnectionFailedWithAutoReconnect - Sign out now";

				bSignedOutByUser = false;

				HandleSignInSignOut();

				LOG(INFO) << "handleJingleEvent_ConnectionFailedWithAutoReconnect - Update event label text";

				//change the label text
				this->labelAppEventMsg->Text = resMgr->GetString("String16_ConnectionFailedWithAutoReconnect");

				//update UI
				handleJingleEvent_SignedOut();
			}

		}

	private: System::Void handleJingleEvent_ConnectionFailed() {
		if (this->buttonSignIn->InvokeRequired)
		{
			this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_ConnectionFailed));
			return;
		}
		else
		{

			ShowErrorDialog(L"Cannot sign in", L"An error occurred during signing in, please try again.");

			LOG(INFO) << "handleJingleEvent_ConnectionFailed - Sign out now";

			//this is not caused my user selection
			bSignedOutByUser = false;

			HandleSignInSignOut();

			LOG(INFO) << "handleJingleEvent_ConnectionFailed - Update event label text";

			//change the label text
			this->labelAppEventMsg->Text = resMgr->GetString("String16_ConnectionFailedWithAutoReconnect");

			//update UI
			handleJingleEvent_SignedOut();

			//stop checking because we are signing out now
			connectionMonitorThread->WaitForSignOut();
		}

	}

	private: System::Void handleJingleEvent_SignedOut() {

		if (this->buttonSignIn->InvokeRequired)
		{
			this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_SignedOut));
			return;
		}
		else
		{
			LOG(INFO) << "handleJingleEvent_SignedOut";

			// This is the UI thread so perform the task.
			this->UseWaitCursor = false;
			//change button state
			this->buttonSignIn->Text = resMgr->GetString("String0_SignIn");
			this->buttonSignIn->Enabled = true;
			button_LinkAccount->Enabled = true;

			this->textBoxHubName->Enabled = true;
			this->buttonSignIn->ImageKey = L"ic_action_import.png";
			
			listBoxShareFolders->Enabled = true;
			buttonRemoveShareFolder->Enabled = true;
			buttonAddShareFolder->Enabled = true;

			btnScanFiles->Enabled = true;
			btnRemoveAllFolders->Enabled = true;

			notifyIcon1->Icon = (System::Drawing::Icon^)resMgr->GetObject(L"applogo_gray");

			bSignedIn = false;
		}
	}

	//ddd
	private: System::Void handleJingleEvent_SignedIn() {

			if (this->buttonSignIn->InvokeRequired)
			{
				this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::handleJingleEvent_SignedIn));
				return;
			}
			else
			{
				LOG(INFO) << "handleJingleEvent_SignedIn";

				// This is the UI thread so perform the task.
				this->UseWaitCursor = false;
				//change button state
				this->buttonSignIn->Text = resMgr->GetString("String2_SignOut");
				this->buttonSignIn->Enabled = true;
				button_LinkAccount->Enabled = true;
				this->buttonSignIn->ImageKey = L"ic_action_export.png";

				this->labelAppEventMsg->Text = L"";

				//disable it for now
				startConnectionMonitor(VueceNetworkMonitorState_OnlineSignedIn);

				//minimize the window
				minimizeAppWindow();

				PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

				//only popup the balloon tip when the main UI is not visible
				if(this->notifyIcon1->Visible)
				{
					notifyIcon1->Icon = (System::Drawing::Icon^)resMgr->GetObject(L"applogo_blue");

					this->notifyIcon1->ShowBalloonTip(
							10*1000,
							resMgr->GetString("String19_VueceHubOnlineNow"),
							resMgr->GetString("String20_ClickToRestoreWindow"),
							System::Windows::Forms::ToolTipIcon::Info
							);
								
				}
			}
		}

	private: System::Void startConnectionMonitor(VueceNetworkMonitorState initialState) {
		LOG(INFO) << "startConnectionMonitor - Start connection monitor thread...";

		stopConnectionMonitor();

		connectionMonitorThread = new VueceConnectionMonitorThread();
		connectionMonitorThread->ui = this;
		connectionMonitorThread->Start(initialState);
	}

	private: System::Void stopConnectionMonitor() {
		LOG(INFO) << "stopConnectionMonitor";

		if(connectionMonitorThread != NULL)
		{
			LOG(INFO) << "connectionMonitorThread is not null, delete it now";

			connectionMonitorThread->Stop();

			delete connectionMonitorThread;
			connectionMonitorThread = NULL;
		}
	}

	//TODO - Continue your work here...
	private: System::Void PersistUserData() {
			VueceCrypt crypt;
			std:string hubName("");
			int i;
			char pTmpBuf[1024];
			char pTmpBuf2[1024];

			memset(pTmpBuf, 0, sizeof(pTmpBuf));
			memset(pTmpBuf2, 0, sizeof(pTmpBuf2));

			VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

			LOG(INFO) << "PersistUserData - Start";

			//check hub id first, create a new one if necessary
			if(strlen(iVueceGlobalSetting->hubID) == 0)
			{
				LOG(INFO) << "PersistUserData - hub id is not created yet, generate a new one";

				if( VueceWinUtilities::GenUUID(iVueceGlobalSetting->hubID ) )
				{
					LOG(INFO) << "PersistUserData - Hub id created: " << iVueceGlobalSetting->hubID;
				}
				else
				{
					LOG(LS_ERROR) << "PersistUserData - Hub id cannot be created.";
				}
			}
			else
			{
				LOG(INFO) << "PersistUserData - hub id is already created, no need to create a new one: " << iVueceGlobalSetting->hubID;
			}

			LOG(INFO) << "PersistUserData - 1";

			String^ hubNameInput = textBoxHubName->Text->Trim();
			if(hubNameInput->Length == 0)
			{
				TCHAR  infoBuf[128];
				char tmpBuf[128];
				DWORD  bufCharCount = 128;

				// Get and display the name of the computer.
				if( GetComputerName( infoBuf, &bufCharCount ) )
				{
					wcstombs(tmpBuf, infoBuf, 128 );
					textBoxHubName->Text = gcnew String(infoBuf);
				}
				else
				{
					//cannnot retrieve PC name, use default value
					textBoxHubName->Text = L"Vuece Music Hub";
				}
			}
			else
			{
				textBoxHubName->Text = hubNameInput;
			}

			VueceWinUtilities::MarshalString(textBoxHubName->Text, hubName);

			strcpy(iVueceGlobalSetting->hubName, hubName.c_str());

			LOG(INFO) << "PersistUserData - 4";

			VueceWinUtilities::GenerateUserSettingString(pTmpBuf, iVueceGlobalSetting);

			LOG(INFO) << "PersistUserData - generateUserSettingString returned:\n" << pTmpBuf;

			i = crypt.EncryptStrToBinary( pTmpBuf, pTmpBuf2,  sizeof(pTmpBuf2) );

			LOG(INFO) << "PersistUserData - EncryptStrToBinary returned, result data len = " << i;

			VueceWinUtilities::SaveUserDataToRegistry(pTmpBuf2, i);

			LOG(INFO) << "PersistUserData - Done";
	}

	private: System::Void handleSignIn()
	{
		VueceCrypt crypt;

		LOG(LS_VERBOSE) << "handleSignIn - Start";

		//validate input at first
		char pTmpBuf[1024];

		std::string tmp_string;

		VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

		memset(pTmpBuf, 0, sizeof(pTmpBuf));

		LOG(INFO) << "handleSignIn - not signed in yet";

		if(iVueceGlobalSetting->iPublicFolderList->size() == 0){
				MessageBox::Show(this,
								resMgr->GetString("String13_SignInButNoSharedFolderDlgTitle"),
								resMgr->GetString("String14_SignInButNoSharedFolderDlgLable"),
								MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
				return;
		}

			LOG(INFO) << "buttonSignIn_Click";

			////////////////////////////////////////////////////////////////
			//Uncomment follwoing code to enable oath2
			////Test code - OAUTH2 implementation
			//OATH2 - START

			memset(iVueceGlobalSetting->accountAccessToken, 0, sizeof(iVueceGlobalSetting->accountAccessToken));

			if( iVueceGlobalSetting->accountLinked )
			{
					LOG(LS_VERBOSE) << "buttonSignIn_Click: Refresh token has been previously acquired:  " << iVueceGlobalSetting->accountRefreshToken;

					bool bret = VueceWinUtilities::QueryAccessToken (true, (const char*)iVueceGlobalSetting->accountRefreshToken,  iVueceGlobalSetting->accountAccessToken, NULL);

			}
			else
			{
					LOG(LS_VERBOSE) << "buttonSignIn_Click: Refresh token is empty, query it now.";
					StartAccountLinkHandling();
			}

			if(strlen(iVueceGlobalSetting->accountAccessToken) == 0)
			{
				//clear setting so next time the dialog will pop up to re-request the auth code
				memset(iVueceGlobalSetting->accountRefreshToken, 0, sizeof(iVueceGlobalSetting->accountRefreshToken));

				ShowErrorDialog(L"Sign in failed", L"An error occurred during sign in process, please try again.");

				ClearUserInfo();

				return;

			}
			else
			{
				LOG(LS_VERBOSE) << "buttonSignIn_Click: Access token acquired: " << iVueceGlobalSetting->accountAccessToken;

				LOG(LS_VERBOSE) << "buttonSignIn_Click: Query and update user account info";

				//update user info
				bool ret = VueceWinUtilities::QueryUserAccountInfo (
					(const char*)iVueceGlobalSetting->accountAccessToken,
					iVueceGlobalSetting->accountUserName,
					iVueceGlobalSetting->accountDisplayName,
					iVueceGlobalSetting->accountImgUrl
					);

				if(!ret)
				{
					LOG(LS_ERROR) << "buttonSignIn_Click: QueryUserAccountInfo failed.";
				}
				else
				{
					UpdateUserInfoOnUi(true);
				}
			}

			//OATH2 - END
			////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//do a scan at first if needed
		if(iVueceGlobalSetting->iRescanNeeded == 1){

			LOG(INFO) << "handleSignIn - A full scan is needed before signing in..";

			if( !StartFileScanThread(true)){

				LOG(INFO) << "handleSignIn - Cannot sign in because scan is not completed yet.";
				return;
			}

			iVueceGlobalSetting->iRescanNeeded = 0;
					
		}

		PersistUserData();

		//Disable/enable related UI components
		handleJingleEvent_SigningIn_UpdateUI();

		if(bFirstSignIn)
		{
			LOG(INFO) << "handleSignIn - This is the first sign in.";
			bFirstSignIn = false;
		}
		else
		{
			//TODO - Need to sync this operation here because at the point
			//we might be still destroying the previous tester instance.
			LOG(INFO) << "handleSignIn - This is NOT the first sign in.";

			jingleClientThread = new VueceJingleClientThread();
			jingleClientThread->ui = this;

			ReLoadUserData();
		}

		//use oauth2
		jingleClientThread->SetAccount(iVueceGlobalSetting->accountUserName, iVueceGlobalSetting->accountAccessToken, 2);

		LOG(INFO) << "handleSignIn - Starting jingle client thread ... ";

		jingleClientThread->Start();

		LOG(INFO) << "handleSignIn - jingleClientThread thread started... ";

		if (jingleClientThread->IsRunning())
		{
			LOG(INFO) << "handleSignIn - jingleClientThread is running now, start public folder monitor.";
//				dirMonitor->StartMonitor();
		}
		else
		{
			LOG(INFO) << "handleSignIn - jingleClientThread is completed ";
		}

		LOG(LS_VERBOSE) << "handleSignIn - End";
	}

	private: System::Void handleSignOut()
	{
		LOG(LS_VERBOSE) << "handleSignOut - Start";

	    //If user explicitly chose to sign in/out, then we need to stop the connection monitor
		if(bSignedOutByUser)
		{
			LOG(LS_VERBOSE) << "handleSignOut - Signed out by user, stop connection monitor thread.";
			stopConnectionMonitor();
		}
		else
		{
			LOG(LS_VERBOSE) << "handleSignOut - Not signed out by user, connection monitor thread will continue to trigger auto re-login";
		}

		//persist user data in case user changed any setting
		PersistUserData();

		//sign out
		//TODO - Need to sync this call, we need to make sure all
		//jingle layer resources (i.e, all kinds of threads, sessions) are finalized at first.
		//This call will eventually result in a call on VueceCoreClient::Quit()

		LOG(INFO) << "UI - Sign out client thread and wait until it's destroyed";

		jingleClientThread->client->LogOut();

		LOG(LS_VERBOSE) << "handleSignOut - client->SignOut called";

		bSignedIn = false;

		LOG(LS_VERBOSE) << "handleSignOut - End";
	}

	private: System::Void HandleSignInSignOut()
	{
		if (this->buttonSignIn->InvokeRequired)
		{
			this->Invoke(gcnew DelegateTest(this, &vuecepc::VuecePCMainForm::HandleSignInSignOut));
			return;
		}

		LOG(INFO) << "HandleSignInSignOut.";

		if(!bSignedIn)
		{
			LOG(INFO) << "HandleSignInSignOut - sign in now";
			handleSignIn();
		}
		else
		{
			LOG(INFO) << "HandleSignInSignOut - sign out now";
			handleSignOut();
		}
	}

	private: System::Void buttonSignIn_Click(System::Object^  sender, System::EventArgs^  e) {

			//check network connection status if this is a sign in attempt
			if(!bSignedIn)
			{
				this->labelAppEventMsg->Text = L"";

				if(!VueceConnectionMonitorThread::CheckConnection())
				{
					//no connection
					LOG(INFO) << "User chose to sign in but there is no connection";

					MessageBox::Show(this,
									resMgr->GetString("String18_UserPressedSignInBtnWithoutConnection"),
									resMgr->GetString("String17_NoAvailableNetworkConnection"),
									MessageBoxButtons::OK,
							         MessageBoxIcon::Information);

					return;
				}
			}
			else
			{
				bSignedOutByUser = true;
			}

			HandleSignInSignOut();
	 	 }

private: System::Void notifyIcon1_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			
			 std::string folder("start explorer ");
//			 folder.append(iVueceGlobalSetting->publicFolderLocation);
//			 system(folder.c_str());
		 }
private: System::Void publicFolderWatcher_Changed(System::Object^  sender, System::IO::FileSystemEventArgs^  e) {
			 // Occurs when the contents of the file change.
			//MessageBox.Show(string.Format("Changed: {0} {1}", e.FullPath, e.ChangeType));
			std::string stdString;
			VueceWinUtilities::MarshalString(e->FullPath, stdString);
			LOG(INFO) << "publicFolderWatcher_Changed: Path: " << stdString;
		 }
private: System::Void publicFolderWatcher_Created(System::Object^  sender, System::IO::FileSystemEventArgs^  e) {
			std::string stdString;
			VueceWinUtilities::MarshalString(e->FullPath, stdString);
			LOG(INFO) << "publicFolderWatcher_Created: Path: " << stdString;
		 }
private: System::Void publicFolderWatcher_Deleted(System::Object^  sender, System::IO::FileSystemEventArgs^  e) {
			std::string stdString;
			VueceWinUtilities::MarshalString(e->FullPath, stdString);
			LOG(INFO) << "publicFolderWatcher_Deleted: Path: " << stdString;
		 }
private: System::Void publicFolderWatcher_Renamed(System::Object^  sender, System::IO::RenamedEventArgs^  e) {
			std::string stdString;
			VueceWinUtilities::MarshalString(e->FullPath, stdString);
			LOG(INFO) << "publicFolderWatcher_Renamed: Path: "  << stdString;
		 }

private: System::Void AddRosterImg(String^ key, String^ data_b64)
{
	LOG(INFO) << "UI:AddRosterImg";

	if(imageListRosterImg->Images->ContainsKey(key))
	{
		LOG(WARNING) <<"UI:AddRosterImg - Key already exist, image data will be updated";
		imageListRosterImg->Images->RemoveByKey(key);
	}

	array<Byte>^binaryData = Convert::FromBase64String( data_b64 );
	MemoryStream^ ms = gcnew MemoryStream(binaryData);

	imageListRosterImg->Images->Add(key, Image::FromStream(ms));

}


public: virtual void OnRosterVCardReceived(const std::string& jid, const std::string& fn, const std::string& img_b64)
{
	std::string decoded;

	String^ img_key = gcnew String(jid.c_str());
	String^ img_data_b64 = gcnew String(img_b64.c_str());

	LOG(INFO) << "UI:OnRosterVCardReceived - jid="<<jid<<", user name: " << fn;

//	array<Byte>^binaryData = Convert::FromBase64String( img_data_b64 );

//	LOG(INFO) << "UI:OnRosterVCardReceived - img data decoded";

//	MemoryStream^ ms = gcnew MemoryStream(binaryData);

//	LOG(INFO) << "UI:OnRosterVCardReceived - img loaded as stream";

	//added image key and data to roster image list BBB

	array<String^>^myStringArray = {img_key, img_data_b64};

	this->Invoke(gcnew DelegateAddRosterImg(this, &vuecepc::VuecePCMainForm::AddRosterImg), myStringArray);
}

public: void HandleRosterSubscriptionMsgReceived(array<VueceRosterSubscriptionMsg* >^myArray)
{
	LOG(INFO) << "UI:HandleRosterSubscriptionMsgReceived";

	VueceRosterSubscriptionMsg* m = myArray[0];

	switch(m->subscribe_type)
	{
	case VueceRosterSubscriptionType_Unavailable:
	{
		String^ imgKey = gcnew String(m->user_id);

		LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - UNAVAILABLE";

		//remove roster image and roster entry in the streaming table
		RemoveStreamingDeviceByKey(imgKey);

		if(imageListRosterImg->Images->ContainsKey(imgKey))
		{
			imageListRosterImg->Images->RemoveByKey(imgKey);

			LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - Roster image remvoved from roster image list";

		}
		else
		{
			LOG(WARNING) << "UI:OnRosterSubscriptionMsgReceived - Roster image list doesn't have this key, sth is wrong";
		}
		break;
	}
	case VueceRosterSubscriptionType_Subscribe:
		{
			LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - SUBSCRIBE";

			break;
		}
	case VueceRosterSubscriptionType_Unsubscribe:
		{
			LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - UNSUBSCRIBE";
			break;
		}
	case VueceRosterSubscriptionType_Subscribed:
		{
			LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - SUBSCRIBED";
			break;
		}
	case VueceRosterSubscriptionType_Unsubscribed:
		{
			LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - UNSUBSCRIBED";
			break;
		}
	default:
	{
		LOG(LS_ERROR) << "UI:OnRosterSubscriptionMsgReceived - UNKNOWN";
		break;
	}
	}
}

public: virtual void OnRosterSubscriptionMsgReceived(VueceRosterSubscriptionMsg* m)
{
	LOG(INFO) << "UI:OnRosterSubscriptionMsgReceived - jid="<<m->user_id<<", type value: " << m->subscribe_type;

	array<VueceRosterSubscriptionMsg*>^myArray = {m};

	this->Invoke(gcnew DelegateOnRosterSubscriptionMsgReceived(this, &vuecepc::VuecePCMainForm::HandleRosterSubscriptionMsgReceived), myArray);

}

public: virtual void OnRemoteDeviceActivity(VueceStreamingDevice* d)
{
	LOG(INFO) << "UI:OnRemoteDeviceActivity";

	if(d == NULL)
	{
		return;
	}

	array<VueceStreamingDevice* >^ myArray = {d};

	this->Invoke(gcnew DelegateOnRemoteDeviceActivity(this, &vuecepc::VuecePCMainForm::handleOnRemoteDeviceActivity), myArray);
}

private: System::Void handleOnRemoteDeviceActivity(array<VueceStreamingDevice* >^ myArray)
{
	LOG(INFO) << "UI:handleOnRemoteDeviceActivity";

	VueceStreamingDevice* d = myArray[0];
	VueceRemoteDeviceActivityType activity_type = d->activity;

	switch(activity_type)
	{
	case VueceRemoteDeviceActivityType_StreamingStarted:
	{
		AddNewStreamingDevice(d);
		break;
	}
	case VueceRemoteDeviceActivityType_StreamingTerminated:
	{
		String^ user_jid = gcnew String(d->user_id);

		LOG(INFO) << "UI:handleOnRemoteDeviceActivity - Calling RemoveStreamingDeviceByKey() with key: " << d->user_id;

		RemoveStreamingDeviceByKey(user_jid);
		break;
	}
	default:
	{
		//ignore
		break;
	}
	}
}


private: System::Void AddNewStreamingDevice(VueceStreamingDevice* d)
{
	VueceRemoteDeviceActivityType activity_type = d->activity;
	String^ user_jid = gcnew String(d->user_id);
	String^ user_name = gcnew String(d->user_name);
	String^ device = gcnew String(d->device_name);
	String^ file_url = gcnew String(d->file_url);

	LOG(INFO) << "UI:AddNewStreamingDevice - Jid: " << d->user_id << ", name: " << d->user_name;

//	 ListViewItem^ item1 = gcnew ListViewItem( user_jid, 0);
//	 item1->SubItems->Add( file_url );
//	 listViewStreamingDevices->Items->Add( item1 );

	if( listViewStreamingDevices-> Items->ContainsKey(user_jid) )
	{
		//should not happen
		LOG(WARNING) << "UI:AddNewStreamingDevice - Record already exists, Jid: " << d->user_id;
	}
	else
	{
		if(this->imageListRosterImg->Images-> ContainsKey(user_jid))
		{
			ListViewItem^ added = listViewStreamingDevices->Items->Add( user_jid, user_name, user_jid);
			added->SubItems->Add( device );
			added->SubItems->Add( file_url );
		}
		else
		{
			//use default image
			ListViewItem^ added = listViewStreamingDevices->Items->Add( user_jid, user_name, 0);
			added->SubItems->Add( device );
			added->SubItems->Add( file_url );
		}

	}
}

private: System::Void RemoveStreamingDeviceByKey(String^ key)
{
	LOG(INFO) << "UI:RemoveStreamingDevice ";

	if( listViewStreamingDevices-> Items->ContainsKey(key) )
	{
		LOG(INFO) << "UI:RemoveStreamingDevice - item located in list, remove it now";
		listViewStreamingDevices->Items->RemoveByKey(key);
	}
	else
	{
		LOG(INFO) << "UI:RemoveStreamingDevice - Record does not exist";
	}
}

public: virtual void OnUserEvent(int code)
{
	LOG(INFO) << "UI:OnUserEvent - code: " << code;

	switch(code)
	{
	case VueceEvent_Client_AuthFailed:
	{
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_AuthFailed";

		handleJingleEvent_LoginFailure();

		// If the no button was pressed ...
//		      if (
//		    		  (MessageBox::Show(
//		         "Are you sure that you would like to close the form?",
//		         "Form Closing", MessageBoxButtons::OK,
//		         MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::No))
//		      {
//				 // cancel the closure of the form.
////		         e->Cancel = true;
//		      }

			break;
	}

	case VueceEvent_Client_SignedIn:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_SignedIn";
		OnClientSignedIn();
		break;
	case VueceEvent_Client_SignedOut:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_SignedOut";
		handleJingleEvent_SignedOut();
		break;
	case VueceEvent_Connection_FailedWithAutoReconnect:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Connection_FailedWithAutoReconnect";
		handleJingleEvent_ConnectionFailedWithAutoReconnect();
		break;
	case VueceEvent_Connection_Failed:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Connection_Failed";
		handleJingleEvent_ConnectionFailed();
		break;

	case VueceEvent_Client_BackOnLine:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_BackOnLine";
		HandleSignInSignOut();
		break;

	case VueceEvent_Client_Destroyed:
		LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_Destroyed, start checking network again";

		if(connectionMonitorThread != NULL)
		{
			connectionMonitorThread->ForceOffline();
		}
		else
		{
			LOG(INFO) << "UI::OnUserEvent - VueceEvent_Client_Destroyed, connectionMonitorThread is null. do nothing";
		}
		break;
	}
}

public: virtual void OnClientSignedIn()
{
	LOG(INFO) << "UI:OnClientSignedIn, update UI status";
	bSignedIn = true;
	handleJingleEvent_SignedIn();
}

private: System::Void Form1_Resize(System::Object^  sender, System::EventArgs^  e) {
			LOG(INFO) << "UI:Form1_Resize";
			 if(this->WindowState == FormWindowState::Minimized)
			 {
				 minimizeAppWindow();
			 }
		 }

private: System::Void menuItemExit_Click(System::Object^  sender, System::EventArgs^  e) {
			LOG(INFO) << "UI:menuItemExit_Click";
			 bExitingApp = true;
			 this->notifyIcon1->Visible = false;

			 //persist user setting again
			 PersistUserData();

			 this->Close();
		 }

private: System::Void menuItemOpenCheckUpdate_Click(System::Object^  sender, System::EventArgs^  e) {
			LOG(INFO) << "UI:menuItemOpenCheckUpdate_Click";
			checkAndUpdate(false);
		 }

private: System::Void menuItemSettings_Click(System::Object^  sender, System::EventArgs^  e) {
			LOG(INFO) << "UI:menuItemSettings_Click";
			//show the setting tab
			this->tabControl1->SelectedIndex = 1;
			restoreAppWindow();
		}
private: System::Void Form1_FormClosed(System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e) {
			LOG(INFO) << "UI:Form1_FormClosed";
		 }
private: System::Void Form1_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			LOG(INFO) << "UI:Form1_FormClosing";
			if(bExitingApp)
			{

			}
			else
			{
				e->Cancel = true;
				minimizeAppWindow();
			}

		 }
private: System::Void notifyIcon1_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			LOG(INFO) << "UI:notifyIcon1_MouseClick";
			if(e->Clicks == 2)
			{
				openShareFolder();
				return;
			} else if(e->Button == Windows::Forms::MouseButtons::Left){
				restoreAppWindow();
			}
		 }
private: System::Void notifyIcon1_BalloonTipClicked(System::Object^  sender, System::EventArgs^  e) {
			LOG(INFO) << "UI:notifyIcon1_BalloonTipClicked";
			openShareFolder();
		 }

private: System::Void restoreAppWindow(){
			LOG(INFO) << "UI:restoreAppWindow";

//			HWND handle = static_cast<HWND>(this->Handle.ToPointer());
//			AnimateWindow(handle, 1000, AW_ACTIVATE | AW_HOR_POSITIVE);

			this->Show();
			this->WindowState = FormWindowState::Normal;

			HWND handle = static_cast<HWND>(this->Handle.ToPointer());
			ShowWindow(handle, SW_RESTORE);
			SwitchToThisWindow(handle, true);
//			this->ShowInTaskbar = true;
//			this->notifyIcon1->Visible = false;
		}


private: System::Void minimizeAppWindow(){
			LOG(INFO) << "UI:minimizeAppWindow";

//			HWND handle = static_cast<HWND>(this->Handle.ToPointer());
//			AnimateWindow(handle, 1000, AW_HIDE);

//			this->ShowInTaskbar = false;
			this->notifyIcon1->Visible = true;
			this->Hide();
		}


private: System::Void openShareFolder(){
			LOG(INFO) << "UI:openShareFolder";
			 std::string folder("start explorer ");
			 //NOT WORKING FOR NOW
//			 folder.append(iVueceGlobalSetting->publicFolderLocation);
//			 system(folder.c_str());
	}

private: void ReLoadUserData(){
	char buf[1024];
	int len = 0;

	memset(buf, 0, sizeof(buf));

	//load and decrypt user data from registry
	len = VueceWinUtilities::LoadUserData(buf, sizeof(buf));

	if(len == -1)
	{
		LOG(INFO) << "ReLoadUserData user failed, use empty values.";
	}
	else
	{
		VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

		//parse user data
		VueceWinUtilities::ParseUserData(buf, len, iVueceGlobalSetting);

		LOG(INFO) << "ReLoadUserData - User data parsed, accountUserName: " << iVueceGlobalSetting->accountUserName;
		LOG(INFO) << "ReLoadUserData - User data parsed, accountRefreshToken: " << iVueceGlobalSetting->accountRefreshToken;
		LOG(INFO) << "ReLoadUserData - User data parsed, hubName: " << iVueceGlobalSetting->hubName;
		LOG(INFO) << "ReLoadUserData - User data parsed, hubID: " << iVueceGlobalSetting->hubID;
		LOG(INFO) << "ReLoadUserData - User data parsed, number of public folders: " << iVueceGlobalSetting->iPublicFolderList->size();
	}
}

	private: System::Void buttonAddShareFolder_Click(System::Object^  sender, System::EventArgs^  e) {
					if (publicFolderBrowserDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
					  {
						 if(listBoxShareFolders->Items->Contains(publicFolderBrowserDialog->SelectedPath))
						 {
							 return;
						 }
						 else
						 {
							 std::string stdString;

							 listBoxShareFolders->Items->Add(publicFolderBrowserDialog->SelectedPath);

							 VueceWinUtilities::MarshalString((System::String^)publicFolderBrowserDialog->SelectedPath, stdString);

							 LOG(LS_VERBOSE) << "buttonAddShareFolder_Click: " << stdString;

							VueceGlobalContext::AddPublicFolderLocation(stdString);
						 }

					  }
			 }

	private: System::Void buttonRemoveShareFolder_Click(System::Object^  sender, System::EventArgs^  e) {

					while(listBoxShareFolders->SelectedItems->Count!=0)
					{
						std::string stdString;
						VueceWinUtilities::MarshalString((System::String^)listBoxShareFolders->SelectedItems[0], stdString);
						listBoxShareFolders->Items->Remove(listBoxShareFolders->SelectedItems[0]);
						VueceGlobalContext::RemovePublicFolderLocation(stdString);
					}

			 }

	//TODO - Remove this if not used
	private: bool ValidateInput(){
		return true;
	}

private: System::Void btnScanFiles_Click(System::Object^  sender, System::EventArgs^  e) {
			StartFileScanThread(false);
		 }

private: bool StartFileScanThread(bool isSigninStarted) {

				PersistUserData();

				VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

				if(iVueceGlobalSetting->iPublicFolderList->size() == 0){

					if(isSigninStarted){

						MessageBox::Show(this,
										resMgr->GetString("String13_SignInButNoSharedFolderDlgTitle"),
										resMgr->GetString("String14_SignInButNoSharedFolderDlgLable"),
										MessageBoxButtons::OK,
								         MessageBoxIcon::Information);
					}else{

						MessageBox::Show(this,
												resMgr->GetString("String11_ScanButListEmpty"),
												resMgr->GetString("String12_ScanButListEmptyDlgTitle"),
												MessageBoxButtons::OK,
										         MessageBoxIcon::Information);
					}

					return false;
				}

				VueceProgressDialog^ vpd = gcnew VueceProgressDialog();
				vpd->resMgr = this->resMgr;

				if(mediaScannerThread)
				{
					mediaScannerThread->Kill();
					delete mediaScannerThread;

					mediaScannerThread = NULL;
				}

				mediaScannerThread = new VueceMediaDBScannerThread();
				mediaScannerThread->ui = vpd;

				mediaScannerThread->Start();

				vpd->UpdateLableContent(isSigninStarted);
				vpd->ShowDialog();

				return true;
		 }

private: System::Void linkLabel1_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
			 System::Diagnostics::Process::Start( resMgr->GetString("String21_Link_CannotAccessGoogleAccount") );
		 }

private: System::Void linkLabel2_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
			  System::Diagnostics::Process::Start( resMgr->GetString("String22_Link_CreateNewGoogleAccount") );
		 }

private: System::Void checkBoxAutoLoginAtSysStart_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				

			 if(this->checkBoxAutoLoginAtSysStart->Checked )
				{
					LOG(LS_VERBOSE) << "Auto login box is checked.";
					VueceGlobalContext::SetAutoLoginAtSysStartup(true	);
					VueceWinUtilities::EnableAutoStart();
				}
				else
				{
					LOG(LS_VERBOSE) << "Auto login box is unchecked.";
					VueceGlobalContext::SetAutoLoginAtSysStartup(false);
				}
		 }

private: System::Void btnRemoveAllFolders_Click(System::Object^  sender, System::EventArgs^  e) {

			if(listBoxShareFolders->Items->Count > 0)
			{
				if (MessageBox::Show(this,
												resMgr->GetString("String10_RemoveAllSharedFolders_Warning"),
												resMgr->GetString("String11_RemoveAllSharedFolders_Warning_Title"),
												MessageBoxButtons::YesNo,
										         MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
				{
					while(listBoxShareFolders->Items->Count!=0)
					{
						std::string stdString;
						VueceWinUtilities::MarshalString((System::String^)listBoxShareFolders->Items[0], stdString);
						listBoxShareFolders->Items->Remove(listBoxShareFolders->Items[0]);

						VueceGlobalContext::RemovePublicFolderLocation(stdString);
						VueceGlobalContext::SetRescanNeeded(true);
					}

				}
			}
		 }

private: System::Void ListBoxShareFolders_DragEnter( Object^ /*sender*/, System::Windows::Forms::DragEventArgs^ e )
   {
		LOG(LS_VERBOSE) << "An item is being dragged onto share folder list box";

		if(e->Data->GetDataPresent(DataFormats::FileDrop))
				e->Effect = DragDropEffects::All;
			else
				e->Effect = DragDropEffects::None;

   }


private: System::Void ListBoxShareFolders_DragDrop( Object^ /*sender*/, System::Windows::Forms::DragEventArgs^ e )
{
	int i;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	LOG(LS_VERBOSE) << "An item is dropped into share folder list box";

	VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

    // Assign the file names to a String* array, in
   // case the user has selected multiple files.
   array<String^>^files = (array<String^>^)e->Data->GetData( DataFormats::FileDrop );

	for(i = 0; i < files->Length; i++)
	{
		//allow folders only
		pin_ptr<const wchar_t> wname = PtrToStringChars(files[i]);
		HANDLE hFind = FindFirstFile(wname, &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			continue;
		}

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(listBoxShareFolders->Items->Contains( files[i]))
			{
				continue;
			}
			else
			{
				std::string stdString;

				listBoxShareFolders->Items->Add( files[i] );

				VueceWinUtilities::MarshalString( files[i], stdString );

				LOG(LS_VERBOSE) << "ListBoxShareFolders_DragDrop - adding folder:  " << stdString;

				VueceGlobalContext::AddPublicFolderLocation(stdString);
			}

		}
	}
}

private: System::Void checkBoxAutoLoginAtAppStart_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

		if(this->checkBoxAutoLoginAtAppStart->Checked )
			{
				VueceGlobalContext::SetAutoLoginAtAppStartup(true);
			}
			else
			{
				VueceGlobalContext::SetAutoLoginAtAppStartup(false);
			}
		 }

private: System::Void checkAndUpdate(bool bIsAutoStart) {

		char latestVersion[64];
		char currentVersion[64];
		char updateUrl[128];

		LOG(INFO) << "CheckAndUpdate - Start";

		memset(latestVersion, 0, sizeof(latestVersion));
		memset(updateUrl, 0, sizeof(updateUrl));

		VueceWinUtilities::QueryUpdate(latestVersion, updateUrl);

		//compare the version number - 1.0.0.0
		//String^ version_S  = Application::ProductVersion;//System::Reflection::Assembly::GetExecutingAssembly()->GetName()->Version->ToString();
		//std::string appVersion("");
		//VueceWinUtilities::MarshalString(version_S,  appVersion);
		
		sprintf(currentVersion, "%d.%d.%d.%d", VUECE_HUB_VER_MAJOR, VUECE_HUB_VER_MINOR, VUECE_HUB_VER_BUILD, VUECE_HUB_VER_REV);
	
		LOG(INFO) << "CheckAndUpdate - The latest version is: " << latestVersion << ", update URL: " << updateUrl;
		LOG(INFO) << "CheckAndUpdate - Current version is: " << currentVersion;

		//if(VueceWinUtilities::CompareVersion((const char*)appVersion.c_str(), (const char*)latestVersion))
		if(VueceWinUtilities::CompareVersion((const char*)currentVersion, (const char*)latestVersion))
		{
			LOG(INFO) << "CheckAndUpdate - Latest version is newer than current version, trigger update now.";

			String^ currentV = gcnew String(currentVersion);
			String^ lv = gcnew String(latestVersion);
			String^ uu = gcnew String(updateUrl);


			VueceUpdateDialog^ vpd = gcnew VueceUpdateDialog();
			vpd->UpdateLableContent(currentV, lv, uu);

			//display as modeless dialog
			vpd->Show();

//			  if (
//					  (MessageBox::Show(
//							  	  resMgr->GetString(L"String24_AskUpdate"),
//							  	  resMgr->GetString(L"String24_AskUpdateTitle"), MessageBoxButtons::YesNo,
//							  	  	  MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes))
//			  {
//				  //open the update link in default web browser
//				  System::String^  gcStr = gcnew System::String(updateUrl);
//				  System::Diagnostics::Process::Start(gcStr);
//			  }

		}
		else
		{

			if(!bIsAutoStart)
			{
				LOG(INFO) << "CheckAndUpdate - Current version is the latest, no need to update for now.";

				MessageBox::Show(
											  	  resMgr->GetString(L"String26_NoNeedUpdate"),
											  	  resMgr->GetString(L"String25_NoNeedUpdateTitle"), MessageBoxButtons::OK,
											  	  	  MessageBoxIcon::Information);
			}

		}

	}

private: System::Void menuItemAboutVueceHub_Click(System::Object^  sender, System::EventArgs^  e) {
				VueceAboutDialog^ vpd = gcnew VueceAboutDialog();
				vpd->UpdateLableContent();
				vpd->ShowDialog();
		 }
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				jingleClientThread->TestSendFile();
		 }
private: System::Void textBoxHubName_TextChanged(System::Object^  sender, System::EventArgs^  e) {
		 }

private: System::Void button_LinkAccount_Click(System::Object^  sender, System::EventArgs^  e) {

		if( !VueceGlobalContext::IsAccountLinked() )
		{
			StartAccountLinkHandling();
		}
		else
		{
			StartAccountUnLinkHandling();
		}
}

private: System::Void pictureBoxUserImage_Paint( Object^ /*sender*/, System::Windows::Forms::PaintEventArgs^ e )
		 {
			     // Create a local version of the graphics object for the PictureBox.
				Graphics^ g = e->Graphics;
				//
				ControlPaint::DrawBorder(g, pictureBox_UserImg->ClientRectangle, Color::LightSkyBlue, ButtonBorderStyle::Solid);
		 }
		 



private: System::Void ClearUserInfo()
{
	LOG(INFO) << "ClearUserInfo";

	VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

	memset(iVueceGlobalSetting->accountAccessToken, 0, sizeof(iVueceGlobalSetting->accountAccessToken));
	memset(iVueceGlobalSetting->accountRefreshToken, 0, sizeof(iVueceGlobalSetting->accountRefreshToken));
	memset(iVueceGlobalSetting->accountUserName, 0, sizeof(iVueceGlobalSetting->accountUserName));
	memset(iVueceGlobalSetting->accountDisplayName, 0, sizeof(iVueceGlobalSetting->accountDisplayName));
	memset(iVueceGlobalSetting->accountImgUrl, 0, sizeof(iVueceGlobalSetting->accountImgUrl));

	//int option = (int)3/* INTERNET_SUPPRESS_COOKIE_PERSIST*/;
	//int* optionPtr = &option;

	//bool success = InternetSetOption(0, 81/*INTERNET_OPTION_SUPPRESS_BEHAVIOR*/, new IntPtr(optionPtr), sizeof(int));
	//if (!success)
	//{
	//	LOG(INFO) << "ClearUserInfo - Something went wrong ";
	//}
	//else
	//{
	//	LOG(INFO) << "ClearUserInfo - Cookie cleared ";
	//}


//	System::Windows::Forms::WebBrowser  webBrowser1;
//
//	webBrowser1.Navigate("javascript:void((function(){var a,b,c,e,f;f=0;a=document.cookie.split('; ');"
//	            +"for(e=0;e<a.length&&a[e];e++){f++;for(b='.'+location.host;b;b=b.replace(/^(?:%5C.|[^%5C.]+)/,''))"
//	            +"{for(c=location.pathname;c;c=c.replace(/.$/,'')){document.cookie=(a[e]+'; domain='+b+'; path='+c+';"
//	            +"expires='+new Date((new Date()).getTime()-1e11).toGMTString());}}}})())");


}

private: System::Void ShowErrorDialog(String^ title, String^ msg)
{
	MessageBox::Show(this,
					msg,
					title,
					MessageBoxButtons::OK,
			         MessageBoxIcon::Warning);
}

private: System::Void ShowInfoDialog(String^ title, String^ msg)
{
	MessageBox::Show(this,
					msg,
					title,
					MessageBoxButtons::OK,
			         MessageBoxIcon::Information);
}


private: System::Boolean ShowConfirmDialog(String^ title, String^ msg)
{
	if (MessageBox::Show(this,
											msg,
											title,
											MessageBoxButtons::YesNo,
									         MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
	{
		return true;
	}
	else
	{
		return false;
	}

}

private: System::Void StartAccountUnLinkHandling()
{
	LOG(INFO) << "StartAccountUnLinkHandling - Start";

	VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

	if( VueceGlobalContext::IsAccountLinked() )
	{
		//ask user at first
		if (
				!ShowConfirmDialog(resMgr->GetString(L"String27_AskUnlinkTitle"), resMgr->GetString(L"String27_AskUnlink"))
			)
			{
				return;
			}

			button_LinkAccount->Text = resMgr->GetString(L"String28_Unlinking");

			//bool res = VueceWinUtilities::RevokeAccountAccess(iVueceGlobalSetting->accountAccessToken);
			bool res = VueceWinUtilities::RevokeAccountAccess(iVueceGlobalSetting->accountRefreshToken);

			if(res)
			{
				System::Windows::Forms::WebBrowser  webBrowser1;

				webBrowser1.Navigate(L"https://www.google.com/accounts/logout");

				webBrowser1.Refresh();

				//Check if singed in at first, user cannot revoke if not signed in
				if( bSignedIn)
				{
					button_LinkAccount->Text = resMgr->GetString(L"String29_SigningOut");

					//flag this to true to stop connection monitor otherwise reconnection will be triggered
					bSignedOutByUser = true;

					HandleSignInSignOut();
				}


				LOG(INFO) << "Unlink is successful, clear existing user account info";

				VueceGlobalContext::SetAccountLinked(false);

				ClearUserInfo();

				//update UI
				label_DisplayName->Text = resMgr->GetString(L"String31_PleaseLink");
				button_LinkAccount->Text = resMgr->GetString(L"String32_ClickToLink");
				label_AccountName->Text = resMgr->GetString(L"String33_NoLinkedAccount");
				pictureBox_UserImg->Image = this->pictureBox_UserImg->InitialImage;

				button_LinkAccount->ImageKey = L"ic_action_link.png";

				PersistUserData();

				ShowInfoDialog(
						resMgr->GetString(L"String37_UnlinkOkTitle"),
						resMgr->GetString(L"String38_UnlinkOkMsg")
						);
			}
			else
			{
				button_LinkAccount->Text = resMgr->GetString(L"String30_Unlink");

				ShowErrorDialog(
						resMgr->GetString(L"String39_UnlinkErrTitle"),
						resMgr->GetString(L"String40_UnlinkErrTitle")
						);

			}
	}
	else
	{
		// should not happen
		LOG(LS_ERROR) << "StartAccountUnLinkHandling - Unlinking is triggered while user is not linked yet, sth is wrong.";
	}

	LOG(INFO) << "StartAccountUnLinkHandling - End";
}

//private: System::Void InitImageList()
//		 {
//			 this->imageList1->Images->SetKeyName(0, L"ic_action_user.png");
//			this->imageList1->Images->SetKeyName(1, L"ic_action_share_2.png");
//			this->imageList1->Images->SetKeyName(2, L"ic_action_android.png");
//			this->imageList1->Images->SetKeyName(3, L"ic_action_exit.png");
//			this->imageList1->Images->SetKeyName(4, L"ic_action_gear.png");
//			this->imageList1->Images->SetKeyName(5, L"ic_action_export.png");
//			this->imageList1->Images->SetKeyName(6, L"ic_action_globe.png");
//			this->imageList1->Images->SetKeyName(7, L"ic_action_heart.png");
//			this->imageList1->Images->SetKeyName(8, L"ic_action_help.png");
//			this->imageList1->Images->SetKeyName(9, L"ic_action_home.png");
//			this->imageList1->Images->SetKeyName(10, L"ic_action_import.png");
//			this->imageList1->Images->SetKeyName(11, L"ic_action_info.png");
//			this->imageList1->Images->SetKeyName(12, L"ic_action_link.png");
//			this->imageList1->Images->SetKeyName(13, L"ic_action_list.png");
//			this->imageList1->Images->SetKeyName(14, L"ic_action_list_2.png");
//			this->imageList1->Images->SetKeyName(15, L"ic_action_phone.png");
//			this->imageList1->Images->SetKeyName(16, L"ic_action_reload.png");
//			this->imageList1->Images->SetKeyName(17, L"ic_action_share.png");
//			this->imageList1->Images->SetKeyName(18, L"ic_action_warning.png");
//		 }


private: System::Void UpdateUserInfoOnUi(bool linked)
{
	if(linked)
	{
				//populate UI with user account ID and display name
		String^ s_user = gcnew String(VueceGlobalContext::GetAccountUserName());
		String^ s_dsp_name = gcnew String(VueceGlobalContext::GetAccountDisplayName());
		String^ s_img_url = gcnew String(VueceGlobalContext::GetAccountImgUrl());

		button_LinkAccount->Text = resMgr->GetString(L"String30_Unlink");
		button_LinkAccount->ImageKey = L"ic_action_lock_open.png";

		label_AccountName->Text = s_user;

		//update display name
		if(s_dsp_name != "")
		{
			label_DisplayName->Text = s_dsp_name;
		}
		else
		{
			label_DisplayName->Text = resMgr->GetString(L"String42_YourCurAccount");;
		}

		//update image
		if(s_img_url != "")
		{
			this->pictureBox_UserImg->ImageLocation = s_img_url;
		}
		else
		{
			pictureBox_UserImg->Image = this->pictureBox_UserImg->InitialImage;
		}
	}
	else
	{
		button_LinkAccount->Text = resMgr->GetString(L"String32_ClickToLink");
		button_LinkAccount->ImageKey = L"ic_action_link.png";
		//TTT
		pictureBox_UserImg->Image = this->pictureBox_UserImg->InitialImage;

		label_DisplayName->Text = resMgr->GetString(L"String31_PleaseLink");
		button_LinkAccount->Text = resMgr->GetString(L"String32_ClickToLink");
		label_AccountName->Text = resMgr->GetString(L"String33_NoLinkedAccount");

	}

}


private: System::Void StartAccountLinkHandling()
{
					bool link_ok = false;

					char auth_code[512+1];
					std::string tmp_string;

					VueceGlobalSetting* iVueceGlobalSetting = VueceGlobalContext::Instance();

					VueceAuthDialog^ vpd = gcnew VueceAuthDialog();

					LOG(INFO) << "StartAccountLinkHandling - Start";

					button_LinkAccount->Text = resMgr->GetString(L"String41_Authorizing");

					//display as modeless dialog
					memset(auth_code, 0, sizeof(auth_code));
					memset(iVueceGlobalSetting->accountRefreshToken, 0, sizeof(iVueceGlobalSetting->accountRefreshToken));
					memset(iVueceGlobalSetting->accountUserName, 0, sizeof(iVueceGlobalSetting->accountUserName));
					memset(iVueceGlobalSetting->accountDisplayName, 0, sizeof(iVueceGlobalSetting->accountDisplayName));

					vpd->ShowDialog();

					VueceWinUtilities::MarshalString(vpd->authCode, tmp_string);

					strcpy(auth_code, tmp_string.c_str());

					LOG(INFO) << "StartAccountLinkHandling: Auth code acquired: " << auth_code << ", querying access token now.";

					if(strlen(auth_code) == 0)
					{
						//TODO - Should show an error dialog here...
						ShowErrorDialog(resMgr->GetString(L"String35_LinkErrTitle"),
								resMgr->GetString(L"String36_LinkErrMsg"));

						button_LinkAccount->Text = resMgr->GetString(L"String32_ClickToLink");

						return;
					}

					bool bret = VueceWinUtilities::QueryAccessToken (false, (const char*)auth_code,  iVueceGlobalSetting->accountAccessToken, iVueceGlobalSetting->accountRefreshToken);

					if(strlen(iVueceGlobalSetting->accountAccessToken) > 0)
					{
						LOG(LS_VERBOSE) << "StartAccountLinkHandling: Access token acquired: " << iVueceGlobalSetting->accountAccessToken << ", start querying user info";

						VueceWinUtilities::QueryUserAccountInfo (
								(const char*)iVueceGlobalSetting->accountAccessToken,
								iVueceGlobalSetting->accountUserName,
								iVueceGlobalSetting->accountDisplayName,
								iVueceGlobalSetting->accountImgUrl
								);

						LOG(LS_VERBOSE) << "StartAccountLinkHandling: Query result, user ID:  " << iVueceGlobalSetting->accountUserName;
						LOG(LS_VERBOSE) << "StartAccountLinkHandling: Query result, display name:  " << iVueceGlobalSetting->accountDisplayName;
						LOG(LS_VERBOSE) << "StartAccountLinkHandling: Query result, image url:  " << iVueceGlobalSetting->accountImgUrl;

						if(strlen(iVueceGlobalSetting->accountUserName) > 0)
						{
							UpdateUserInfoOnUi(true);
							
							this->buttonSignIn->Enabled = true;

							iVueceGlobalSetting->accountLinked = true;
							
							link_ok = true;

							PersistUserData();

							LOG(LS_VERBOSE) << "StartAccountLinkHandling: Account is flagged as linked";
						}
						else
						{
							//do nothing, error will be check by link_ok flag
						}
					}
					else
					{
						//do nothing, error will be check by link_ok flag
					}
					//OATH2 - END
					////////////////////////////////////////////////////////////////////////////////////////////////////////////

					if(!link_ok)
					{
						ShowErrorDialog(L"Linking failed", L"An error occurred during linking process, please try again.");

						ClearUserInfo();
					}

					LOG(INFO) << "StartAccountLinkHandling - Done";
}

private: System::Void checkBoxAllowFriendAccess_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

				if(this->checkBoxAllowFriendAccess->Checked )
				{
					VueceGlobalContext::SetAllowFriendAccess(true);
				}
				else
				{
					VueceGlobalContext::SetAllowFriendAccess(false);
				}
		 }

private: System::Void numericMaxConcurrentStreaming_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 LOG(INFO) << "numericMaxConcurrentStreaming_ValueChanged - " << (int)numericMaxConcurrentStreaming->Value;
				VueceGlobalContext::SetMaxConcurrentStreaming((int)numericMaxConcurrentStreaming->Value);
		 }
};
}


