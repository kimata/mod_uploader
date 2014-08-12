#!/usr/bin/env zsh
###############################################################################
# Debian/Ubuntu をセットアップするスクリプト
###############################################################################
# 準備
# # vi /etc/hostname
# $HOST
# # vi /etc/dhcp3/dhclient.conf
# send host-name "$HOST";
# # apt-get install sudo
# # visudo
# root    ALL=(ALL)       ALL
# %sudo   ALL=(ALL)       NOPASSWD: ALL
# # adduser $USER
# # gpasswd -a kimata sudo
###############################################################################

unsetopt FUNCTION_ARGZERO

run() {
    echo "$0: running \`$@'"
    $@ || exit -1
}

host=$1

# ログイン用の鍵の転送
run ssh-copy-id -i ~/.ssh/vmware.id_rsa $host

# Subversion アクセス用の鍵の転送
run ssh $host "rm -f .ssh/svn.id_rsa"
run scp ~/.ssh/svn.id_rsa $host:.ssh/
run ssh $host "rm -f .ssh/svn.id_rsa.pub"
run scp ~/.ssh/svn.id_rsa.pub $host:.ssh/
run scp =(echo "Host columbia";
          echo "Port 443";
          echo "IdentityFile ~/.ssh/svn.id_rsa") $host:.ssh/config

# ホスト名の設定
run scp =(echo "127.0.0.1 localhost") $host:/tmp/hosts
run ssh $host "sudo mv /tmp/hosts /etc"

# 基本ツールのインストール
run ssh $host "sudo apt-get -y update"
run ssh $host "sudo apt-get -y install zsh"
if [ $host = debian ]; then
    run ssh $host "sudo apt-get -y install emacs21-nox"
else
    run ssh $host "sudo apt-get -y install emacs22-nox"
fi
run ssh $host "sudo apt-get -y install subversion"

# Z Shell の環境を整える
run ssh $host "sudo chsh -s /bin/zsh ${USER}"
run scp =(echo "PROMPT='%{$fg[$(load_color)]%}%B$USER@%m%#%b '";
          echo "autoload -U compinit";
          echo "compinit -u";
          echo "HISTFILE=$HOME/.zsh_history";
          echo "HISTSIZE=1000000";
          echo "SAVEHIST=1000000";
          echo "setopt share_history") $host:.zshrc
run scp =(echo "export LANG=C";
          echo "export PERL_BADLANG=0";
          echo "export DEBFULLNAME='Tetsuya Kimata'";
          echo "export DEBEMAIL='kimata@acapulco.dyndns.org'";
          echo "export PATH=$PATH:$HOME/bin") $host:.zshenv
run ssh $host "sudo chsh -s /bin/zsh root"
run ssh $host "sudo ln -s /home/$USER/.zshrc /root"
run ssh $host "sudo ln -s /home/$USER/.zshenv /root"

# keychain をインストール
run ssh $host "mkdir -p bin"
run scp /usr/bin/keychain $host:bin/

# known_hosts の追加
run ssh $host "ssh-keyscan -p 443 -t rsa columbia > .ssh/known_hosts"
run ssh $host "ssh-keyscan -H -p 443 -t rsa columbia >> .ssh/known_hosts"

# mod_uploader に必要なパッケージのインストール
run ssh $host "sudo apt-get -y install g++"
run ssh $host "sudo apt-get -y install make"
run ssh $host "sudo apt-get -y install libtool"
run ssh $host "sudo apt-get -y install apache2-mpm-prefork"
run ssh $host "sudo apt-get -y install apache2-prefork-dev"
run ssh $host "sudo apt-get -y install nkf"
run ssh $host "sudo apt-get -y install libwww-mechanize-perl"
if [ $host = debian ]; then
    run ssh $host "sudo apt-get -y install lsb-release"

    # deb のビルドに必要なパッケージのインストール
    run ssh $host "sudo apt-get -y install libmagick++9-dev"
    run ssh $host "sudo apt-get -y install autoconf"
    run ssh $host "sudo apt-get -y install automake"
    run ssh $host "sudo apt-get -y install debhelper"
    run ssh $host "sudo apt-get -y install devscripts"
    run ssh $host "sudo apt-get -y install fakeroot"
    run ssh $host "sudo apt-get -y install lintian"
fi

# リポジトリからチェックアウト
run ssh $host "bin/keychain ~/.ssh/*.id_rsa"
run ssh $host "mkdir -p prog"
run ssh $host "source ~/.keychain/$host*-sh; cd prog; svn co svn+ssh://svn@columbia/var/svn/repos/prog/apache Apache"
