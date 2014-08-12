using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace UploaderConfig
{
    class ApacheConfigCreator
    {
        public static string create(UploaderConfig uploaderConfig)
        {
            StringBuilder buffer = new StringBuilder();

            buffer.AppendFormat("#### mod_uploader ###################################################\r\n");

            buffer.AppendFormat("LoadFile \"{0}\"\r\n",
                Path.Combine(uploaderConfig.imagemagickPath, "CORE_RL_Magick++_.dll"));
            buffer.AppendFormat("LoadModule uploader_module \"{0}\"\r\n",
                uploaderConfig.modulePath);

            buffer.AppendFormat("<Location {0}>\r\n",
                uploaderConfig.path);
            buffer.AppendFormat("    SetHandler                     uploader\r\n");
            buffer.AppendFormat("    UploaderBaseUrl                \"{0}\"\r\n",
                uploaderConfig.baseUrl);

            buffer.AppendFormat("    UploaderDataDirectory          \"{0}\"\r\n",
                uploaderConfig.dataDirPath);
            buffer.AppendFormat("    UploaderFileDirectory          \"{0}\"\r\n",
                uploaderConfig.fileDirPath);
            buffer.AppendFormat("    UploaderThumbDirectory         \"{0}\"\r\n",
                uploaderConfig.thumbDirPath);
            buffer.AppendFormat("    UploaderTempDirectory          \"{0}\"\r\n",
                uploaderConfig.tempDirPath);

            buffer.AppendFormat("    UploaderTotalFileSizeLimit     \"{0}\"\r\n",
                uploaderConfig.totalFileSizeLimit * 1024);
            buffer.AppendFormat("    UploaderTotalFileNumberLimit   \"{0}\"\r\n",
                uploaderConfig.totalFileNumberLimit);
            buffer.AppendFormat("    UploaderFileSizeLimit          \"{0}\"\r\n",
                uploaderConfig.fileSizeLimit * 1024);
            buffer.AppendFormat("    UploaderPerPageItemNumber      \"{0}\"\r\n",
                uploaderConfig.perPageItemNumber);

            buffer.AppendFormat("    UploaderIndexViewTemplate      \"{0}\"\r\n",
                uploaderConfig.indexTmplPath);
            buffer.AppendFormat("    UploaderInfoViewTemplate       \"{0}\"\r\n",
                uploaderConfig.infoTmplPath);
            buffer.AppendFormat("    UploaderProgressViewTemplate   \"{0}\"\r\n",
                uploaderConfig.progressTmplPath);
            buffer.AppendFormat("    UploaderDownloadViewTemplate   \"{0}\"\r\n",
                uploaderConfig.downloadTmplPath);
            buffer.AppendFormat("    UploaderThumbnailViewTemplate  \"{0}\"\r\n",
                uploaderConfig.thumbnailTmplPath);
            buffer.AppendFormat("    UploaderAdminViewTemplate      \"{0}\"\r\n",
                uploaderConfig.adminTmplPath);
            buffer.AppendFormat("    UploaderErrorViewTemplate      \"{0}\"\r\n",
                uploaderConfig.errorTmplPath);
            buffer.AppendFormat("</Location>\r\n");

            buffer.AppendFormat("<Location {0}/admin>\r\n",
                uploaderConfig.path);
            buffer.AppendFormat("    Order Deny,Allow\r\n");
            buffer.AppendFormat("    Deny From All\r\n");
            buffer.AppendFormat("    Allow From 127.0.0.1\r\n");
            buffer.AppendFormat("</Location>\r\n");

            appendAccess(buffer, uploaderConfig.imageDirPath);
            appendAccess(buffer, uploaderConfig.stylesheetDirPath);
            appendAccess(buffer, uploaderConfig.javascriptDirPath);

            buffer.AppendFormat("Alias /up_img                  \"{0}\"\r\n",
                uploaderConfig.imageDirPath);
            buffer.AppendFormat("Alias /up_css                  \"{0}\"\r\n",
                uploaderConfig.stylesheetDirPath);
            buffer.AppendFormat("Alias /up_js                   \"{0}\"\r\n",
                uploaderConfig.javascriptDirPath);

            buffer.AppendFormat("#####################################################################\r\n");

            return buffer.ToString();
        }

        private static void appendAccess(StringBuilder buffer, string path)
        {
            buffer.AppendFormat("<Directory \"{0}\">\r\n", path);
            buffer.AppendFormat("    Order Allow,Deny\r\n");
            buffer.AppendFormat("    Allow From All\r\n");
            buffer.AppendFormat("</Directory>\r\n");
        }
    }
}
