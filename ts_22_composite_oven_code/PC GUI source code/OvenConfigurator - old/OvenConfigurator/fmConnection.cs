using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace OvenConfigurator
{
    public partial class fmConnection : Form
    {
        bool ConnectButtonStatus = false;

        public fmConnection()
        {
            InitializeComponent();

            if(SerialConnection._SerialPort.IsOpen)
            {
                btnConnect.Text = "Disconnect";
                ConnectButtonStatus = true;
                comboPortSelect.Items.Add(SerialConnection._SerialPort.PortName);
                comboPortSelect.SelectedIndex = 0;
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (ConnectButtonStatus == false)
            {
                if (comboPortSelect.Text == "") return;

                if (!SerialConnection.Connect(comboPortSelect.Text)) return;

                SerialConnection._SerialPort.DiscardInBuffer();
                btnConnect.Text = "Disconnect";
                ConnectButtonStatus = true;
            }
            else
            {
                SerialConnection._SerialPort.Close();
                btnConnect.Text = "Connect";
                ConnectButtonStatus = false;
            }
        }

        private void comboPortSelect_Click(object sender, EventArgs e)
        {
            if(comboPortSelect.Text == "")
            {
                comboPortSelect.Items.Clear();
                comboPortSelect.Items.AddRange(SerialPort.GetPortNames());
            }
        }

        private void btnCook_Click(object sender, EventArgs e)
        {

            //activate cook sequence
            if (SerialConnection._SerialPort.IsOpen)
            {
                byte[] Data = new byte[7];

                Data[0] = 0x22; //Cook command signature

                Data[1] = (byte)OvenCookProfile.CookHours;
                Data[2] = (byte)OvenCookProfile.CookMinutes;
                Data[3] = (byte)OvenCookProfile.CookSeconds;
                Data[4] = (byte)OvenCookProfile.CookTemp;
                Data[5] = (byte)(10 * OvenCookProfile.HeatRate);
                Data[6] = (byte)(10 * OvenCookProfile.CoolRate);

                SerialConnection._SerialPort.Write(Data, 0, 7);
                Data[0] = 1;
            }
        }
    }
}
