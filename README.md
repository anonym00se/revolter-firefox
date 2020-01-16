
# revolter-firefox

**项目的原作者 [Xmader](https://github.com/Xmader/) 于前不久惨遭喝茶（据信），此为备份，欢迎各位有志之士和我一起重启项目开发**

**原项目消失和 v2ray 的作者失联几乎同时，怀疑是有计划地针对翻墙软件的作者进行**

> 对 SNI RST 说不！ 翻墙新方式！

> 如果网站支持 ESNI，则使用 ESNI，否则可以让 firefox 不发送 SNI 信息以绕过 SNI RST

> 修改 Firefox (依赖库 nss) 的源代码, 使其在发送 TLS 握手 ClientHello 消息时可以不发送 SNI 扩展，拔除 SNI！

> 支持通过[编辑配置文件](#关于配置文件)，对在每一个网站上的行为进行精细化配置 (去除、保留或替换 SNI 信息)

[![构建状态](https://api.travis-ci.org/revolter-firefox/revolter-firefox.svg)](https://travis-ci.org/revolter-firefox/revolter-firefox/) 

[![总下载数](https://img.shields.io/github/downloads/revolter-firefox/revolter-firefox/total.svg)](https://github.com/revolter-firefox/revolter-firefox/releases)

## 下载

[Releases](https://github.com/revolter-firefox/revolter-firefox/releases)

## 安装使用

1. 下载安装 firefox 68esr  
https://www.mozilla.org/en-US/firefox/organizations/all/  
https://archive.mozilla.org/pub/firefox/releases/68.0.1esr/  
（正常版 firefox 68 也可以使用）

2. 启用 DNS over HTTPS (DoH)   
在 firefox 中打开 `about:preferences`，打开 Network Settings (网络设置) - Settings (设置) 窗口，勾选 Enable DNS over HTTPS (启用 DNS over HTTPS)   

3. 启用 ESNI 支持  
在 firefox 中打开 `about:config`，搜索 `network.security.esni.enabled`，将值变为 `true`

4. 重启浏览器
如果无法访问任何网站，请尝试更换 DoH 服务器

5. 测试 DoH 和 ESNI 已经启用  
打开 https://www.cloudflare.com/ssl/encrypted-sni/ , 点击 `Check My Browser` ，会看到测试全部通过

6. [下载](https://github.com/revolter-firefox/revolter-firefox/releases) nss3.dll 文件 (Windows) 或 libssl3.so 文件 (Linux)，覆盖到 firefox 的安装目录中  
记得备份原始文件

## 备注

* 无法解决网站的 IP 地址被屏蔽的问题
* 可以解决 DNS 污染和 SNI RST 的问题
* 有些网站需要使用发送的 SNI 扩展 (TLS 握手过程中在 ClientHello 中发送的扩展协议) 返回 TLS 证书，可能会造成原本可以访问的网站变得无法访问，可以通过[编辑配置文件](#关于配置文件)解决
* 仅支持 64位 (amd64) Linux 和 Windows 操作系统 ，如果需要在 32位 操作系统或不同构架上使用，请自行构建

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
* cloudflare-ipfs.com
* Amazon: amazon.co.jp (日本亚马逊) 、amazon.com (美国亚马逊)
* 所有使用 cloudflare 的网站
* *.github.io (使用 Github Pages 的网站)

欢迎补充

## 关于配置文件

配置文件文件为 firefox 安装目录 (`firefox.exe` 可执行文件所在目录) 下的 `sni.config` 文件，请自行创建。

[默认规则](/security/nss/lib/ssl/sni-config.c#L179): 不发送任何网站的 SNI 信息  
（配置文件不会覆盖默认规则，如果需要，请添加一条优先级更高的对应规则）

在配置文件越下方的规则优先级越高 （默认规则的优先级最低）

当一条规则匹配到了网站的 hostname (主机名、域名) ，不会继续再匹配优先级更低的规则了

### 规则格式

```
<主机名>, <方法/行为>, <用作替换的 SNI 信息 (如果方法为 replace)>
```

* `主机名` 支持使用正则表达式 （仅支持[有限的子集](https://github.com/cesanta/slre/blob/master/docs/syntax.md)，外加 `\w` 和 `\W` [字符类别匹配](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/RegExp#character-classes)，不支持嵌套括号），写在两个 `/` 符号之间 ~~（确实很废，以后会优化的）~~ ~~（至少比 glob 强）~~

* `方法`: 
    * `drop` 去除 SNI 信息
    * `bypass` 保留 SNI 信息
    * `replace` 替换 SNI 信息 (需要在规则种加上加上用作替换的 SNI 信息)

任何不符合上述格式开始的行都会被视为注释，上述格式后加上空格可自由添加内容作为注释 ~~（十分宽松的配置文件语法）~~

### 示例

`sni.config`

```c
example.com,bypass  // 我是注释
我也是注释
www.google.com, drop, 1  // 逗号后可添加任意数量空格，如果方法不是 `replace`, 用作替换的 SNI 信息会被忽略

/\w+\.wikipedia\.org/, replace, en.wikipedia.org  // 匹配任意语言的维基百科页面，将其 SNI 信息替换为 `en.wikipedia.org` (并不会循环替换)

/.+\.blogspot\.com/, bypass
```

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

# install lastest llvm
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main"
sudo apt-get update
sudo apt-get install -y clang-9 llvm-9-dev
export CC=clang-9
export CXX=clang++-9

# mach bootstrap
sudo apt-get install -y autoconf2.13 build-essential nodejs python-dev python-pip python-setuptools unzip uuid zip
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
sudo apt install -y wine-stable-amd64 wine-stable winehq-stable

# download build tools
export TOOLTOOL_DIR=$HOME/.revolter-firefox-build
mkdir -p $TOOLTOOL_DIR
cd $TOOLTOOL_DIR
wget -nv \
    https://taskcluster-artifacts.net/E6GMKuBBTyi0SllWEAtxwg/0/public/build/clangmingw.tar.xz \
    https://taskcluster-artifacts.net/X2KEt8RQTZGpxxCx__ksXQ/0/public/build/rustc.tar.xz \
    https://taskcluster-artifacts.net/faq0ZDr8R3Ws89rRTVzjIg/0/public/build/cbindgen.tar.xz \
    https://taskcluster-artifacts.net/Ns7ruxQ9Sjquyyv1J3zpCA/0/public/build/fxc2.tar.xz 
ls *.tar.xz | xargs -n1 tar -xJf
cd -

sudo mkdir -p /builds/worker/workspace/build/src/
cd /builds/worker/workspace/build/src/
sudo wget -nv https://taskcluster-artifacts.net/beAXMRJZT1a7zg6MbA-y-w/0/public/build/nsis.tar.xz
sudo tar -xJf nsis.tar.xz
cd -

# mozconfig
echo "
ac_add_options MOZ_PGO=1
ac_add_options --disable-av1
" > mozconfig
cat mozconfig-win >> mozconfig

chmod +x ./revolter-firefox-nss-build.sh
chmod +x ./mach

# mach bootstrap
sudo apt-get install -y autoconf2.13 build-essential nodejs python-dev python-pip python-setuptools unzip uuid zip
./mach bootstrap --application-choice=browser --no-interactive

./revolter-firefox-nss-build.sh
```

## License

MPL-2.0
