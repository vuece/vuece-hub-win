#pragma once

#include <stdio.h>
#include <string.h>
#include "VueceHubVersion.h"

namespace vuecepc {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for VueceAboutDialog
	/// </summary>
	public ref class VueceAboutDialog : public System::Windows::Forms::Form
	{
	public:
		VueceAboutDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//

			UpdateLableContent();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VueceAboutDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  panel1;
	protected: 
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::Button^  buttonOk;
	private: System::Windows::Forms::LinkLabel^  linkLabelVueceWebSite;
	private: System::Windows::Forms::Label^  labelCopyright;
	private: System::Windows::Forms::Label^  labelVersionNumber;
	private: System::Windows::Forms::Panel^  panel3;

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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(VueceAboutDialog::typeid));
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->linkLabelVueceWebSite = (gcnew System::Windows::Forms::LinkLabel());
			this->labelCopyright = (gcnew System::Windows::Forms::Label());
			this->labelVersionNumber = (gcnew System::Windows::Forms::Label());
			this->buttonOk = (gcnew System::Windows::Forms::Button());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->panel1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->panel2->SuspendLayout();
			this->panel3->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			resources->ApplyResources(this->panel1, L"panel1");
			this->panel1->Controls->Add(this->label1);
			this->panel1->Controls->Add(this->pictureBox1);
			this->panel1->Name = L"panel1";
			// 
			// label1
			// 
			this->label1->BackColor = System::Drawing::Color::Transparent;
			resources->ApplyResources(this->label1, L"label1");
			this->label1->ForeColor = System::Drawing::SystemColors::InactiveCaptionText;
			this->label1->Name = L"label1";
			// 
			// pictureBox1
			// 
			this->pictureBox1->BackColor = System::Drawing::Color::Transparent;
			resources->ApplyResources(this->pictureBox1, L"pictureBox1");
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->TabStop = false;
			// 
			// panel2
			// 
			this->panel2->Controls->Add(this->linkLabelVueceWebSite);
			this->panel2->Controls->Add(this->labelCopyright);
			this->panel2->Controls->Add(this->labelVersionNumber);
			resources->ApplyResources(this->panel2, L"panel2");
			this->panel2->Name = L"panel2";
			// 
			// linkLabelVueceWebSite
			// 
			resources->ApplyResources(this->linkLabelVueceWebSite, L"linkLabelVueceWebSite");
			this->linkLabelVueceWebSite->Name = L"linkLabelVueceWebSite";
			this->linkLabelVueceWebSite->TabStop = true;
			this->linkLabelVueceWebSite->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VueceAboutDialog::linkLabelVueceWebSite_LinkClicked);
			// 
			// labelCopyright
			// 
			resources->ApplyResources(this->labelCopyright, L"labelCopyright");
			this->labelCopyright->Name = L"labelCopyright";
			// 
			// labelVersionNumber
			// 
			resources->ApplyResources(this->labelVersionNumber, L"labelVersionNumber");
			this->labelVersionNumber->Name = L"labelVersionNumber";
			// 
			// buttonOk
			// 
			resources->ApplyResources(this->buttonOk, L"buttonOk");
			this->buttonOk->Name = L"buttonOk";
			this->buttonOk->UseVisualStyleBackColor = true;
			this->buttonOk->Click += gcnew System::EventHandler(this, &VueceAboutDialog::buttonOk_Click);
			// 
			// panel3
			// 
			this->panel3->Controls->Add(this->buttonOk);
			resources->ApplyResources(this->panel3, L"panel3");
			this->panel3->Name = L"panel3";
			// 
			// VueceAboutDialog
			// 
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->panel3);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->panel1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VueceAboutDialog";
			this->panel1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->panel3->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void linkLabelVueceWebSite_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
				 System::Diagnostics::Process::Start( VUECE_WEB_URL );
			 }

	private: System::Void buttonOk_Click(System::Object^  sender, System::EventArgs^  e) {
			 this->Hide();
		 }

	public: void UpdateLableContent()
	{
		char ver[64+1];

		memset(ver, 0, sizeof(ver));
		sprintf(ver, "%s v%d.%d.%d", VUECE_PRODUCT_NAME,
				VUECE_HUB_VER_MAJOR,
				VUECE_HUB_VER_MINOR,
				VUECE_HUB_VER_BUILD);

		String^name = gcnew String(ver);
		String^cp = gcnew String(VUECE_COPY_RIGHT);
//		String^ version  = System::Reflection::Assembly::GetExecutingAssembly()->GetName()->Version->ToString();
//		String^versionLableText = String::Concat(name, version);

		this->labelVersionNumber->Text = name;
		this->labelCopyright->Text = cp;
	}


};
}
