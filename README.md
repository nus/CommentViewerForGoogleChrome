コメントビュワー for Google Chrome
==================================

License
=======
The MIT License.
PLEASE READ LICENSE.txt

概要
====
Google Chromeで動くニコニコ生放送用のコメントビュワー

機能一覧
========
* コメントのフィルタリング
* コメントを表示する件数を制限
* ニックネームを付ける

ビルドに必要なもの
==================
OS:Windows
Visual C++ 2010
Python 2.6以上
Subversion
Git

ビルド方法
==========
ビルドに必要なNixysaをレポジトリからチェックアウトし、適当な場所に本ソースコードもチェックアウトする。
+ get_building_esseintial.cmdを実行
+ Visual C++ 2010を使いリリースモードでビルド

パッケージング
==============
Google Chromeでインストールできる用にパッケージ化を行う。
+ make_package.cmdを実行


