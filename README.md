
# revolter-firefox

> 对 SNI RST 说不！

> 如果网站支持 ESNI，则使用 ESNI，否则让 firefox 不发送 SNI 信息以绕过 SNI RST

> 修改 Firefox (依赖库 nss) 的源代码, 使其在发送 TLS 握手 ClientHello 消息时不发送 SNI 扩展，拔除 SNI！

[![构建状态](https://dev.azure.com/xmader/apps/_apis/build/status/revolter-firefox)](https://dev.azure.com/xmader/apps/_build/latest?definitionId=21) 

[![总下载数](https://img.shields.io/github/downloads/Xmader/revolter-firefox/total.svg)](https://github.com/Xmader/revolter-firefox/releases)

## 下载

[Releases](https://github.com/Xmader/revolter-firefox/releases)

## 安装使用

1. 下载安装 firefox 68esr  
https://www.mozilla.org/en-US/firefox/organizations/all/  
https://archive.mozilla.org/pub/firefox/releases/68.0esr/  
（正常版 firefox 68 也可以使用）

2. 启用 DNS over HTTPS (DoH)   
在 firefox 中打开 `about:preferences`，打开 Network Settings (网络设置) - Settings (设置) 窗口，勾选 Enable DNS over HTTPS (启用 DNS over HTTPS)   

3. 启用 ESNI 支持  
在 firefox 中打开 `about:config`，搜索 `network.security.esni.enabled`，将值变为 `true`

4. 重启浏览器
如果无法访问任何网站，请尝试更换 DoH 服务器

5. 测试 DoH 和 ESNI 已经启用  
打开 https://www.cloudflare.com/ssl/encrypted-sni/ , 点击 `Check My Browser` ，会看到测试全部通过

6. [下载](https://github.com/Xmader/revolter-firefox/releases) nss3.dll 文件 (Windows) 或 libssl3.so 文件 (Linux)，覆盖到 firefox 的安装目录中  
记得备份原始文件

## 备注

* 无法解决网站的 IP 地址被屏蔽的问题
* 可以解决 DNS 污染和 SNI RST 的问题
* 有些网站依赖于 SNI 扩展 (TLS 握手过程中在 ClientHello 中发送的扩展协议)，可能会造成原本可以访问的网站变得无法访问
* 可以安装 firefox 到不同目录中，在访问网站时使用不同的浏览器安装，以应对上述情况
* 仅支持 64位 Linux 和 Windows 操作系统 (amd64) ，如果需要在 32位 操作系统或不同构架上使用，请自行构建

## 测试可用网站

> 墙内可访问网站不在此列表中, 也有可能墙内可访问网站在安装后无法访问

> 仅通过 DNS 污染屏蔽的网站不在此列表中，DoH 可以绕过 DNS 污染

> 证实被屏蔽 IP 的网站不在此列表中, 也不可能在此列表中 (例如 Google 和 Facebook)

* Wikipedia (维基百科)
* reddit
* pixiv
* Medium
* 中国数字时代 chinadigitaltimes.net
* Steam: steamcommunity.com (Steam 社区、创意工坊)
* Amazon: amazon.co.jp (日本亚马逊) 、amazon.com (美国亚马逊)
* 所有使用 cloudflare 的网站
* *.github.io (使用 Github Pages 的网站)

欢迎补充

## 自行构建

以在 Linux (Ubuntu) 上构建为例

### 构建 Linux 版

```sh
echo "
ac_add_options MOZ_PGO=1
ac_add_options --disable-av1
" > mozconfig

chmod +x ./revolter-firefox-nss-build.sh
chmod +x ./mach

./mach bootstrap --application-choice=browser --no-interactive
./revolter-firefox-nss-build.sh
```

### 构建 Windows 版

```sh
# install wine
sudo dpkg --add-architecture i386
curl https://dl.winehq.org/wine-builds/winehq.key | sudo apt-key add
sudo apt-add-repository 'deb https://dl.winehq.org/wine-builds/ubuntu/ xenial main'
sudo add-apt-repository ppa:wine/wine-builds -y 
sudo apt update
sudo apt install -y winehq-stable

# download build tools
export TOOLTOOL_DIR=$HOME/.revolter-firefox-build
mkdir -p $TOOLTOOL_DIR
cd $TOOLTOOL_DIR
wget \
    https://index.taskcluster.net/v1/task/gecko.cache.level-3.toolchains.v3.linux64-clang-8-mingw-x64.latest/artifacts/public/build/clangmingw.tar.xz \
    https://index.taskcluster.net/v1/task/gecko.cache.level-3.toolchains.v3.mingw32-rust-1.36.latest/artifacts/public/build/rustc.tar.xz \
    https://index.taskcluster.net/v1/task/gecko.cache.level-3.toolchains.v3.linux64-cbindgen.latest/artifacts/public/build/cbindgen.tar.xz \
    https://index.taskcluster.net/v1/task/gecko.cache.level-3.toolchains.v2.linux64-mingw-fxc2-x86.latest/artifacts/public/build/fxc2.tar.xz
ls *.tar.xz | xargs -n1 tar -xJf
cd -

sudo mkdir -p /builds/worker/workspace/build/src/
cd /builds/worker/workspace/build/src/
wget https://index.taskcluster.net/v1/task/gecko.cache.level-3.toolchains.v3.linux64-mingw32-nsis.latest/artifacts/public/build/nsis.tar.xz
tar -xJf nsis.tar.xz
cd -

# mozconfig
echo "
ac_add_options MOZ_PGO=1
ac_add_options --disable-av1
" > mozconfig
cat mozconfig-win >> mozconfig

chmod +x ./revolter-firefox-nss-build.sh
chmod +x ./mach

./mach bootstrap --application-choice=browser --no-interactive
./revolter-firefox-nss-build.sh
```

## License

MPL-2.0
