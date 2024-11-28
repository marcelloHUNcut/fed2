using System;
using System.IO.Ports;
using System.Text;
using System.Windows.Forms;

namespace visuals
{
    public partial class Form1 : Form
    {
        public Label lbl_pos;
        private SerialPort serialPort;

        // Változók tárolják az aktuális pozíciót és orientációt
        int pos_x = 0;
        int pos_y = 0;
        int orientation = 0;

        private const byte StartByte = 0x02; // Start byte
        private const byte EndByte = 0x03;   // End byte

        private List<byte> buffer = new List<byte>();


        public Form1()
        {
            InitializeComponent();

            // Label inicializálása
            lbl_pos = new Label();
            lbl_pos.Location = new Point(1500, 10);
            lbl_pos.Size = new Size(500, 30);
            lbl_pos.Text = "Waiting for position...";
            this.Controls.Add(lbl_pos);

            // SerialPort inicializálása
            serialPort = new SerialPort("COM10", 9600);
            serialPort.DataBits = 8;
            serialPort.Parity = Parity.None;
            serialPort.StopBits = StopBits.One;
            serialPort.Handshake = Handshake.None;
            serialPort.DtrEnable = false;
            serialPort.RtsEnable = false;

            // Eseménykezelő regisztrálása
            serialPort.DataReceived += SerialPort_DataReceived;

            // Soros port megnyitása
            try
            {
                serialPort.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Hiba a soros port megnyitásakor: {ex.Message}");
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                // Beérkező üzenet olvasása byte-okként
                int bytesToRead = serialPort.BytesToRead;
                byte[] bufferBytes = new byte[bytesToRead];
                serialPort.Read(bufferBytes, 0, bytesToRead);

                // Feldolgozzuk a byte-okat
                foreach (byte b in bufferBytes)
                {
                    // Ha találunk kezdő byte-ot, új üzenetet kezdünk
                    if (b == StartByte)
                    {
                        buffer.Clear(); // Üzenet törlése
                    }

                    // A byte hozzáadása az aktuális bufferhez
                    buffer.Add(b);

                    // Ha találunk záró byte-ot, az üzenet befejeződött
                    if (b == EndByte)
                    {
                        ProcessMessage(buffer);  // Üzenet feldolgozása
                        buffer.Clear();  // Buffer ürítése a következő üzenethez
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Hiba az adatok feldolgozásakor: {ex.Message}");
            }
        }
        private void ProcessMessage(List<byte> message)
        {
            // Ellenőrizzük, hogy az üzenet megfelelő start és end byte-tal rendelkezik
            if (message.Count < 8 || message[0] != StartByte || message[^1] != EndByte)
                return; // Minimum 8 byte (Start + 6 adat + End)

            // Start és End byte eltávolítása
            message.RemoveAt(0); // Start byte
            message.RemoveAt(message.Count - 1); // End byte

            // Ellenőrizzük, hogy pontosan 6 adatbájt maradt-e
            if (message.Count != 6)
                return;

            try
            {
                // Az üzenet adatainak külön kezelése
                int x_low = message[0];      // X alsó 8 bit
                int x_high = message[1];     // X felső 8 bit
                int y_low = message[2];      // Y alsó 8 bit
                int y_high = message[3];     // Y felső 8 bit
                int pszi_low = message[4];   // Pszi alsó 8 bit
                int pszi_high = message[5];  // Pszi felső 8 bit

                // 16 bites értékek összeállítása kis-endian módon
                pos_x = (x_high << 8) | x_low; // High byte balra tolása és Low byte hozzáadása
                pos_y = (y_high << 8) | y_low;
                orientation = (pszi_high << 8) | pszi_low;

                // UI frissítése
                UpdateLabel();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Hiba az üzenet feldolgozásakor: {ex.Message}");
            }
        }

        private void UpdateLabel()
        {
            if (lbl_pos.InvokeRequired)
            {
                lbl_pos.Invoke((MethodInvoker)delegate {
                    lbl_pos.Text = $"Position: X = {pos_x}, Y = {pos_y}, Orientation = {orientation}";
                });
            }
            else
            {
                lbl_pos.Text = $"Position: X = {pos_x}, Y = {pos_y}, Orientation = {orientation}";
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (serialPort != null)
            {
                try
                {
                    if (serialPort.IsOpen)
                    {
                        serialPort.DataReceived -= SerialPort_DataReceived;
                        serialPort.Close();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Hiba a soros port lezárásakor: {ex.Message}");
                }
            }

            base.OnFormClosing(e);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            // A jármű helyzetének frissítése a pozíció adatokkal
            car.Location = new Point(Convert.ToInt32(pos_x), Convert.ToInt32(pos_y));
        }
    }
}
