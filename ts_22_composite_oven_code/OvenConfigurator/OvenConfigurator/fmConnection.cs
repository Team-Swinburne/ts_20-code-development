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
using NModbus;
using NModbus.Serial;

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
                SerialConnection._SerialPort.ReadTimeout = 1000;
                SerialConnection._SerialPort.WriteTimeout = 1000;
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
                /*
                byte[] Data = new byte[7];

                Data[0] = 0x22; //Cook command signature

                Data[1] = (byte)OvenCookProfile.CookHours;
                Data[2] = (byte)OvenCookProfile.CookMinutes;
                Data[3] = (byte)OvenCookProfile.CookSeconds;
                Data[4] = (byte)OvenCookProfile.CookTemp;
                Data[5] = (byte)(100 * OvenCookProfile.HeatRate);
                Data[6] = (byte)(100 * OvenCookProfile.CoolRate);

                SerialConnection._SerialPort.Write(Data, 0, 7);
                Data[0] = 1;
                */

                try
                {
                    var factory = new ModbusFactory();
                    IModbusMaster master = factory.CreateRtuMaster(SerialConnection._SerialPort);

                    ushort[] Data = new ushort[8];

                    Data[0] = (ushort)OvenCookProfile.CookHours;
                    Data[1] = (ushort)OvenCookProfile.CookMinutes;
                    Data[2] = (ushort)OvenCookProfile.CookSeconds;
                    Data[3] = (ushort)OvenCookProfile.BeginTemp;
                    Data[4] = (ushort)OvenCookProfile.CookTemp;
                    Data[5] = (ushort)OvenCookProfile.FinishTemp;
                    Data[6] = (ushort)(100 * OvenCookProfile.HeatRate);
                    Data[7] = (ushort)(100 * OvenCookProfile.CoolRate);

                    try
                    {
                        master.WriteMultipleRegisters(0x01, 0x1000, Data);
                    }
                    catch(Exception)
                    {

                    }

                    master.WriteSingleCoil(0x01, 0x01, true);
                }
                catch(Exception)
                {
                    return;
                }
            }
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            //activate stop sequence
            if (SerialConnection._SerialPort.IsOpen)
            {
                var factory = new ModbusFactory();
                IModbusMaster master = factory.CreateRtuMaster(SerialConnection._SerialPort);

                master.WriteSingleCoil(0x01, 0x02, true);
            }
        }
    }
}
