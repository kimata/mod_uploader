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
 * $Id: UploadController.as 2619 2007-11-02 16:25:07Z svn $
 *****************************************************************************/

package org.dyndns.acapulco
{
    import flash.events.*;
    import flash.utils.*;
    import flash.net.*;

    import mx.collections.*;
    import mx.controls.*;
    import mx.validators.*;
    import mx.rpc.events.AbstractEvent;

    public class UploadController {
        public var url:String;
        public var maxFileSize:Number;
        public var fileInput:TextInput;
        public var progressBar:ProgressBar;
        private var file:FileReference;
        private var isDuringUpload:Boolean;

        public function init(url:String, maxFileSize:Number): void
        {
            this.url = url;
            this.maxFileSize = maxFileSize;
            this.isDuringUpload = false;
        }

        public function browseFile(event:MouseEvent): void
        {
        	if (isDuringUpload) {
        		return;        	
        	}
            file = new FileReference();
            file.addEventListener(Event.SELECT, selectFile);
            file.browse();
        }

        public function startUpload(event:MouseEvent,
                                      comment:String, downloadPass:String, removePass:String,
                                      validators:Array): void
        {
        	if (isDuringUpload) {
        		return;        	
        	}
            if (!validatInput(validators)) {
                Alert.show("入力に不備があります．");
                return;
            }
            execUpload(comment, downloadPass, removePass);
        }

        private function validatInput(validators:Array): Boolean
        {
            var error:Array;

            error = Validator.validateAll(validators);

            return error.length == 0;
        }
        private function execUpload(comment:String, downloadPass:String, removePass:String): void
        {
            var request:URLRequest = new URLRequest(url + "upload/");
            var variables:URLVariables = new URLVariables();

            variables.comment = comment;
            variables.download_pass = downloadPass;
            variables.remove_pass = removePass;
            variables.code_pat= "京";

            request.method = URLRequestMethod.POST;
            request.data = variables;

            file.addEventListener(ProgressEvent.PROGRESS, uploadProgressHandler);
            file.addEventListener(DataEvent.UPLOAD_COMPLETE_DATA, uploadResponseHandler);
            file.addEventListener(IOErrorEvent.IO_ERROR, uploadIOErrorHandler);

            progressBar.label = "UPLOADING %3%%";
            progressBar.visible = true;

			isDuringUpload = true;

            file.upload(request, "file", false);
        }

        private function selectFile(event:Event): void
        {
            if (file.name.length > 64) {
                Alert.show("ファイルの名前が長すぎます．");
                return;
            }
            if (file.size > maxFileSize) {
                Alert.show("ファイルサイズが大きすぎます．");
                return;
            }
            fileInput.text = file.name;
        }

        private function uploadProgressHandler(event:ProgressEvent):void
        {
            progressBar.setProgress(event.bytesLoaded, event.bytesTotal);
        }

        private function uploadResponseHandler(event:DataEvent): void
        {
            const ERROR_BEGIN:String = "<span id=\"error_message\">";
            const ERROR_END:String = "</span>";

			isDuringUpload = false;
            progressBar.label = "FINISH.";

            var errorPos:int = event.text.search(ERROR_BEGIN);
            if (errorPos == -1) {
                Alert.show("アップロードが完了しました．");

                var timer:Timer = new Timer(2000, 1);
                timer.addEventListener("timer",
                                       function(event:TimerEvent): void
                                       {
                                           var request:URLRequest;

                                           request = new URLRequest(url);
                                           request.method = URLRequestMethod.POST;

                                           navigateToURL(request, "_self");
                                       });
                timer.start();
            } else {
                var message:String;

                message = event.text.substring(errorPos + ERROR_BEGIN.length);
                message = message.substring(0, message.search(ERROR_END));

                // できれば message も表示したいけど，EUC-JP のために文字化けしてしまう...
                Alert.show("エラーが発生しました．" + 
                           "連続して複数のファイルをアップロードする場合は間隔を開けて行ってください．");
            }
        }

        private function uploadIOErrorHandler(event:IOErrorEvent): void
        {
			isDuringUpload = false;
            progressBar.label = "ERROR.";
            Alert.show("I/O エラーが発生しました．");
        }
    }
}
