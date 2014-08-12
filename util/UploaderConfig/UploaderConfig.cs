using System.Text;
using System.IO;

namespace UploaderConfig
{
    public class UploaderConfig
    {
        private string modulePath_;

        private string path_;
        private string baseUrl_;

        private string imagemagickPath_;

        private string dataDirPath_;
        private string fileDirPath_;
        private string thumbDirPath_;
        private string tempDirPath_;

        private ulong totalFileSizeLimit_;
        private uint totalFileNumberLimit_;
        private ulong fileSizeLimit_;
        private uint perPageItemNumber_;

        private string indexTmplPath_;
        private string infoTmplPath_;
        private string progressTmplPath_;
        private string downloadTmplPath_;
        private string thumbnailTmplPath_;
        private string adminTmplPath_;
        private string errorTmplPath_;

        private string imageDirPath_;
        private string stylesheetDirPath_;
        private string javascriptDirPath_;

        private string apacheConfigText_;

        public UploaderConfig(string baseDir, string imagemagickPathArg)
        {
            imagemagickPath = imagemagickPathArg;

            modulePath_ = Path.Combine(baseDir, "mod_uploader.so");

            baseUrl = "http://localhost/up";
            path    = "/up";

            dataDirPath = Path.Combine(baseDir, "data");
            fileDirPath = Path.Combine(baseDir, "file");
            thumbDirPath = Path.Combine(baseDir, "thumb");
            tempDirPath = Path.Combine(baseDir, "temp");

            imageDirPath      = Path.Combine(baseDir, "img");
            stylesheetDirPath = Path.Combine(baseDir, "css");
            javascriptDirPath = Path.Combine(baseDir, "js");

            totalFileSizeLimit = 10*1024;
            totalFileNumberLimit = 1000;
            fileSizeLimit     = 100;
            perPageItemNumber = 20;

            string tmplDirPath = Path.Combine(baseDir, "tmpl");

            indexTmplPath     = Path.Combine(tmplDirPath, "index.htm");
            infoTmplPath      = Path.Combine(tmplDirPath, "info.htm");
            progressTmplPath  = Path.Combine(tmplDirPath, "progress.htm");
            downloadTmplPath  = Path.Combine(tmplDirPath, "download.htm");
            thumbnailTmplPath = Path.Combine(tmplDirPath, "thumbnail.htm");
            adminTmplPath     = Path.Combine(tmplDirPath, "admin.htm");
            errorTmplPath     = Path.Combine(tmplDirPath, "error.htm");
        }

        public string modulePath
        {
            get { return modulePath_; }
            set {
                modulePath_ = value;
                updateConfigText();
            }
        }

        public string path
        {
            get { return path_; }
            set {
                path_ = value;
                updateConfigText();
            }
        }

        public string baseUrl
        {
            get { return baseUrl_; }
            set {
                baseUrl_ = value;
                updateConfigText();
            }
        }

        public string imagemagickPath
        {
            get { return imagemagickPath_; }
            set {
                imagemagickPath_ = value;
                updateConfigText();
            }
        }

        public string dataDirPath
        {
            get { return dataDirPath_; }
            set {
                dataDirPath_ = value;
                updateConfigText();
            }
        }
        public string fileDirPath
        {
            get { return fileDirPath_; }
            set {
                fileDirPath_ = value;
                updateConfigText(); 
            }
        }
        public string thumbDirPath
        {
            get { return thumbDirPath_; }
            set
            {
                thumbDirPath_ = value;
                updateConfigText();
            }
        }
        public string tempDirPath
        {
            get { return tempDirPath_; }
            set {
                tempDirPath_ = value;
                updateConfigText();
            }
        }

        public ulong totalFileSizeLimit
        {
            get { return totalFileSizeLimit_; }
            set
            {
                totalFileSizeLimit_ = value;
                updateConfigText();
            }
        }
        public uint totalFileNumberLimit
        {
            get { return totalFileNumberLimit_; }
            set
            {
                totalFileNumberLimit_ = value;
                updateConfigText();
            }
        }
        public ulong fileSizeLimit
        {
            get { return fileSizeLimit_; }
            set
            {
                fileSizeLimit_ = value;
                updateConfigText();
            }
        }
        public uint perPageItemNumber
        {
            get { return perPageItemNumber_; }
            set
            {
                perPageItemNumber_ = value;
                updateConfigText();
            }
        }

       public string indexTmplPath
        {
            get { return indexTmplPath_; }
            set {
                indexTmplPath_ = value;
                updateConfigText();
            }
        }
        public string infoTmplPath
        {
            get { return infoTmplPath_; }
            set
            {
                infoTmplPath_ = value;
                updateConfigText();
            }
        }
        public string progressTmplPath
        {
            get { return progressTmplPath_; }
            set
            {
                progressTmplPath_ = value;
                updateConfigText();
            }
        }
        public string downloadTmplPath
        {
            get { return downloadTmplPath_; }
            set {
                downloadTmplPath_ = value;
                updateConfigText();
            }
        }
        public string thumbnailTmplPath
        {
            get { return thumbnailTmplPath_; }
            set {
                thumbnailTmplPath_ = value;
                updateConfigText();
            }
        }
        public string adminTmplPath
        {
            get { return adminTmplPath_; }
            set
            {
                adminTmplPath_ = value;
                updateConfigText();
            }
        }
        public string errorTmplPath
        {
            get { return errorTmplPath_; }
            set
            {
                errorTmplPath_ = value;
                updateConfigText();
            }
        }

        public string imageDirPath
        {
            get { return imageDirPath_; }
            set {
                imageDirPath_ = value;
                updateConfigText();
            }
        }

        public string stylesheetDirPath
        {
            get { return stylesheetDirPath_; }
            set {
                stylesheetDirPath_ = value;
                updateConfigText();
            }
        }

        public string javascriptDirPath
        {
            get { return javascriptDirPath_; }
            set {
                javascriptDirPath_ = value;
                updateConfigText();
            }
        }

        public string apacheConfigText
        {
            get { return apacheConfigText_; }
        }

        private void updateConfigText()
        {
            apacheConfigText_ = ApacheConfigCreator.create(this);
        }
    }
}
