using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using HslCommunication;
using HslCommunication.Enthernet.Redis;
using Newtonsoft.Json;
namespace EasyImageShower
{
    public partial class FrmMain : Form
    {
        private RedisClient redisClient = new RedisClient("127.0.0.1", 6379, "");
        public class JmjMeasureResult
        {
            public int id;
            public double distance;
            public string image;
            public string camera_tag;
        }
        public FrmMain()
        {
            InitializeComponent();
        }

        private void BtnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void BtnFresh_Click(object sender, EventArgs e)
        {
            int load_num = 0;
            OperateResult<string> read = redisClient.ListRightPop("to_ccd");
            while (read.IsSuccess)
            {
                string value = read.Content;
                if (value.Length <= 0)
                {
                    MessageBox.Show("No image captured!");
                    read = redisClient.ListRightPop("to_ccd");
                    continue;
                }
                JmjMeasureResult jmj_result = JsonConvert.DeserializeObject<JmjMeasureResult>(value);
                foreach (Control ctrl in this.Controls)
                {
                    if (ctrl.GetType().ToString() == "System.Windows.Forms.PictureBox" && ctrl.Tag.ToString() == jmj_result.camera_tag)
                    {
                        ((PictureBox)ctrl).Load(jmj_result.image);
                        load_num++;
                    }
                }
                read = redisClient.ListRightPop("to_ccd");
            }
            MessageBox.Show(String.Format("Image redis list load finished, totally [{0:D}] pics  loaded!", load_num));
        }
    }
}
