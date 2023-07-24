using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace OvenConfigurator
{
    public partial class fmCookingProfile : Form
    {
        public fmCookingProfile()
        {
            InitializeComponent();

            updownBeginTemp.Value = OvenCookProfile.BeginTemp;
            updownCookTemp.Value = OvenCookProfile.CookTemp;
            updownFinishTemp.Value = OvenCookProfile.FinishTemp;
            updownHeatRate.Value = OvenCookProfile.HeatRate;
            updownCoolRate.Value = OvenCookProfile.CoolRate;
            updownCookHours.Value = OvenCookProfile.CookHours;
            updownCookMinutes.Value = OvenCookProfile.CookMinutes;
            updownCookSeconds.Value = OvenCookProfile.CookSeconds;
        }

        private void updownBeginTemp_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.BeginTemp = (int)updownBeginTemp.Value;
            UpDateTimeTaken();
        }

        private void updownCookTemp_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookTemp = (int)updownCookTemp.Value;
            UpDateTimeTaken();
        }

        private void updownFinishTemp_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.FinishTemp = (int)updownFinishTemp.Value;
            UpDateTimeTaken();
        }

        private void updownHeatRate_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.HeatRate = updownHeatRate.Value;
            UpDateTimeTaken();
        }

        private void updownCoolRate_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CoolRate = updownCoolRate.Value;
            UpDateTimeTaken();
        }

        private void updownCookHours_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookHours = (int)updownCookHours.Value;
            UpDateTimeTaken();
        }

        private void updownCookMinutes_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookMinutes = (int)updownCookMinutes.Value;
            UpDateTimeTaken();
        }

        private void updownCookSeconds_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookSeconds = (int)updownCookSeconds.Value;
            UpDateTimeTaken();
        }

        private void UpDateTimeTaken()
        {
            decimal HeatTime = (OvenCookProfile.CookTemp - OvenCookProfile.BeginTemp) * ((decimal)60/OvenCookProfile.HeatRate);
            int CookTime = 3600*OvenCookProfile.CookHours + 60*OvenCookProfile.CookMinutes + OvenCookProfile.CookSeconds;
            decimal CoolTime = (OvenCookProfile.CookTemp - OvenCookProfile.FinishTemp) * ((decimal)60 / OvenCookProfile.CoolRate);

            if (HeatTime < 0) HeatTime = 0;
            if (CookTime < 0) CookTime = 0;
            if (CoolTime < 0) CoolTime = 0;

            int TotalTime = (int)HeatTime + CookTime + (int)CoolTime;

            if (TotalTime < 0) TotalTime = 0;

            lblHeatHour.Text = ((int)HeatTime / 3600).ToString();
            lblHeatMinute.Text = ((int)(HeatTime % 3600)/60).ToString();
            lblHeatSecond.Text = ((int)(HeatTime % 3600) % 60).ToString();

            lblCookHour.Text = (CookTime / 3600).ToString();
            lblCookMinute.Text = ((CookTime % 3600) / 60).ToString();
            lblCookSecond.Text = ((CookTime % 3600) % 60).ToString();

            lblCoolHour.Text = ((int)(CoolTime / 3600)).ToString();
            lblCoolMinute.Text = ((int)(CoolTime % 3600) / 60).ToString();
            lblCoolSecond.Text = ((int)(CoolTime % 3600) % 60).ToString();

            lblTotalHour.Text = (TotalTime / 3600).ToString();
            lblTotalMinute.Text = ((TotalTime % 3600) / 60).ToString();
            lblTotalSecond.Text = ((TotalTime % 3600) % 60).ToString();
        }

        private void btnLoadProfile_Click(object sender, EventArgs e)
        {
            var dlg = new OpenFileDialog();
            dlg.InitialDirectory = Application.StartupPath;
            dlg.Filter = "Cooking Profile (*.dat)|*.dat";

            if (dlg.ShowDialog() != DialogResult.OK)
            {
                return;
            }

            string[] lines = System.IO.File.ReadAllLines(dlg.FileName);

            updownCookHours.Value = int.Parse(lines[0].Split('=')[1]);
            updownCookMinutes.Value = int.Parse(lines[1].Split('=')[1]);
            updownCookSeconds.Value = int.Parse(lines[2].Split('=')[1]);
            updownBeginTemp.Value = int.Parse(lines[3].Split('=')[1]);
            updownCookTemp.Value = int.Parse(lines[4].Split('=')[1]);
            updownFinishTemp.Value = int.Parse(lines[5].Split('=')[1]);
            updownHeatRate.Value = decimal.Parse(lines[6].Split('=')[1]);
            updownCoolRate.Value = decimal.Parse(lines[7].Split('=')[1]);

            OvenCookProfile.BeginTemp = (int)updownBeginTemp.Value;
            OvenCookProfile.CookTemp = (int)updownCookTemp.Value;
            OvenCookProfile.FinishTemp = (int)updownFinishTemp.Value;
            OvenCookProfile.HeatRate = updownHeatRate.Value;
            OvenCookProfile.CoolRate = updownCoolRate.Value;
            OvenCookProfile.CookHours = (int)updownCookHours.Value;
            OvenCookProfile.CookMinutes = (int)updownCookMinutes.Value;
            OvenCookProfile.CookSeconds = (int)updownCookSeconds.Value;
        }

        private void btnSaveProfile_Click(object sender, EventArgs e)
        {
            var dlg = new SaveFileDialog();
            dlg.InitialDirectory = Application.StartupPath;
            dlg.Filter = "Cooking Profile (*.dat)|*.dat";

            if (dlg.ShowDialog() != DialogResult.OK)
            {
                return;
            }

            string[] WriteString = new string[8];

            WriteString[0] = "COOK_HOUR=" + updownCookHours.Value.ToString();
            WriteString[1] = "COOK_MINUTE=" + updownCookMinutes.Value.ToString();
            WriteString[2] = "COOK_SECOND=" + updownCookSeconds.Value.ToString();
            WriteString[3] = "BEGIN_TEMP=" + updownBeginTemp.Value.ToString();
            WriteString[4] = "COOK_TEMP=" + updownCookTemp.Value.ToString();
            WriteString[5] = "FINISH_TEMP=" + updownFinishTemp.Value.ToString();
            WriteString[6] = "HEAT_RATE=" + (1*updownHeatRate.Value).ToString();
            WriteString[7] = "COOL_RATE=" + (1*updownCoolRate.Value).ToString();

            string StringToWrite = "";
            for (int i = 0; i < WriteString.Length; i++)
            {
                StringToWrite += WriteString[i] + "\r\n";
            }

            System.IO.File.WriteAllText(dlg.FileName, StringToWrite);
        }

    }
}
