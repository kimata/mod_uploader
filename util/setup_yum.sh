#!/usr/bin/env zsh
###############################################################################
# CentOS/Fedora をセットアップするスクリプト
###############################################################################
# 準備
# # vi /etc/sysconfig/network-scripts/ifcfg-eth0
# DHCP_HOSTNAME=$HOST
# # yum install sudo
# # visudo
# root    ALL=(ALL)       ALL
# %wheel  ALL=(ALL)       NOPASSWD: ALL
# # adduser $USER
# # passwd $USER
# # gpasswd -a kimata wheel
###############################################################################

unsetopt FUNCTION_ARGZERO

run() {
    echo "$0: running \`$@'"
    $@ || exit -1
}

host=$1

# ログイン用の鍵の転送
run ssh-copy-id -i ~/.ssh/vmware.id_rsa ${host}

# Subversion アクセス用の鍵の転送
run ssh ${host} "rm -f .ssh/svn.id_rsa"
run scp ~/.ssh/svn.id_rsa ${host}:.ssh/
run ssh ${host} "rm -f .ssh/svn.id_rsa.pub"
run scp ~/.ssh/svn.id_rsa.pub ${host}:.ssh/
run scp =(echo "Host columbia";
          echo "Port 443";
          echo "IdentityFile ~/.ssh/home.id_rsa") ${host}:.ssh/config

# ホスト名の設定
run scp =(echo "127.0.0.1 localhost") $host:/tmp/hosts
run ssh $host "sudo mv /tmp/hosts /etc"

# 基本ツールのインストール
run scp =(echo "127.0.0.1 localhost") ${host}:/etc/hosts
run ssh ${host} "sudo yum -y update"
run ssh ${host} "sudo yum -y install yum-fastestmirror"
run ssh ${host} "sudo yum -y install zsh"
run ssh ${host} "sudo yum -y install emacs-nox"
run ssh ${host} "sudo yum -y install subversion"

# Z Shell の環境を整える
run ssh ${host} "sudo chsh -s /bin/zsh $USER"
run scp =(echo "PROMPT='%{$fg[$(load_color)]%}%B$USER@%m%#%b '";
          echo "autoload -U compinit";
          echo "compinit -u";
          echo "HISTFILE=$HOME/.zsh_history";
          echo "HISTSIZE=1000000";
          echo "SAVEHIST=1000000";
          echo "setopt share_history") ${host}:.zshrc
run scp =(echo "export LANG=C";
          echo "export PATH=$PATH:$HOME/bin") ${host}:.zshenv
run ssh ${host} "sudo chsh -s /bin/zsh root"
run ssh ${host} "sudo ln -s /home/$USER/.zshrc /root"
run ssh ${host} "sudo ln -s /home/$USER/.zshenv /root"

# keychain をインストール
run ssh ${host} "mkdir -p bin"
run scp /usr/bin/keychain ${host}:bin/

# known_hosts の追加
run ssh ${host} "ssh-keyscan -p 443 -t rsa columbia > .ssh/known_hosts"
run ssh ${host} "ssh-keyscan -H -p 443 -t rsa columbia >> .ssh/known_hosts"

# mod_uploader に必要なパッケージのインストール
run ssh ${host} "sudo yum -y install gcc-c++"
run ssh ${host} "sudo yum -y install make"
run ssh ${host} "sudo yum -y install libtool"
run ssh ${host} "sudo yum -y install httpd-devel"
run ssh ${host} "sudo yum -y install nkf"
if [ ${host} = fedora ]; then
    run ssh ${host} "sudo yum -y install perl-WWW-Mechanize"
    run ssh ${host} "sudo yum -y install perl-Test-Simple"
fi
if [ ${host} = centos ]; then
    # rpm のビルドに必要なパッケージのインストール
    run ssh $host "sudo yum -y install ImageMagick-c++-devel"
    run ssh $host "sudo yum -y install rpm-build"
fi

run ssh ${host} "sudo yum -y install redhat-lsb"

# リポジトリからチェックアウト
run ssh ${host} "bin/keychain ~/.ssh/*.id_rsa"
run ssh ${host} "mkdir -p prog"
run ssh ${host} "source ~/.keychain/${host}*-sh; cd prog; svn co svn+ssh://svn@columbia/var/svn/repos/prog/apache Apache"
