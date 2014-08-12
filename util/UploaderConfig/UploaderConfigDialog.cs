using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Text.RegularExpressions;
using Microsoft.Win32;
using System.Diagnostics;

namespace UploaderConfig
{
    public partial class UploaderConfigDialog: Form
    {
        private UploaderConfig uploaderConfig;

        public UploaderConfigDialog()
        {
            InitializeComponent();
            setValidator();

            uploaderConfig =  new UploaderConfig(Directory.GetCurrentDirectory(),
                getImagemagickDir());
            uploaderConfigBindingSource.DataSource = uploaderConfig;

            apachePathTextBox.Text = getApacheDir();

            FormBorderStyle = FormBorderStyle.FixedDialog;
        }

        private static string getApacheDir()
        {
            string apacheDir;

            if ((apacheDir = getApache22Dir()) != "") {
                return apacheDir;
            } else {
                return getApache20Dir();
            }
        }

        private static string getApache22Dir()
        {
            return getApacheDirImpl("Software\\Apache Software Foundation\\Apache");
        }

        private static string getApache20Dir()
        {
            return getApacheDirImpl("Software\\Apache Group\\Apache");
        }

        private static string getApacheDirImpl(string keyName)
        {
            string valueName = "ServerRoot";

            string dir = "";

            try {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyName);
                dir = (string)(key.OpenSubKey(key.GetSubKeyNames()[0]).GetValue(valueName));
                key.Close();
            } catch (Exception) {
                // 無視
            }

            return dir;
        }

        private static string getImagemagickDir()
        {
            string keyName = "Software\\ImageMagick\\Current";
            string valueName = "LibPath";

            string dir = "";

            try {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyName);
                dir = (string)key.GetValue(valueName);
                key.Close();
            } catch (Exception) {
                // 無視
            }

