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
    public partial class MainWindow : Form
    {
        Form activeForm = null;
        bool ConnectButtonStatus = true;
        SerialPort _SerialPort = new SerialPort();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void loadChildWindow(Form childForm)
        {
            if (activeForm != null) activeForm.Close();

            activeForm = childForm;

            childForm.TopLevel = false;
            childForm.FormBorderStyle = FormBorderStyle.None;
            childForm.Dock = DockStyle.Fill;
            panelDisplayWindow.Controls.Add(childForm);
            panelDisplayWindow.Tag = childForm;
            childForm.BringToFront();
            childForm.Show();
        }

        private void btnConnection_Click(object sender, EventArgs e)
        {
            ClearButtonsHighlight();
            btnConnection.BackColor = Color.FromArgb(255, 200, 200, 200);
            loadChildWindow(new fmConnection());
        }

        private void btnCookingProfile_Click(object sender, EventArgs e)
        {
            ClearButtonsHighlight();
            btnCookingProfile.BackColor = Color.FromArgb(255, 200, 200, 200);
            loadChildWindow(new fmCookingProfile());
        }

        private void btnOvenStatus_Click(object sender, EventArgs e)
        {
            ClearButtonsHighlight();
            btnOvenStatus.BackColor = Color.FromArgb(255, 200, 200, 200);
            loadChildWindow(new fmOvenStatus());
        }

        private void ClearButtonsHighlight()
        {
            btnConnection.BackColor = Color.FromArgb(255, 160, 160, 160);
            btnCookingProfile.BackColor = Color.FromArgb(255, 160, 160, 160);
            btnOvenStatus.BackColor = Color.FromArgb(255, 160, 160, 160);
        }
    }

    static public class OvenCookProfile
    {
        static public int CookTemp = 30;
        static public decimal HeatRate = 1;
        static public decimal CoolRate = 1;
        static public int CookHours = 0;
        static public int CookMinutes = 0;
        static public int CookSeconds = 0;
    }

    static public class SerialConnection
    {
        static public SerialPort _SerialPort = new SerialPort();

        static public bool Connect(string ComPortName)
        {
            _SerialPort.PortName = ComPortName;
            _SerialPort.BaudRate = 9600;
            _SerialPort.ReadTimeout = 50;
            _SerialPort.WriteTimeout = 50;
            _SerialPort.ReadBufferSize = 6;

            try
            {
                _SerialPort.Open();
            }
            catch (Exception)
            {
                return false;
            }

            _SerialPort.DiscardInBuffer();

            return true;
        }
    }
}
