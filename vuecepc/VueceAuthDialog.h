#pragma once

#include "VueceWinUtilities.h"
#include "talk/base/logging.h"

//			this->webBrowser1->Url = (gcnew System::Uri(OATH2_REQ_URL, System::UriKind::Absolute));
//#define OATH2_REQ_URL "https://accounts.google.com/o/oauth2/auth?scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fgoogletalk+email&redirect_uri=urn:ietf:wg:oauth:2.0:oob&response_type=code&client_id=489257891728-rftvnu18uv50q61qjrq5quu3b1p9s54s.apps.googleusercontent.com"

namespace vuecepc {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Resources;
	using namespace System::Reflection;

	/// <summary>
	/// Summary for VueceAuthDialog
	/// </summary>
	public ref class VueceAuthDialog : public System::Windows::Forms::Form
	{
	public:
		VueceAuthDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			InitBrowserUrl();

		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VueceAuthDialog()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::WebBrowser^  webBrowser1;
	ResourceManager^ strResMgr;		
	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

	public: String^ authCode;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{	
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(VueceAuthDialog::typeid));
			this->webBrowser1 = (gcnew System::Windows::Forms::WebBrowser());
			this->SuspendLayout();
			// 
			// webBrowser1
			// 
			resources->ApplyResources(this->webBrowser1, L"webBrowser1");
			this->webBrowser1->MinimumSize = System::Drawing::Size(27, 25);
			this->webBrowser1->Name = L"webBrowser1";
			this->webBrowser1->ScriptErrorsSuppressed = true;
			this->webBrowser1->DocumentTitleChanged += gcnew System::EventHandler(this, &VueceAuthDialog::WebBrowser1_DocumentTitleChanged);
			// 
			// VueceAuthDialog
			// 
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->webBrowser1);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VueceAuthDialog";
			this->ResumeLayout(false);

		}
#pragma endregion

			void InitBrowserUrl()
			{
					strResMgr = gcnew ResourceManager("VuecePC.VueceStrings", Assembly::GetExecutingAssembly());
				   this->webBrowser1->Url = (gcnew System::Uri(strResMgr->GetString("OATH2_REQ_URL"), System::UriKind::Absolute));
			}

		   // Updates the title bar with the current document title.
		   void WebBrowser1_DocumentTitleChanged( Object^ /*sender*/, System::EventArgs^ /*e*/ )
		   {
			   //https://accounts.google.com/o/oauth2/auth?scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fgoogletalk&redirect_uri=urn:ietf:wg:oauth:2.0:oob&response_type=code&client_id=489257891728-rftvnu18uv50q61qjrq5quu3b1p9s54s.apps.googleusercontent.com
			   //Example response: "Success code=4/AltjGklf3qe_KiVdRczjw2XVkptmlJpxmV2ALHPdQ_c"
			   if(webBrowser1->DocumentTitle->StartsWith("Success code=",  StringComparison::CurrentCultureIgnoreCase) )
			   {

					std::string buf;
					VueceWinUtilities::MarshalString(webBrowser1->DocumentTitle, buf);
					LOG(LS_VERBOSE) << "Received response: " << buf;

				   this->authCode = webBrowser1->DocumentTitle->Substring(webBrowser1->DocumentTitle->IndexOf("=")+1);
				   this->authCode =  this->authCode->Trim();

					VueceWinUtilities::MarshalString(this->authCode, buf);
					LOG(LS_VERBOSE) << "Partial token extracted: " << buf;

					//locate the actual token value
//					int pos_start = webBrowser1->DocumentText->LastIndexOf(authCode);
//					int pos_end = webBrowser1->DocumentText->IndexOf("\"", pos_start);
//					this->authCode = webBrowser1->DocumentText->Substring(pos_start, pos_end-pos_start);
					
					VueceWinUtilities::MarshalString(this->authCode, buf);
					LOG(LS_VERBOSE) << "Complete token extracted: " << buf;

				    this->Hide();
			   }
			   else
			   {
				   this->Text = webBrowser1->DocumentTitle;
				   this->authCode =  L"";
			   }
		   }

	};
}
