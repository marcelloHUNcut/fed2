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

        
        int pos_x = 0;
        int pos_y = 0;
        int orientation = 0;

        private const byte StartByte = 0xF0; // Start byte
        private const byte EndByte = 0xFF;   // End byte

        private List<byte> buffer = new List<byte>();


        public Form1()
        {
            InitializeComponent();

            // Label inicializálása
            lbl_pos = new Label();
            lbl_pos.Location = new Point(1500, 10);
            lbl_pos.Size = new Size(500, 30);
            lbl_pos.Text = "Waiting for data...";
            this.Controls.Add(lbl_pos);

            // SerialPort inicializálása
            serialPort = new SerialPort("COM11", 9600);
            serialPort.DataBits = 8;
            serialPort.Parity = Parity.None;
            serialPort.StopBits = StopBits.One;
            serialPort.Handshake = Handshake.None;
            serialPort.DtrEnable = false;
            serialPort.RtsEnable = false;

            
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
                        buffer.Clear();
                        buffer.Add(b);
                    }

                    else if (buffer.Count > 0) // Csak akkor gyűjtsük, ha egy üzenet már elkezdődött
                    {
                        buffer.Add(b);

                        // Ha a buffer elérte a 8 bájt hosszúságot, ellenőrizzük az üzenetet
                        if (buffer.Count == 8)
                        {
                            // Ellenőrizzük a záró byte-ot
                            if (buffer[^1] == EndByte)
                            {
                                ProcessMessage(buffer); // Üzenet feldolgozása
                            }
                            // Mindig ürítsük a buffert, ha elérte a 8 bájtot
                            buffer.Clear();
                        }
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
            // Ellenőrizzük, hogy az üzenet formátuma helyes-e (minimum 8 byte és helyes Start/End byte)
            if (message.Count != 8 || message[0] != StartByte || message[^1] != EndByte) return;

            try
            {
                // Adatbájtok beolvasása (start és end byte figyelmen kívül hagyása)
                byte x_low = message[1];
                byte x_high = message[2];
                byte y_low = message[3];
                byte y_high = message[4];
                byte pszi_low = message[5];
                byte pszi_high = message[6];

                // Mukodo
                pos_x = (short)(((int)x_high << 8) | (int)x_low);
                pos_y = (short)(((int)y_high << 8) | (int)y_low);
                orientation = (short)(((int)pszi_high << 8) | (int)pszi_low);

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
            double ScaleFactor = 1;
            double centerX = this.ClientSize.Width / 2;
            double centerY = this.ClientSize.Height / 2;

            double screen_x = (centerX + pos_x / ScaleFactor); // Középponthoz viszonyítva
            double screen_y = (centerY + pos_y / ScaleFactor);

            // Képernyőhatárok figyelembevétele (opcionális)
            //screen_x = Math.Max(0, Math.Min(this.ClientSize.Width - pictureBox.Width, screen_x));
            //screen_y = Math.Max(0, Math.Min(this.ClientSize.Height - pictureBox.Height, screen_y));

            car.Location = new Point((int)screen_x, (int)screen_y);


            //car.Location = new Point(pos_x, pos_y);
        }
    }
}
