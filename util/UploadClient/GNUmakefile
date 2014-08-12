###############################################################################
# Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
#
# All rights reserved.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any
# damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must
#    not claim that you wrote the original software. If you use this
#    software in a product, an acknowledgment in the product
#    documentation would be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must
#    not be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.
#
# $Id: GNUmakefile 2602 2007-10-26 13:50:43Z svn $
###############################################################################

###############################################################################
# 基本設定
JAR_NAME        := UploadClient.jar

JAVA_PACKAGE	:= org/dyndns/acapulco

CLASS_PATH      := .:commons-codec.jar:commons-httpclient.jar:commons-logging.jar:junit.jar
JAVAC_FLAGS     := -Xlint:deprecation -encoding UTF-8

###############################################################################
# 依存関係
$(JAR_NAME): UploadClient.class
	jar cmf UploadClient.mf $@ $(JAVA_PACKAGE)/*.class
	jar -i $@
	jarsigner -storetype pkcs12 -keystore certificate.p12 $@ \
	"thawte freemail member's thawte consulting (pty) ltd. id"

UploadClient.class: UploadFrame.class UploadController.class HTTPUploadController.class

###############################################################################
# 掃除
clean:
	rm -rf $(JAR_NAME)
	rm -rf $(JAVA_PACKAGE)/*.class

###############################################################################
# 型ルール
%.class: %.java
	javac $(JAVAC_FLAGS) -classpath $(CLASS_PATH) $<

vpath %.java $(JAVA_PACKAGE)
vpath %.class $(JAVA_PACKAGE)

###############################################################################
# その他
.SUFFIXES: .jar .java .class
.PHONY: all compile clean

# Local Variables:
# mode: makefile
# coding: utf-8-unix
# End:
