
namespace OvenConfigurator
{
    partial class fmOvenStatus
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
            this.components = new System.ComponentModel.Container();
            this.lblOvenStateTxt = new System.Windows.Forms.Label();
            this.panelOvenState = new System.Windows.Forms.Panel();
            this.lblOvenState = new System.Windows.Forms.Label();
            this.panelOvenTemperature = new System.Windows.Forms.Panel();
            this.lblCurrentTempC = new System.Windows.Forms.Label();
            this.lblCurrentTemp = new System.Windows.Forms.Label();
            this.lblCurrentTempTxt = new System.Windows.Forms.Label();
            this.panelOvenTimeLeft = new System.Windows.Forms.Panel();
            this.lblCookSeconds = new System.Windows.Forms.Label();
            this.lblCookSecondsLeft = new System.Windows.Forms.Label();
            this.lblCookMinutes = new System.Windows.Forms.Label();
            this.lblCookMinutesLeft = new System.Windows.Forms.Label();
            this.lblCookHours = new System.Windows.Forms.Label();
            this.lblCookHoursLeft = new System.Windows.Forms.Label();
            this.lblTimeRemaining = new System.Windows.Forms.Label();
            this.timerStatusUpdate = new System.Windows.Forms.Timer(this.components);
            this.panelOvenState.SuspendLayout();
            this.panelOvenTemperature.SuspendLayout();
            this.panelOvenTimeLeft.SuspendLayout();
            this.SuspendLayout();
            // 
            // lblOvenStateTxt
            // 
            this.lblOvenStateTxt.AutoSize = true;
            this.lblOvenStateTxt.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblOvenStateTxt.Location = new System.Drawing.Point(7, 7);
            this.lblOvenStateTxt.Name = "lblOvenStateTxt";
            this.lblOvenStateTxt.Size = new System.Drawing.Size(83, 17);
            this.lblOvenStateTxt.TabIndex = 0;
            this.lblOvenStateTxt.Text = "Oven State:";
            // 
            // panelOvenState
            // 
            this.panelOvenState.Controls.Add(this.lblOvenState);
            this.panelOvenState.Controls.Add(this.lblOvenStateTxt);
            this.panelOvenState.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelOvenState.Location = new System.Drawing.Point(0, 0);
            this.panelOvenState.Name = "panelOvenState";
            this.panelOvenState.Padding = new System.Windows.Forms.Padding(7);
            this.panelOvenState.Size = new System.Drawing.Size(732, 36);
            this.panelOvenState.TabIndex = 1;
            // 
            // lblOvenState
            // 
            this.lblOvenState.AutoSize = true;
            this.lblOvenState.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblOvenState.Location = new System.Drawing.Point(90, 7);
            this.lblOvenState.Name = "lblOvenState";
            this.lblOvenState.Size = new System.Drawing.Size(66, 17);
            this.lblOvenState.TabIndex = 1;
            this.lblOvenState.Text = "Unknown";
            // 
            // panelOvenTemperature
            // 
            this.panelOvenTemperature.Controls.Add(this.lblCurrentTempC);
            this.panelOvenTemperature.Controls.Add(this.lblCurrentTemp);
            this.panelOvenTemperature.Controls.Add(this.lblCurrentTempTxt);
            this.panelOvenTemperature.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelOvenTemperature.Location = new System.Drawing.Point(0, 36);
            this.panelOvenTemperature.Name = "panelOvenTemperature";
            this.panelOvenTemperature.Padding = new System.Windows.Forms.Padding(7);
            this.panelOvenTemperature.Size = new System.Drawing.Size(732, 36);
            this.panelOvenTemperature.TabIndex = 2;
            // 
            // lblCurrentTempC
            // 
            this.lblCurrentTempC.AutoSize = true;
            this.lblCurrentTempC.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCurrentTempC.Location = new System.Drawing.Point(176, 7);
            this.lblCurrentTempC.Name = "lblCurrentTempC";
            this.lblCurrentTempC.Size = new System.Drawing.Size(23, 17);
            this.lblCurrentTempC.TabIndex = 3;
            this.lblCurrentTempC.Text = "°C";
            // 
            // lblCurrentTemp
            // 
            this.lblCurrentTemp.AutoSize = true;
            this.lblCurrentTemp.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCurrentTemp.Location = new System.Drawing.Point(152, 7);
            this.lblCurrentTemp.Name = "lblCurrentTemp";
            this.lblCurrentTemp.Size = new System.Drawing.Size(24, 17);
            this.lblCurrentTemp.TabIndex = 1;
            this.lblCurrentTemp.Text = "30";
            // 
            // lblCurrentTempTxt
            // 
            this.lblCurrentTempTxt.AutoSize = true;
            this.lblCurrentTempTxt.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCurrentTempTxt.Location = new System.Drawing.Point(7, 7);
            this.lblCurrentTempTxt.Name = "lblCurrentTempTxt";
            this.lblCurrentTempTxt.Size = new System.Drawing.Size(145, 17);
            this.lblCurrentTempTxt.TabIndex = 0;
            this.lblCurrentTempTxt.Text = "Current Temperature:";
            // 
            // panelOvenTimeLeft
            // 
            this.panelOvenTimeLeft.Controls.Add(this.lblCookSeconds);
            this.panelOvenTimeLeft.Controls.Add(this.lblCookSecondsLeft);
            this.panelOvenTimeLeft.Controls.Add(this.lblCookMinutes);
            this.panelOvenTimeLeft.Controls.Add(this.lblCookMinutesLeft);
            this.panelOvenTimeLeft.Controls.Add(this.lblCookHours);
            this.panelOvenTimeLeft.Controls.Add(this.lblCookHoursLeft);
            this.panelOvenTimeLeft.Controls.Add(this.lblTimeRemaining);
            this.panelOvenTimeLeft.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelOvenTimeLeft.Location = new System.Drawing.Point(0, 72);
            this.panelOvenTimeLeft.Name = "panelOvenTimeLeft";
            this.panelOvenTimeLeft.Padding = new System.Windows.Forms.Padding(7);
            this.panelOvenTimeLeft.Size = new System.Drawing.Size(732, 36);
            this.panelOvenTimeLeft.TabIndex = 3;
            // 
            // lblCookSeconds
            // 
            this.lblCookSeconds.AutoSize = true;
            this.lblCookSeconds.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookSeconds.Location = new System.Drawing.Point(328, 7);
            this.lblCookSeconds.Name = "lblCookSeconds";
            this.lblCookSeconds.Size = new System.Drawing.Size(73, 17);
            this.lblCookSeconds.TabIndex = 6;
            this.lblCookSeconds.Text = "Second(s)";
            // 
            // lblCookSecondsLeft
            // 
            this.lblCookSecondsLeft.AutoSize = true;
            this.lblCookSecondsLeft.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookSecondsLeft.Location = new System.Drawing.Point(312, 7);
            this.lblCookSecondsLeft.Name = "lblCookSecondsLeft";
            this.lblCookSecondsLeft.Size = new System.Drawing.Size(16, 17);
            this.lblCookSecondsLeft.TabIndex = 5;
            this.lblCookSecondsLeft.Text = "0";
            // 
            // lblCookMinutes
            // 
            this.lblCookMinutes.AutoSize = true;
            this.lblCookMinutes.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookMinutes.Location = new System.Drawing.Point(245, 7);
            this.lblCookMinutes.Name = "lblCookMinutes";
            this.lblCookMinutes.Size = new System.Drawing.Size(67, 17);
            this.lblCookMinutes.TabIndex = 4;
            this.lblCookMinutes.Text = "Minute(s)";
            // 
            // lblCookMinutesLeft
            // 
            this.lblCookMinutesLeft.AutoSize = true;
            this.lblCookMinutesLeft.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookMinutesLeft.Location = new System.Drawing.Point(229, 7);
            this.lblCookMinutesLeft.Name = "lblCookMinutesLeft";
            this.lblCookMinutesLeft.Size = new System.Drawing.Size(16, 17);
            this.lblCookMinutesLeft.TabIndex = 3;
            this.lblCookMinutesLeft.Text = "0";
            // 
            // lblCookHours
            // 
            this.lblCookHours.AutoSize = true;
            this.lblCookHours.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookHours.Location = new System.Drawing.Point(173, 7);
            this.lblCookHours.Name = "lblCookHours";
            this.lblCookHours.Size = new System.Drawing.Size(56, 17);
            this.lblCookHours.TabIndex = 2;
            this.lblCookHours.Text = "Hour(s)";
            // 
            // lblCookHoursLeft
            // 
            this.lblCookHoursLeft.AutoSize = true;
            this.lblCookHoursLeft.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblCookHoursLeft.Location = new System.Drawing.Point(157, 7);
            this.lblCookHoursLeft.Name = "lblCookHoursLeft";
            this.lblCookHoursLeft.Size = new System.Drawing.Size(16, 17);
            this.lblCookHoursLeft.TabIndex = 1;
            this.lblCookHoursLeft.Text = "0";
            // 
            // lblTimeRemaining
            // 
            this.lblTimeRemaining.AutoSize = true;
            this.lblTimeRemaining.Dock = System.Windows.Forms.DockStyle.Left;
            this.lblTimeRemaining.Location = new System.Drawing.Point(7, 7);
            this.lblTimeRemaining.Name = "lblTimeRemaining";
            this.lblTimeRemaining.Size = new System.Drawing.Size(150, 17);
            this.lblTimeRemaining.TabIndex = 0;
            this.lblTimeRemaining.Text = "Cook Time Remaining:";
            // 
            // timerStatusUpdate
            // 
            this.timerStatusUpdate.Enabled = true;
            this.timerStatusUpdate.Interval = 1000;
            this.timerStatusUpdate.Tick += new System.EventHandler(this.timerStatusUpdate_Tick);
            // 
            // fmOvenStatus
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(230)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.ClientSize = new System.Drawing.Size(732, 417);
            this.Controls.Add(this.panelOvenTimeLeft);
            this.Controls.Add(this.panelOvenTemperature);
            this.Controls.Add(this.panelOvenState);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "fmOvenStatus";
            this.Text = "fmOvenStatus";
            this.panelOvenState.ResumeLayout(false);
            this.panelOvenState.PerformLayout();
            this.panelOvenTemperature.ResumeLayout(false);
            this.panelOvenTemperature.PerformLayout();
            this.panelOvenTimeLeft.ResumeLayout(false);
            this.panelOvenTimeLeft.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label lblOvenStateTxt;
        private System.Windows.Forms.Panel panelOvenState;
        private System.Windows.Forms.Label lblOvenState;
        private System.Windows.Forms.Panel panelOvenTemperature;
        private System.Windows.Forms.Label lblCurrentTemp;
        private System.Windows.Forms.Label lblCurrentTempTxt;
        private System.Windows.Forms.Label lblCurrentTempC;
        private System.Windows.Forms.Panel panelOvenTimeLeft;
        private System.Windows.Forms.Label lblCookHoursLeft;
        private System.Windows.Forms.Label lblTimeRemaining;
        private System.Windows.Forms.Label lblCookSeconds;
        private System.Windows.Forms.Label lblCookSecondsLeft;
        private System.Windows.Forms.Label lblCookMinutes;
        private System.Windows.Forms.Label lblCookMinutesLeft;
        private System.Windows.Forms.Label lblCookHours;
        private System.Windows.Forms.Timer timerStatusUpdate;
    }
}