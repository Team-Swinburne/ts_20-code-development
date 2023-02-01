
namespace OvenConfigurator
{
    partial class fmCookingProfile
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label6 = new System.Windows.Forms.Label();
            this.updownCookTemp = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label4 = new System.Windows.Forms.Label();
            this.updownCoolRate = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.updownHeatRate = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label9 = new System.Windows.Forms.Label();
            this.updownCookSeconds = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.updownCookMinutes = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.updownCookHours = new System.Windows.Forms.NumericUpDown();
            this.panel1 = new System.Windows.Forms.Panel();
            this.btnSaveProfile = new System.Windows.Forms.Button();
            this.btnLoadProfile = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookTemp)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCoolRate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownHeatRate)).BeginInit();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookSeconds)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookMinutes)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookHours)).BeginInit();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.updownCookTemp);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox1.Location = new System.Drawing.Point(10, 10);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(10, 5, 5, 5);
            this.groupBox1.Size = new System.Drawing.Size(477, 51);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Temperature";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Dock = System.Windows.Forms.DockStyle.Left;
            this.label6.Location = new System.Drawing.Point(191, 20);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(23, 17);
            this.label6.TabIndex = 6;
            this.label6.Text = "°C";
            // 
            // updownCookTemp
            // 
            this.updownCookTemp.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownCookTemp.Location = new System.Drawing.Point(140, 20);
            this.updownCookTemp.Maximum = new decimal(new int[] {
            204,
            0,
            0,
            0});
            this.updownCookTemp.Minimum = new decimal(new int[] {
            30,
            0,
            0,
            0});
            this.updownCookTemp.Name = "updownCookTemp";
            this.updownCookTemp.Size = new System.Drawing.Size(51, 22);
            this.updownCookTemp.TabIndex = 1;
            this.updownCookTemp.Value = new decimal(new int[] {
            30,
            0,
            0,
            0});
            this.updownCookTemp.ValueChanged += new System.EventHandler(this.updownCookTemp_ValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Dock = System.Windows.Forms.DockStyle.Left;
            this.label1.Location = new System.Drawing.Point(10, 20);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(130, 17);
            this.label1.TabIndex = 1;
            this.label1.Text = "Cook Temperature:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.updownCoolRate);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.updownHeatRate);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox2.Location = new System.Drawing.Point(10, 61);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(10, 5, 5, 5);
            this.groupBox2.Size = new System.Drawing.Size(477, 51);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Heat/Cool rate";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Dock = System.Windows.Forms.DockStyle.Left;
            this.label4.Location = new System.Drawing.Point(411, 20);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(49, 17);
            this.label4.TabIndex = 5;
            this.label4.Text = "°C/min";
            // 
            // updownCoolRate
            // 
            this.updownCoolRate.DecimalPlaces = 1;
            this.updownCoolRate.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownCoolRate.Location = new System.Drawing.Point(360, 20);
            this.updownCoolRate.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.updownCoolRate.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.updownCoolRate.Name = "updownCoolRate";
            this.updownCoolRate.Size = new System.Drawing.Size(51, 22);
            this.updownCoolRate.TabIndex = 3;
            this.updownCoolRate.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.updownCoolRate.ValueChanged += new System.EventHandler(this.updownCoolRate_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Dock = System.Windows.Forms.DockStyle.Left;
            this.label5.Location = new System.Drawing.Point(181, 20);
            this.label5.Name = "label5";
            this.label5.Padding = new System.Windows.Forms.Padding(110, 0, 0, 0);
            this.label5.Size = new System.Drawing.Size(179, 17);
            this.label5.TabIndex = 4;
            this.label5.Text = "Cool rate:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Dock = System.Windows.Forms.DockStyle.Left;
            this.label3.Location = new System.Drawing.Point(132, 20);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(49, 17);
            this.label3.TabIndex = 2;
            this.label3.Text = "°C/min";
            // 
            // updownHeatRate
            // 
            this.updownHeatRate.DecimalPlaces = 1;
            this.updownHeatRate.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownHeatRate.Location = new System.Drawing.Point(81, 20);
            this.updownHeatRate.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.updownHeatRate.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.updownHeatRate.Name = "updownHeatRate";
            this.updownHeatRate.Size = new System.Drawing.Size(51, 22);
            this.updownHeatRate.TabIndex = 1;
            this.updownHeatRate.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.updownHeatRate.ValueChanged += new System.EventHandler(this.updownHeatRate_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Dock = System.Windows.Forms.DockStyle.Left;
            this.label2.Location = new System.Drawing.Point(10, 20);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(71, 17);
            this.label2.TabIndex = 1;
            this.label2.Text = "Heat rate:";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.label9);
            this.groupBox3.Controls.Add(this.updownCookSeconds);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.updownCookMinutes);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Controls.Add(this.updownCookHours);
            this.groupBox3.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox3.Location = new System.Drawing.Point(10, 112);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Padding = new System.Windows.Forms.Padding(10, 5, 5, 5);
            this.groupBox3.Size = new System.Drawing.Size(477, 51);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Cook time";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Dock = System.Windows.Forms.DockStyle.Left;
            this.label9.Location = new System.Drawing.Point(286, 20);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(73, 17);
            this.label9.TabIndex = 12;
            this.label9.Text = "Second(s)";
            // 
            // updownCookSeconds
            // 
            this.updownCookSeconds.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownCookSeconds.Location = new System.Drawing.Point(235, 20);
            this.updownCookSeconds.Maximum = new decimal(new int[] {
            59,
            0,
            0,
            0});
            this.updownCookSeconds.Name = "updownCookSeconds";
            this.updownCookSeconds.Size = new System.Drawing.Size(51, 22);
            this.updownCookSeconds.TabIndex = 11;
            this.updownCookSeconds.ValueChanged += new System.EventHandler(this.updownCookSeconds_ValueChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Dock = System.Windows.Forms.DockStyle.Left;
            this.label7.Location = new System.Drawing.Point(168, 20);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(67, 17);
            this.label7.TabIndex = 10;
            this.label7.Text = "Minute(s)";
            // 
            // updownCookMinutes
            // 
            this.updownCookMinutes.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownCookMinutes.Location = new System.Drawing.Point(117, 20);
            this.updownCookMinutes.Maximum = new decimal(new int[] {
            59,
            0,
            0,
            0});
            this.updownCookMinutes.Name = "updownCookMinutes";
            this.updownCookMinutes.Size = new System.Drawing.Size(51, 22);
            this.updownCookMinutes.TabIndex = 9;
            this.updownCookMinutes.ValueChanged += new System.EventHandler(this.updownCookMinutes_ValueChanged);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Dock = System.Windows.Forms.DockStyle.Left;
            this.label8.Location = new System.Drawing.Point(61, 20);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(56, 17);
            this.label8.TabIndex = 8;
            this.label8.Text = "Hour(s)";
            // 
            // updownCookHours
            // 
            this.updownCookHours.Dock = System.Windows.Forms.DockStyle.Left;
            this.updownCookHours.Location = new System.Drawing.Point(10, 20);
            this.updownCookHours.Maximum = new decimal(new int[] {
            99,
            0,
            0,
            0});
            this.updownCookHours.Name = "updownCookHours";
            this.updownCookHours.Size = new System.Drawing.Size(51, 22);
            this.updownCookHours.TabIndex = 7;
            this.updownCookHours.ValueChanged += new System.EventHandler(this.updownCookHours_ValueChanged);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.btnSaveProfile);
            this.panel1.Controls.Add(this.btnLoadProfile);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(10, 163);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(477, 45);
            this.panel1.TabIndex = 4;
            // 
            // btnSaveProfile
            // 
            this.btnSaveProfile.Location = new System.Drawing.Point(246, 0);
            this.btnSaveProfile.Name = "btnSaveProfile";
            this.btnSaveProfile.Size = new System.Drawing.Size(231, 45);
            this.btnSaveProfile.TabIndex = 4;
            this.btnSaveProfile.Text = "Save Profile";
            this.btnSaveProfile.UseVisualStyleBackColor = true;
            this.btnSaveProfile.Click += new System.EventHandler(this.btnSaveProfile_Click);
            // 
            // btnLoadProfile
            // 
            this.btnLoadProfile.Dock = System.Windows.Forms.DockStyle.Left;
            this.btnLoadProfile.Location = new System.Drawing.Point(0, 0);
            this.btnLoadProfile.Name = "btnLoadProfile";
            this.btnLoadProfile.Size = new System.Drawing.Size(235, 45);
            this.btnLoadProfile.TabIndex = 3;
            this.btnLoadProfile.Text = "Load Profile";
            this.btnLoadProfile.UseVisualStyleBackColor = true;
            this.btnLoadProfile.Click += new System.EventHandler(this.btnLoadProfile_Click);
            // 
            // fmCookingProfile
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(230)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.ClientSize = new System.Drawing.Size(497, 417);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "fmCookingProfile";
            this.Padding = new System.Windows.Forms.Padding(10);
            this.Text = "fmCookingProfile";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookTemp)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCoolRate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownHeatRate)).EndInit();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookSeconds)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookMinutes)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.updownCookHours)).EndInit();
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.NumericUpDown updownCookTemp;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.NumericUpDown updownHeatRate;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown updownCoolRate;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.NumericUpDown updownCookSeconds;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown updownCookMinutes;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown updownCookHours;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button btnSaveProfile;
        private System.Windows.Forms.Button btnLoadProfile;
    }
}