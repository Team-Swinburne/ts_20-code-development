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
                try
                {
                    var factory = new ModbusFactory();
                    IModbusMaster master = factory.CreateRtuMaster(SerialConnection._SerialPort);

                    byte slaveId = 1;
                    ushort startAddress = 100;
                    ushort[] registers = new ushort[] { 1, 2, 3 };

                    // write three registers
                    //master.WriteMultipleRegisters(slaveId, startAddress, registers);
                    ushort[] value = master.ReadInputRegisters(slaveId, 0x2000, 5);

                    string OvenState = "Idle";

                    switch (value[0])
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
                    lblCurrentTemp.Text = value[1].ToString();
                    lblCookHoursLeft.Text = value[2].ToString();
                    lblCookMinutesLeft.Text = value[3].ToString();
                    lblCookSecondsLeft.Text = value[4].ToString();
                }
                catch(Exception)
                {
                    return;
                }

                /*
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
                */
            }
        }
    }
}
