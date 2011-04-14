# -*- coding: utf-8 -*-
'''
Copyright (c) 2011, Yota Ichino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
'''


'''Google Chromeの拡張機能のパッケージを作成

-パッケージの作成方法
> python make_extensio.py
できたディレクトリをGoogle Chromeに読み込ませる。
'''
import os
import shutil

version = '0.0.3'
out_dir = './NC4GC/'  # コピー先のディレクトリ


# とりあえずコピー先のディレクトリを消去し、再度mkdir
if os.path.exists(out_dir):
    shutil.rmtree(out_dir)

if not os.path.exists(out_dir):
    os.mkdir(out_dir)

# ファイルのコピー
file_list = [
    'Release/NiconamaClientPlugin.dll',
    'background.html',
    'popup.html',
    'CommentViewer.js',
    'icon.png',
]
dir_list = [
    'lib',
    'css'
]
for f in file_list:
    print(out_dir + f)
    shutil.copy(f, out_dir)

for d in dir_list:
    print(out_dir + d)
    shutil.copytree(d, out_dir + d)

# マニフェストファイルの作成
manifestfile = '''
{
  "name": "コメントビュワー for Google Chrome",
  "description": "ニコニコ生放送のコメントビュワー",
  "version": "%s",
  "background_page": "background.html",
  "permissions": [
    "http://watch.live.nicovideo.jp/",
    "http://live.nicovideo.jp/watch/",
    "http://www.nicovideo.jp/user/"
  ],
  "icons": {
    "48": "icon.png"
  },
  "browser_action": {
    "default_icon": "icon.png"
  },
  "plugins": [
    {"path": "NiconamaClientPlugin.dll"}
  ],
  "update_url": "https://github.com/downloads/nus/CommentViewerForGoogleChrome/updates.xml"
}
''' % version
with open(out_dir + 'manifest.json', 'w') as f:
    print(f.name)
    f.write(manifestfile)

# アップデート通知ファイル
updates_xml = '''<?xml version='1.0' encoding='UTF-8'?>
<gupdate xmlns='http://www.google.com/update2/response' protocol='2.0'>
  <app appid='gmngaokcabjmeocdnkenaipbdnbpbclg'>
    <updatecheck codebase='https://github.com/downloads/nus/CommentViewerForGoogleChrome/NC4GC.crx' version='%s' />
  </app>
</gupdate>
''' % version
with open('updates.xml', 'w') as f:
    print(f.name)
    f.write(updates_xml)

# Chromeによる拡張機能のパッケージを生成
os.system("make_package.cmd")