            return dir;
        }

        private void setValidator()
        {
            dataDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(dataDirPathLabel, errorProvider)).validate);
            fileDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(fileDirPathLabel, errorProvider)).validate);
            thumbDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(thumbDirPathLabel, errorProvider)).validate);
            tempDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(tempDirPathLabel, errorProvider)).validate);

            imageDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(imageDirPathLabel, errorProvider)).validate);
            stylesheetDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(stylesheetDirPathLabel, errorProvider)).validate);
            javascriptDirPathTextBox.Validating += 
                new CancelEventHandler((new FolderPathValidator(javascriptDirPathLabel, errorProvider)).validate);

            indexTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(indexTmplPathLabel, errorProvider)).validate);
            infoTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(infoTmplPathLabel, errorProvider)).validate);

            progressTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(progressTmplPathLabel, errorProvider)).validate);
            downloadTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(downloadTmplPathLabel, errorProvider)).validate);
            thumbnailTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(thumbnailTmplPathLabel, errorProvider)).validate);
            adminTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(adminTmplPathLabel, errorProvider)).validate);
            errorTmplPathTextBox.Validating +=
                new CancelEventHandler((new FilePathValidator(errorTmplPathLabel, errorProvider)).validate);
        }

        private void modulePathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseModuleFile(modulePathLabel,
                modulePathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.modulePath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void apachePathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(apachePathLabel,
                apachePathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            apachePathTextBox.Text = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void imagemagickPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(imagemagickPathLabel, 
                imagemagickPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.imagemagickPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void indexTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(indexTmplPathLabel,
                indexTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.indexTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void infoTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(infoTmplPathLabel,
                infoTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.downloadTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void progressTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(progressTmplPathLabel,
                progressTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.downloadTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void downloadTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(downloadTmplPathLabel,
                downloadTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.progressTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void thumbnailTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(thumbnailTmplPathLabel,
                thumbnailTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.thumbnailTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void adminTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(adminTmplPathLabel,
                adminTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.adminTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void errorTmplPathButton_Click(object sender, EventArgs e)
        {
            string file_path = chooseTemplateFile(errorTmplPathLabel,
                errorTmplPathTextBox.Text);

            if (file_path == null) {
                return;
            }

            uploaderConfig.errorTmplPath = file_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void dataDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(dataDirPathLabel, 
                dataDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.dataDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void fileDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(fileDirPathLabel, 
                fileDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.fileDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void thumbDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(thumbDirPathLabel,
                thumbDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.thumbDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void tempDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(tempDirPathLabel, 
                tempDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.tempDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void imageDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(imageDirPathLabel,
                imageDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.imageDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void stylesheetDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(stylesheetDirPathLabel,
                stylesheetDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.stylesheetDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void javascriptDirPathButton_Click(object sender, EventArgs e)
        {
            string folder_path = chooseFolder(javascriptDirPathLabel, 
                javascriptDirPathTextBox.Text);

            if (folder_path == null) {
                return;
            }

            uploaderConfig.javascriptDirPath = folder_path;
            uploaderConfigBindingSource.ResetBindings(false);
            ValidateChildren();
        }

        private void copyButton_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(uploaderConfig.apacheConfigText);
        }

        private void apacheStripButton_Click(object sender, EventArgs e)
        {
            ProcessStartInfo psi = new ProcessStartInfo();

            psi.FileName = apachePathTextBox.Text;
            psi.Verb = "explore";

            Process.Start(psi);
        }

        private void helpStripButton_Click(object sender, EventArgs e)
        {
            Process.Start("http://acapulco.dyndns.org/mod_uploader/apache-win.htm");
        }

        private void copyToolStripButton_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(uploaderConfig.apacheConfigText);
        }

        private string chooseModuleFile(Label label, string currentFile)
        {
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.Title = label.Text + "を選択してください";
            dialog.CheckFileExists = true;
            dialog.FileName = currentFile;
            dialog.Filter = "モジュール(mod_uploader.so)|mod_uploader*.so";
            dialog.FilterIndex = 0;

            if (dialog.ShowDialog() != DialogResult.OK) {
                return null;
            }

            return dialog.FileName;
        }

        private string chooseFolder(Label label, string currentFolder)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();

            dialog.Description = label.Text + "を選択してください";
            dialog.SelectedPath = currentFolder;

            if (dialog.ShowDialog() != DialogResult.OK) {
                return null;
            }

            return dialog.SelectedPath;
        }

        private string chooseTemplateFile(Label label, string currentFile)
        {
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.Title = label.Text + "を選択してください";
            dialog.CheckFileExists = true;
            dialog.FileName = currentFile;
            dialog.Filter = "テンプレートファイル(*.htm)|*.htm";
            dialog.FilterIndex = 0;

            if (dialog.ShowDialog() != DialogResult.OK) {
                return null;
            }

            return dialog.FileName;
        }

        private void modulePathTextBox_Validating(object sender, CancelEventArgs e)
        {
            string path = (sender as TextBox).Text;

            if (!File.Exists(path)) {
                errorProvider.SetError(sender as TextBox,
                    "「" + modulePathLabel.Text + "」が正しくありません");
                return;
            }

            errorProvider.SetError(sender as TextBox, null);
        }

        private void apachePathTextBox_Validating(object sender, CancelEventArgs e)
        {
            string path = (sender as TextBox).Text;

            if ((!File.Exists(Path.Combine(Path.Combine(path, "bin"), "httpd.exe"))) &&
                (!File.Exists(Path.Combine(Path.Combine(path, "bin"), "Apache.exe")))) {
                errorProvider.SetError(sender as TextBox,
                    "「" + apachePathLabel.Text + "」が正しくありません");
                return;
            }
            apacheSetIconvPath(path);

            errorProvider.SetError(sender as TextBox, null);
        }

        private void apacheSetIconvPath(string apacheDir)
        {
            string envKey = "System\\CurrentControlSet\\Control\\Session Manager\\Environment";
            RegistryKey key = Registry.LocalMachine.CreateSubKey(envKey);

            key.SetValue("APR_ICONV_PATH", Path.Combine(Path.Combine(apacheDir, "bin"), "iconv"));
            key.Close();
        }

        private void imagemagickPathTextBox_Validating(object sender, CancelEventArgs e)
        {
            string path = (sender as TextBox).Text;

            if (!File.Exists(Path.Combine(path, "CORE_RL_Magick++_.dll"))) {
                errorProvider.SetError(sender as TextBox,
                    "「" + imageDirPathLabel.Text + "」が正しくありません");
                return;
            }

            errorProvider.SetError(sender as TextBox, null);
        }

        private void baseUrlLabel_Validating(object sender, CancelEventArgs e)
        {
            string url = (sender as TextBox).Text;

            Regex regex = new System.Text.RegularExpressions.
                Regex("^(https?://(?:[\\w.-]+\\.)*(?:[\\w.-]+)(/(?:(?:[\\w.-])+/)*(?:(?:[\\w.-])+)))/?$");

            Match match = regex.Match(url);
            if (!match.Success) {
                errorProvider.SetError(sender as TextBox,
                    "「" + baseUrlLabel.Text + "」が正しくありません");
                return;
            }

            ((TextBox)sender).Text = match.Groups[1].ToString();
            uploaderConfig.path = match.Groups[2].ToString();

            errorProvider.SetError(sender as TextBox, null);
        }

        class FolderPathValidator
        {
            private readonly Label label_;
            private readonly ErrorProvider errorProvider_;

            public FolderPathValidator(Label label, ErrorProvider errorProvider)
            {
                label_ = label;
                errorProvider_ = errorProvider;
            }

            public void validate(object sender, CancelEventArgs e)
            {
                string path = (sender as TextBox).Text;

                if (!Directory.Exists(path)) {
                    errorProvider_.SetError(sender as TextBox,
                        "「" + label_.Text + "」で指定されたフォルダは存在しません");
                    return;
                }

                errorProvider_.SetError(sender as TextBox, null);
            }
        }

        class FilePathValidator
        {
            private readonly Label label_;
            private readonly ErrorProvider errorProvider_;

            public FilePathValidator(Label label, ErrorProvider errorProvider)
            {
                label_ = label;
                errorProvider_ = errorProvider;
            }

            public void validate(object sender, CancelEventArgs e)
            {
                string path = (sender as TextBox).Text;

                if (!File.Exists(path)) {
                    errorProvider_.SetError(sender as TextBox,
                        "「" + label_.Text + "」で指定されたファイルは存在しません");
                    return;
                }

                errorProvider_.SetError(sender as TextBox, null);
            }
        }

        private void UploaderConfigDialog_Load(object sender, EventArgs e)
        {
            ValidateChildren();
        }

        private void textBox_KeyUp(object sender, KeyEventArgs e)
        {
            ValidateChildren();
        }

        private void configTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            ValidateChildren();
        }
    }
}
