using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace QA
{
    public partial class QA : Form
    {
        public QA()
        {
            InitializeComponent();
            System.Windows.Forms.Control.CheckForIllegalCrossThreadCalls = false;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //这里需要添加引用Microsoft.VisualBasic的引用，提供操作计算机组件（如：音频，时钟，键盘文件系统等）的属性
            Microsoft.VisualBasic.Devices.Computer pc = new Microsoft.VisualBasic.Devices.Computer();
            //循环该计算机上所有串行端口的集合
            foreach (string s in pc.Ports.SerialPortNames)
            {
                //串口名称添加到cbxPortName下拉框上
                //一般计算机上是有COM1和COM2串口的，如果没有自己在cbxPortName下拉框里写COM1 和 COM2的字符串(如：this.cbxPortName.Items.Add("COM2"))
                this.cbxPortName.Items.Add(s);
            }
            //防止报错，万一计算机上没有串口，就不走这一步
            if (pc.Ports.SerialPortNames.Count > 0)
            {
                cbxPortName.SelectedIndex = 0;
            }
            cbxPortName.Text = "COM3";//串口号默认值
            cmbbaud.Text = "9600";//波特率默认值
            cmbParity.SelectedIndex = 2;
            cmbBits.SelectedIndex = 0;
            cmbStop.SelectedIndex = 0;
            System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
            serialPort1.DataReceived += new SerialDataReceivedEventHandler(serialport1_DataReceived);//添加事件处理程序
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.PortName = cbxPortName.Text;//设置串口号
                serialPort1.BaudRate = Convert.ToInt32(cmbbaud.Text, 10);//十进制数据转换，设置波特率
                //设置数据位
                serialPort1.DataBits = Convert.ToInt32(cmbBits.Text);
                //根据选择的数据，设置停止位
                //if (cmbStop.SelectedIndex == 0)
                //    Myport.StopBits = StopBits.None;
                if (cmbStop.SelectedIndex == 1)
                    serialPort1.StopBits = StopBits.One;
                if (cmbStop.SelectedIndex == 2)
                    serialPort1.StopBits = StopBits.OnePointFive;
                if (cmbStop.SelectedIndex == 3)
                    serialPort1.StopBits = StopBits.Two;

                //根据选择的数据，设置奇偶校验位
                if (cmbParity.SelectedIndex == 0)
                    serialPort1.Parity = Parity.Even;
                if (cmbParity.SelectedIndex == 1)
                    serialPort1.Parity = Parity.Mark;
                if (cmbParity.SelectedIndex == 2)
                    serialPort1.Parity = Parity.None;
                if (cmbParity.SelectedIndex == 3)
                    serialPort1.Parity = Parity.Odd;
                if (cmbParity.SelectedIndex == 4)
                    serialPort1.Parity = Parity.Space;


                serialPort1.Open();//打开串口
                if (serialPort1.IsOpen)
                {
                    MessageBox.Show("the port is opened!");
                }
                else
                {
                    MessageBox.Show("failure to open the port!");
                }
                btnOpen.Enabled = false;//打开串口按钮不可用
                btnClose.Enabled =true ;//关闭串口按钮可用
            }
            catch (Exception ex)
            {
                MessageBox.Show("failure to open the port!" + ex.ToString());
            }
        }



        private void button1_Click(object sender, EventArgs e)
        {
            //System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
            //byte[] sendData = null;
            byte[] Data = new byte[1];
            if (serialPort1.IsOpen)//判断串口是否打开，如果打开执行下一步操作
            {
                if (txtSend.Text != "")
                {

                    try
                    {
                        serialPort1.Encoding = Encoding.GetEncoding("Gb2312");
                        Byte[] writeBytes = serialPort1.Encoding.GetBytes(txtSend.Text);//发送端编码GB2312
                        serialPort1.Write(writeBytes, 0, writeBytes.Length);//发送数据
                        txtReceive.AppendText("$发送：" + txtSend.Text + "\n");//添加内容textBox文本框中依次向后显示
                        
                    }
                    catch
                    {
                        MessageBox.Show("串口数据写入错误", "错误");//出错提示
                        serialPort1.Close();
                        btnOpen.Enabled = true;//打开串口按钮可用
                        btnClose.Enabled = false;//关闭串口按钮不可用
                    }


                }
            }
        }
        

        /// DataReceived事件委托的方法
        ///
        ///
        ///
        public static byte[] StringToByte(string s)
        {
            return System.Text.Encoding.Default.GetBytes(s);
         }

        private void serialport1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            serialPort1.Encoding = Encoding.GetEncoding("Gb2312");
            Byte[] readBytes = new Byte[serialPort1.BytesToRead];
            serialPort1.Read(readBytes, 0, readBytes.Length);//接收数据
            String decodedString = System.Text.Encoding.Default.GetString(readBytes);//接收端编码
            txtReceive.AppendText("$接收："+decodedString+"\t\n");//添加内容textBox文本框中依次向后显示
        }

        private void cmbStop_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void txtReceive_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.Close();//关闭串口
                btnOpen.Enabled = true;//打开串口按钮可用
                btnClose.Enabled = false;//关闭串口按钮不可用
            }
            catch
            {
                MessageBox.Show("串口关闭错误", "错误");
            }
        }

        private void cmbParity_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void txtSend_TextChanged(object sender, EventArgs e)
        {
            if (txtSend.Text != "")
                btnwrite.Enabled = true;
            else
                btnwrite.Enabled = false;
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            txtSend.Text = "";
            txtReceive.AppendText("请发送： 读ADR地址开始的 N BYTE \r\n");
        }

        private void btnCloseAll_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void label6_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            byte[] Data = new byte[1];
            if (serialPort1.IsOpen)//判断串口是否打开，如果打开执行下一步操作
            {
                if (txtSend.Text != "")
                {

                    try
                    {
                        serialPort1.Encoding = Encoding.GetEncoding("Gb2312");
                        Byte[] writeBytes = serialPort1.Encoding.GetBytes(txtSend.Text+"\n\t");//发送端编码GB2312
                        serialPort1.Write(writeBytes, 0, writeBytes.Length);//发送数据
                        txtReceive.AppendText("$发送：" + txtSend.Text + "\n");//添加内容textBox文本框中依次向后显示

                    }
                    catch
                    {
                        MessageBox.Show("串口数据写入错误", "错误");//出错提示
                        serialPort1.Close();
                        btnOpen.Enabled = true;//打开串口按钮可用
                        btnClose.Enabled = false;//关闭串口按钮不可用
                    }


                }
            }
        }

        private void btnread_Click(object sender, EventArgs e)
        {
            byte[] Data = new byte[1];
            if (serialPort1.IsOpen)//判断串口是否打开，如果打开执行下一步操作
            {
                if (txtSend.Text != "")
                {

                    try
                    {
                        serialPort1.Encoding = Encoding.GetEncoding("Gb2312");
                        Byte[] writeBytes = serialPort1.Encoding.GetBytes(txtSend.Text);//发送端编码GB2312
                        serialPort1.Write(writeBytes, 0, writeBytes.Length);//发送数据
                        txtReceive.AppendText("$发送：" + txtSend.Text + "\n");//添加内容textBox文本框中依次向后显示

                    }
                    catch
                    {
                        MessageBox.Show("串口数据写入错误", "错误");//出错提示
                        serialPort1.Close();
                        btnOpen.Enabled = true;//打开串口按钮可用
                        btnClose.Enabled = false;//关闭串口按钮不可用
                    }


                }
            }
        }

        private void txtReceive_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
