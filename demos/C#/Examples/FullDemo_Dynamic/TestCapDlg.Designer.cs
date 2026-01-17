
namespace TWAINDemo
{
    partial class TestCapDlg
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cmbGetTypes = new System.Windows.Forms.ComboBox();
            this.cmbContainer = new System.Windows.Forms.ComboBox();
            this.cmbDataType = new System.Windows.Forms.ComboBox();
            this.btnStartTest = new System.Windows.Forms.Button();
            this.btnReset = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.lstResults = new System.Windows.Forms.ListBox();
            this.button1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(35, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(403, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Note: Testing using non-default container or data type may have undesirable resul" +
    "ts.";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(40, 74);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(76, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Get Operation:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(203, 74);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Container:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(373, 74);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(60, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Data Type:";
            // 
            // cmbGetTypes
            // 
            this.cmbGetTypes.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbGetTypes.FormattingEnabled = true;
            this.cmbGetTypes.Location = new System.Drawing.Point(43, 91);
            this.cmbGetTypes.Name = "cmbGetTypes";
            this.cmbGetTypes.Size = new System.Drawing.Size(121, 21);
            this.cmbGetTypes.TabIndex = 4;
            this.cmbGetTypes.SelectedIndexChanged += new System.EventHandler(this.cmbGetTypes_SelectedIndexChanged);
            // 
            // cmbContainer
            // 
            this.cmbContainer.FormattingEnabled = true;
            this.cmbContainer.Location = new System.Drawing.Point(206, 90);
            this.cmbContainer.Name = "cmbContainer";
            this.cmbContainer.Size = new System.Drawing.Size(145, 21);
            this.cmbContainer.TabIndex = 5;
            // 
            // cmbDataType
            // 
            this.cmbDataType.FormattingEnabled = true;
            this.cmbDataType.Location = new System.Drawing.Point(376, 90);
            this.cmbDataType.Name = "cmbDataType";
            this.cmbDataType.Size = new System.Drawing.Size(121, 21);
            this.cmbDataType.TabIndex = 6;
            // 
            // btnStartTest
            // 
            this.btnStartTest.Location = new System.Drawing.Point(148, 140);
            this.btnStartTest.Name = "btnStartTest";
            this.btnStartTest.Size = new System.Drawing.Size(75, 23);
            this.btnStartTest.TabIndex = 7;
            this.btnStartTest.Text = "Start Test";
            this.btnStartTest.UseVisualStyleBackColor = true;
            this.btnStartTest.Click += new System.EventHandler(this.btnStartTest_Click);
            // 
            // btnReset
            // 
            this.btnReset.Location = new System.Drawing.Point(284, 140);
            this.btnReset.Name = "btnReset";
            this.btnReset.Size = new System.Drawing.Size(101, 23);
            this.btnReset.TabIndex = 8;
            this.btnReset.Text = "Reset To Default";
            this.btnReset.UseVisualStyleBackColor = true;
            this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(39, 216);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(45, 13);
            this.label5.TabIndex = 9;
            this.label5.Text = "Results:";
            // 
            // lstResults
            // 
            this.lstResults.FormattingEnabled = true;
            this.lstResults.Location = new System.Drawing.Point(42, 245);
            this.lstResults.Name = "lstResults";
            this.lstResults.Size = new System.Drawing.Size(451, 95);
            this.lstResults.TabIndex = 10;
            // 
            // button1
            // 
            this.button1.AllowDrop = true;
            this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button1.Location = new System.Drawing.Point(226, 359);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 11;
            this.button1.Text = "Ok";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // TestCapDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(532, 402);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.lstResults);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.btnReset);
            this.Controls.Add(this.btnStartTest);
            this.Controls.Add(this.cmbDataType);
            this.Controls.Add(this.cmbContainer);
            this.Controls.Add(this.cmbGetTypes);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "TestCapDlg";
            this.Text = "TestCapDlg";
            this.Load += new System.EventHandler(this.TestCapDlg_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cmbGetTypes;
        private System.Windows.Forms.ComboBox cmbContainer;
        private System.Windows.Forms.ComboBox cmbDataType;
        private System.Windows.Forms.Button btnStartTest;
        private System.Windows.Forms.Button btnReset;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ListBox lstResults;
        private System.Windows.Forms.Button button1;
    }
}