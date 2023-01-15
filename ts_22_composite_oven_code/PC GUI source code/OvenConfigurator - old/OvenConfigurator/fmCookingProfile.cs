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

            updownCookTemp.Value = OvenCookProfile.CookTemp;
            updownHeatRate.Value = OvenCookProfile.HeatRate;
            updownCoolRate.Value = OvenCookProfile.CoolRate;
            updownCookHours.Value = OvenCookProfile.CookHours;
            updownCookMinutes.Value = OvenCookProfile.CookMinutes;
            updownCookSeconds.Value = OvenCookProfile.CookSeconds;
        }

        private void updownCookTemp_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookTemp = (int)updownCookTemp.Value;
        }

        private void updownHeatRate_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.HeatRate = updownHeatRate.Value;
        }

        private void updownCoolRate_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CoolRate = updownCoolRate.Value;
        }

        private void updownCookHours_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookHours = (int)updownCookHours.Value;
        }

        private void updownCookMinutes_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookMinutes = (int)updownCookMinutes.Value;
        }

        private void updownCookSeconds_ValueChanged(object sender, EventArgs e)
        {
            OvenCookProfile.CookSeconds = (int)updownCookSeconds.Value;
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
            updownCookTemp.Value = int.Parse(lines[3].Split('=')[1]);
            updownHeatRate.Value = decimal.Parse(lines[4].Split('=')[1]);
            updownCoolRate.Value = decimal.Parse(lines[5].Split('=')[1]);

            OvenCookProfile.CookTemp = (int)updownCookTemp.Value;
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

            string[] WriteString = new string[6];

            WriteString[0] = "COOK_HOUR=" + updownCookHours.Value.ToString();
            WriteString[1] = "COOK_MINUTE=" + updownCookMinutes.Value.ToString();
            WriteString[2] = "COOK_SECOND=" + updownCookSeconds.Value.ToString();
            WriteString[3] = "COOK_TEMP=" + updownCookTemp.Value.ToString();
            WriteString[4] = "HEAT_RATE=" + (1*updownHeatRate.Value).ToString();
            WriteString[5] = "COOL_RATE=" + (1*updownCoolRate.Value).ToString();

            string StringToWrite = "";
            for (int i = 0; i < 6; i++)
            {
                StringToWrite += WriteString[i] + "\r\n";
            }

            System.IO.File.WriteAllText(dlg.FileName, StringToWrite);
        }
    }
}
