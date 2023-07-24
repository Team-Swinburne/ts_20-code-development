
namespace OvenConfigurator
{
    partial class MainWindow
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.panelSideBar = new System.Windows.Forms.Panel();
            this.btnOvenStatus = new System.Windows.Forms.Button();
            this.btnCookingProfile = new System.Windows.Forms.Button();
            this.btnConnection = new System.Windows.Forms.Button();
            this.panelLogo = new System.Windows.Forms.Panel();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.panelDisplayWindow = new System.Windows.Forms.Panel();
            this.panelSideBar.SuspendLayout();
            this.panelLogo.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // panelSideBar
            // 
            this.panelSideBar.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(127)))), ((int)(((byte)(127)))), ((int)(((byte)(127)))));
            this.panelSideBar.Controls.Add(this.btnOvenStatus);
            this.panelSideBar.Controls.Add(this.btnCookingProfile);
            this.panelSideBar.Controls.Add(this.btnConnection);
            this.panelSideBar.Controls.Add(this.panelLogo);
            this.panelSideBar.Dock = System.Windows.Forms.DockStyle.Left;
            this.panelSideBar.Location = new System.Drawing.Point(0, 0);
            this.panelSideBar.Margin = new System.Windows.Forms.Padding(4);
            this.panelSideBar.Name = "panelSideBar";
            this.panelSideBar.Size = new System.Drawing.Size(200, 440);
            this.panelSideBar.TabIndex = 0;
            // 
            // btnOvenStatus
            // 
            this.btnOvenStatus.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(160)))), ((int)(((byte)(160)))), ((int)(((byte)(160)))));
            this.btnOvenStatus.Dock = System.Windows.Forms.DockStyle.Top;
            this.btnOvenStatus.FlatAppearance.BorderColor = System.Drawing.Color.White;
            this.btnOvenStatus.FlatAppearance.BorderSize = 0;
            this.btnOvenStatus.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
            this.btnOvenStatus.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnOvenStatus.Location = new System.Drawing.Point(0, 377);
            this.btnOvenStatus.Margin = new System.Windows.Forms.Padding(4);
            this.btnOvenStatus.Name = "btnOvenStatus";
            this.btnOvenStatus.Size = new System.Drawing.Size(200, 62);
            this.btnOvenStatus.TabIndex = 3;
            this.btnOvenStatus.Text = "Oven Status";
            this.btnOvenStatus.UseVisualStyleBackColor = false;
            this.btnOvenStatus.Click += new System.EventHandler(this.btnOvenStatus_Click);
            // 
            // btnCookingProfile
            // 
            this.btnCookingProfile.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(160)))), ((int)(((byte)(160)))), ((int)(((byte)(160)))));
            this.btnCookingProfile.Dock = System.Windows.Forms.DockStyle.Top;
            this.btnCookingProfile.FlatAppearance.BorderSize = 0;
            this.btnCookingProfile.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
            this.btnCookingProfile.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnCookingProfile.Location = new System.Drawing.Point(0, 315);
            this.btnCookingProfile.Margin = new System.Windows.Forms.Padding(4);
            this.btnCookingProfile.Name = "btnCookingProfile";
            this.btnCookingProfile.Size = new System.Drawing.Size(200, 62);
            this.btnCookingProfile.TabIndex = 2;
            this.btnCookingProfile.Text = "Cooking Profile";
            this.btnCookingProfile.UseVisualStyleBackColor = false;
            this.btnCookingProfile.Click += new System.EventHandler(this.btnCookingProfile_Click);
            // 
            // btnConnection
            // 
            this.btnConnection.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(160)))), ((int)(((byte)(160)))), ((int)(((byte)(160)))));
            this.btnConnection.Dock = System.Windows.Forms.DockStyle.Top;
            this.btnConnection.FlatAppearance.BorderSize = 0;
            this.btnConnection.FlatAppearance.MouseDownBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
            this.btnConnection.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnConnection.Location = new System.Drawing.Point(0, 253);
            this.btnConnection.Margin = new System.Windows.Forms.Padding(4);
            this.btnConnection.Name = "btnConnection";
            this.btnConnection.Size = new System.Drawing.Size(200, 62);
            this.btnConnection.TabIndex = 1;
            this.btnConnection.Text = "Connection";
            this.btnConnection.UseVisualStyleBackColor = false;
            this.btnConnection.Click += new System.EventHandler(this.btnConnection_Click);
            // 
            // panelLogo
            // 
            this.panelLogo.Controls.Add(this.label2);
            this.panelLogo.Controls.Add(this.label1);
            this.panelLogo.Controls.Add(this.pictureBox1);
            this.panelLogo.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelLogo.Location = new System.Drawing.Point(0, 0);
            this.panelLogo.Margin = new System.Windows.Forms.Padding(4);
            this.panelLogo.Name = "panelLogo";
            this.panelLogo.Size = new System.Drawing.Size(200, 253);
            this.panelLogo.TabIndex = 0;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Dock = System.Windows.Forms.DockStyle.Top;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.ForeColor = System.Drawing.SystemColors.ButtonHighlight;
            this.label2.Location = new System.Drawing.Point(0, 225);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(189, 25);
            this.label2.TabIndex = 1;
            this.label2.Text = "Oven Configurator";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Dock = System.Windows.Forms.DockStyle.Top;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.SystemColors.ButtonHighlight;
            this.label1.Location = new System.Drawing.Point(0, 200);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(175, 25);
            this.label1.TabIndex = 1;
            this.label1.Text = "Team Swinburne";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Dock = System.Windows.Forms.DockStyle.Top;
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.InitialImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.InitialImage")));
            this.pictureBox1.Location = new System.Drawing.Point(0, 0);
            this.pictureBox1.Margin = new System.Windows.Forms.Padding(4);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(200, 200);
            this.pictureBox1.TabIndex = 1;
            this.pictureBox1.TabStop = false;
            // 
            // panelDisplayWindow
            // 
            this.panelDisplayWindow.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(230)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.panelDisplayWindow.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelDisplayWindow.Location = new System.Drawing.Point(200, 0);
            this.panelDisplayWindow.Name = "panelDisplayWindow";
            this.panelDisplayWindow.Size = new System.Drawing.Size(732, 440);
            this.panelDisplayWindow.TabIndex = 1;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(10F, 22F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(932, 440);
            this.Controls.Add(this.panelDisplayWindow);
            this.Controls.Add(this.panelSideBar);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.MinimumSize = new System.Drawing.Size(950, 487);
            this.Name = "MainWindow";
            this.Text = "Composite Oven Configurator";
            this.panelSideBar.ResumeLayout(false);
            this.panelLogo.ResumeLayout(false);
            this.panelLogo.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panelSideBar;
        private System.Windows.Forms.Button btnOvenStatus;
        private System.Windows.Forms.Button btnCookingProfile;
        private System.Windows.Forms.Button btnConnection;
        private System.Windows.Forms.Panel panelLogo;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Panel panelDisplayWindow;
    }
}

