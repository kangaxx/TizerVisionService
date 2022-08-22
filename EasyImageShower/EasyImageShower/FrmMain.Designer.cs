
namespace EasyImageShower
{
    partial class FrmMain
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.BtnClose = new System.Windows.Forms.Button();
            this.BtnFresh = new System.Windows.Forms.Button();
            this.PicBox01 = new System.Windows.Forms.PictureBox();
            this.PicBox02 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PicBox01)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PicBox02)).BeginInit();
            this.SuspendLayout();
            // 
            // BtnClose
            // 
            this.BtnClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.BtnClose.Location = new System.Drawing.Point(1275, 526);
            this.BtnClose.Name = "BtnClose";
            this.BtnClose.Size = new System.Drawing.Size(114, 42);
            this.BtnClose.TabIndex = 0;
            this.BtnClose.Text = "关闭";
            this.BtnClose.UseVisualStyleBackColor = true;
            this.BtnClose.Click += new System.EventHandler(this.BtnClose_Click);
            // 
            // BtnFresh
            // 
            this.BtnFresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.BtnFresh.Location = new System.Drawing.Point(1160, 526);
            this.BtnFresh.Name = "BtnFresh";
            this.BtnFresh.Size = new System.Drawing.Size(109, 42);
            this.BtnFresh.TabIndex = 1;
            this.BtnFresh.Text = "刷新";
            this.BtnFresh.UseVisualStyleBackColor = true;
            this.BtnFresh.Click += new System.EventHandler(this.BtnFresh_Click);
            // 
            // PicBox01
            // 
            this.PicBox01.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.PicBox01.Location = new System.Drawing.Point(12, 12);
            this.PicBox01.Name = "PicBox01";
            this.PicBox01.Size = new System.Drawing.Size(668, 453);
            this.PicBox01.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.PicBox01.TabIndex = 2;
            this.PicBox01.TabStop = false;
            this.PicBox01.Tag = "0";
            // 
            // PicBox02
            // 
            this.PicBox02.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.PicBox02.Location = new System.Drawing.Point(705, 12);
            this.PicBox02.Name = "PicBox02";
            this.PicBox02.Size = new System.Drawing.Size(674, 453);
            this.PicBox02.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.PicBox02.TabIndex = 3;
            this.PicBox02.TabStop = false;
            this.PicBox02.Tag = "1";
            // 
            // FrmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1401, 580);
            this.Controls.Add(this.PicBox02);
            this.Controls.Add(this.PicBox01);
            this.Controls.Add(this.BtnFresh);
            this.Controls.Add(this.BtnClose);
            this.Name = "FrmMain";
            this.Text = "EasyImageShower ver. beta";
            ((System.ComponentModel.ISupportInitialize)(this.PicBox01)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PicBox02)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button BtnClose;
        private System.Windows.Forms.Button BtnFresh;
        private System.Windows.Forms.PictureBox PicBox01;
        private System.Windows.Forms.PictureBox PicBox02;
    }
}

