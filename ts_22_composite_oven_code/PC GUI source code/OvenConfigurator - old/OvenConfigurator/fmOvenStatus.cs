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
    public partial class fmOvenStatus : Form
    {
        public fmOvenStatus()
        {
            InitializeComponent();

            if(SerialConnection._SerialPort.IsOpen) SerialConnection._SerialPort.DiscardInBuffer();
        }

        private void timerStatusUpdate_Tick(object sender, EventArgs e)
        {
            if (SerialConnection._SerialPort.IsOpen)
            {
                byte[] Data = new byte[6];

                try
                {
                    SerialConnection._SerialPort.Read(Data, 0, 6);
                }
                catch (Exception)
                {
                    return;
                }

                if (Data[0] != 0x55) return;

                if ((Data[1] < 1) || (Data[1] > 4)) return;

                string OvenState = "Idle";

                switch (Data[1])
                {
                    case 1:
                        OvenState = "Idle";
                        break;

                    case 2:
                        OvenState = "Heating";
                        break;

                    case 3:
                        OvenState = "Cooking";
                        break;

                    case 4:
                        OvenState = "Cooling";
                        break;
                }

                lblOvenState.Text = OvenState;
                lblCurrentTemp.Text = Data[2].ToString();
                lblCookHoursLeft.Text = Data[3].ToString();
                lblCookMinutesLeft.Text = Data[4].ToString();
                lblCookSecondsLeft.Text = Data[5].ToString();
            }
        }
    }
}
