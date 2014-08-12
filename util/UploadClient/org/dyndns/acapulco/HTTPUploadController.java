/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: HTTPUploadController.java 2611 2007-10-27 17:23:55Z svn $
 *****************************************************************************/

package org.dyndns.acapulco;

import java.io.File;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.multipart.FilePart;
import org.apache.commons.httpclient.methods.multipart.MultipartRequestEntity;
import org.apache.commons.httpclient.methods.multipart.Part;
import org.apache.commons.httpclient.methods.multipart.StringPart;

class HTTPUploadController implements UploadController
{
    private UploadFrame frame;
    private static final int TIMEOUT_SEC = 30;
    private static final String CHAR_CODE = "EUC-JP";
    private static final String ERROR_BEGIN = "<span id=\"error_message\">";
    private static final String ERROR_END = "</span>";

    public void startUpload() {
        upload();
    }

    public void setFrame(UploadFrame aFrame) {
        frame = aFrame;
    }

    private void upload() {
        UploadConfig config = frame.getConfig();

        PostMethod post = new PostMethod(config.getURL().concat("upload/"));
        try {
            Part[] parts = {
                new FilePart("file", new File(config.getFilePath()),
                             "application/octet-stream", CHAR_CODE),
                new StringPart("comment", config.getComment(), CHAR_CODE),
                new StringPart("download_pass", config.getDownloadPass(), CHAR_CODE),
                new StringPart("remove_pass", config.getRemovePass(), CHAR_CODE),
                new StringPart("code_pat", "京", CHAR_CODE),
            };
            post.getParams().setContentCharset(CHAR_CODE);
            post.setRequestEntity(new MultipartRequestEntity(parts, post.getParams()));

            HttpClient client = new HttpClient();
            client.getHttpConnectionManager().getParams().setConnectionTimeout(TIMEOUT_SEC * 1000);

            if (client.executeMethod(post) == HttpStatus.SC_OK) {
                String response;
                int errorBeginPos;
                int errorEndPos;

                response = post.getResponseBodyAsString();
                errorBeginPos = response.indexOf(ERROR_BEGIN);
                if (errorBeginPos != -1) {
                    errorBeginPos += ERROR_BEGIN.length();
                    errorEndPos = response.indexOf(ERROR_END, errorBeginPos);

                    frame.showMessage("エラーが発生しました．\n" +
                                      response.substring(errorBeginPos, errorEndPos));
                } else {
                    frame.showMessage("アップロードが完了しました．");
                }
            } else {
                frame.showMessage("アップロードに失敗しました．");
            }
        } catch (Exception ex) {
            frame.showMessage("アップロード中に例外が発生しました．");
            ex.printStackTrace();
        } finally {
            post.releaseConnection();
        }
    }

}

// Local Variables:
// mode: java
// coding: utf-8-dos
// End:
