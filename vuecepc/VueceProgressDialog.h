#pragma once

#include "VueceProgressUI.h"

namespace vuecepc {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Resources;

	/// <summary>
	/// Summary for VueceProgressDialog
	/// </summary>
	public ref class VueceProgressDialog : public System::Windows::Forms::Form, VueceProgressUI
	{
	public:
		VueceProgressDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			deletageShowMsg = gcnew DelegateMethodShowMsg( this, &vuecepc::VueceProgressDialog::UpdateProgressMsg );
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VueceProgressDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel1;
	protected: 
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::Label^  labelMainMsg;

	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel2;


	private: System::Windows::Forms::ProgressBar^  progressBar1;

	private: delegate void DelegateMethodHide(int i);
	private: delegate void DelegateMethodShowMsg(int hint, String^ msg);

	private: DelegateMethodHide^ delegateHide;
	private: DelegateMethodShowMsg^ deletageShowMsg;


	private: System::Windows::Forms::Label^  labelMessage1;
	public: ResourceManager^ resMgr;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel3;
	public: 
	private: System::Windows::Forms::Label^  labelMessage2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(VueceProgressDialog::typeid));
			this->flowLayoutPanel1 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->labelMainMsg = (gcnew System::Windows::Forms::Label());
			this->flowLayoutPanel2 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->flowLayoutPanel3 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->labelMessage1 = (gcnew System::Windows::Forms::Label());
			this->labelMessage2 = (gcnew System::Windows::Forms::Label());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->flowLayoutPanel1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->flowLayoutPanel2->SuspendLayout();
			this->flowLayoutPanel3->SuspendLayout();
			this->SuspendLayout();
			// 
			// flowLayoutPanel1
			// 
			resources->ApplyResources(this->flowLayoutPanel1, L"flowLayoutPanel1");
			this->flowLayoutPanel1->Controls->Add(this->pictureBox1);
			this->flowLayoutPanel1->Controls->Add(this->labelMainMsg);
			this->flowLayoutPanel1->Name = L"flowLayoutPanel1";
			// 
			// pictureBox1
			// 
			this->pictureBox1->BackColor = System::Drawing::Color::Transparent;
			resources->ApplyResources(this->pictureBox1, L"pictureBox1");
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->TabStop = false;
			// 
			// labelMainMsg
			// 
			resources->ApplyResources(this->labelMainMsg, L"labelMainMsg");
			this->labelMainMsg->BackColor = System::Drawing::Color::Transparent;
			this->labelMainMsg->Name = L"labelMainMsg";
			// 
			// flowLayoutPanel2
			// 
			this->flowLayoutPanel2->Controls->Add(this->flowLayoutPanel3);
			this->flowLayoutPanel2->Controls->Add(this->progressBar1);
			resources->ApplyResources(this->flowLayoutPanel2, L"flowLayoutPanel2");
			this->flowLayoutPanel2->Name = L"flowLayoutPanel2";
			// 
			// flowLayoutPanel3
			// 
			this->flowLayoutPanel3->Controls->Add(this->labelMessage1);
			this->flowLayoutPanel3->Controls->Add(this->labelMessage2);
			resources->ApplyResources(this->flowLayoutPanel3, L"flowLayoutPanel3");
			this->flowLayoutPanel3->Name = L"flowLayoutPanel3";
			// 
			// labelMessage1
			// 
			resources->ApplyResources(this->labelMessage1, L"labelMessage1");
			this->labelMessage1->Name = L"labelMessage1";
			// 
			// labelMessage2
			// 
			resources->ApplyResources(this->labelMessage2, L"labelMessage2");
			this->labelMessage2->Name = L"labelMessage2";
			// 
			// progressBar1
			// 
			this->progressBar1->BackColor = System::Drawing::SystemColors::GradientActiveCaption;
			this->progressBar1->ForeColor = System::Drawing::Color::Green;
			resources->ApplyResources(this->progressBar1, L"progressBar1");
			this->progressBar1->MarqueeAnimationSpeed = 15;
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Style = System::Windows::Forms::ProgressBarStyle::Marquee;
			this->progressBar1->Value = 60;
			// 
			// VueceProgressDialog
			// 
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::ButtonFace;
			this->ControlBox = false;
			this->Controls->Add(this->flowLayoutPanel2);
			this->Controls->Add(this->flowLayoutPanel1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VueceProgressDialog";
			this->flowLayoutPanel1->ResumeLayout(false);
			this->flowLayoutPanel1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->flowLayoutPanel2->ResumeLayout(false);
			this->flowLayoutPanel3->ResumeLayout(false);
			this->flowLayoutPanel3->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion


	public: virtual void OnFinish(int numScannedItem)
	{
		if (this->InvokeRequired)
		{
			array<Object^>^myArray = {numScannedItem};

			this->Invoke(
					gcnew DelegateMethodHide(this, &vuecepc::VueceProgressDialog::OnFinish), myArray
			);
			return;
		}
		else
		{
			this->Hide();
		}

	}

	public: void UpdateLableContent(bool isSigninStarted)
	{
		this->Text = resMgr->GetString(L"String8_Scanning");

		if(isSigninStarted)
		{
			this->labelMainMsg->Text = resMgr->GetString(L"String15_ScanDialogLable_BeforeSigin");
		}
		else
		{
			this->labelMainMsg->Text = resMgr->GetString(L"String10_ScanDialogLable");
		}
	}

	public: void UpdateProgressMsg(int hint, String^ msg)
	{
		if(hint == VueceProgressUI::MSG_HINT_SCANNING){
			this->labelMessage1->Text = resMgr->GetString(L"String8_Scanning");
		}else if(hint == VueceProgressUI::MSG_HINT_SCAN_DONE){
			this->labelMessage1->Text = resMgr->GetString(L"String9_UpdatingDb");
		}

		this->labelMessage2->Text = msg;
	}

	public: virtual void OnMessage(int hint, const std::string& msg)
	{
		if (this->InvokeRequired)
		{
			String^s = gcnew String(msg.c_str());
			array<Object^>^myStringArray = {hint, s};
			this->Invoke( deletageShowMsg, myStringArray );
		}
		else
		{
			String^s = gcnew String(msg.c_str());
			UpdateProgressMsg(hint, s);
		}
	}


};
}
