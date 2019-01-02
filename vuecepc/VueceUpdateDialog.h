#pragma once

namespace vuecepc {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for VueceUpdateDialog
	/// </summary>
	public ref class VueceUpdateDialog : public System::Windows::Forms::Form
	{
	public:
		VueceUpdateDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VueceUpdateDialog()
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
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Button^  buttonYes;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanel1;
	private: System::Windows::Forms::Button^  buttonNo;

	private: String^ currentVersion;
	private: String^ latestVersion;
	private: String^ updateUrl;

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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(VueceUpdateDialog::typeid));
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->buttonYes = (gcnew System::Windows::Forms::Button());
			this->flowLayoutPanel1 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->buttonNo = (gcnew System::Windows::Forms::Button());
			this->panel1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->flowLayoutPanel1->SuspendLayout();
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
			resources->ApplyResources(this->label1, L"label1");
			this->label1->BackColor = System::Drawing::Color::Transparent;
			this->label1->ForeColor = System::Drawing::SystemColors::ControlText;
			this->label1->Name = L"label1";
			// 
			// pictureBox1
			// 
			this->pictureBox1->BackColor = System::Drawing::Color::Transparent;
			resources->ApplyResources(this->pictureBox1, L"pictureBox1");
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->TabStop = false;
			// 
			// label2
			// 
			resources->ApplyResources(this->label2, L"label2");
			this->label2->Name = L"label2";
			// 
			// buttonYes
			// 
			resources->ApplyResources(this->buttonYes, L"buttonYes");
			this->buttonYes->Name = L"buttonYes";
			this->buttonYes->UseVisualStyleBackColor = true;
			this->buttonYes->Click += gcnew System::EventHandler(this, &VueceUpdateDialog::buttonYes_Click);
			// 
			// flowLayoutPanel1
			// 
			this->flowLayoutPanel1->Controls->Add(this->buttonNo);
			this->flowLayoutPanel1->Controls->Add(this->buttonYes);
			resources->ApplyResources(this->flowLayoutPanel1, L"flowLayoutPanel1");
			this->flowLayoutPanel1->Name = L"flowLayoutPanel1";
			// 
			// buttonNo
			// 
			this->buttonNo->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			resources->ApplyResources(this->buttonNo, L"buttonNo");
			this->buttonNo->Name = L"buttonNo";
			this->buttonNo->UseVisualStyleBackColor = true;
			this->buttonNo->Click += gcnew System::EventHandler(this, &VueceUpdateDialog::buttonNo_Click);
			// 
			// VueceUpdateDialog
			// 
			this->AcceptButton = this->buttonYes;
			resources->ApplyResources(this, L"$this");
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->buttonNo;
			this->Controls->Add(this->flowLayoutPanel1);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->panel1);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VueceUpdateDialog";
			this->TopMost = true;
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->flowLayoutPanel1->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	private: System::Void buttonYes_Click(System::Object^  sender, System::EventArgs^  e) {
					System::Diagnostics::Process::Start(this->updateUrl);
					this->Hide();
			 }

	private: System::Void buttonNo_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->Hide();
			 }

	public: void UpdateLableContent(String^ currentVersion, String^ latestVersion, String^ updateUrl)
		{
			this->currentVersion = currentVersion;
			this->latestVersion = latestVersion;
			this->updateUrl = updateUrl;
		}
};
}
