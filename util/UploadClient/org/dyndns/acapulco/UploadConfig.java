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
 * $Id: UploadConfig.java 2600 2007-10-24 14:36:03Z svn $
 *****************************************************************************/

package org.dyndns.acapulco;

class UploadConfig {
	private String url;
	private String filePath;
	private String comment;
	private String removePass;
	private String downloadPass;
	
	public UploadConfig(String aURL, String aFilePath, String aComment, 
						String aRemovePass, String aDownloadPass) {
		url = aURL;
		filePath = aFilePath;
		comment = aComment;
		removePass = aRemovePass;
		downloadPass = aDownloadPass;
	}
	
	public String getURL() {
		return url;
	}
		
	public String getFilePath() {
		return filePath;
	}
	
	public String getComment() {
		return comment;
	}

	public String getRemovePass() {
		return removePass;
	}

	public String getDownloadPass() {
		return downloadPass;
	}
}
