@echo off

svn checkout http://nixysa.googlecode.com/svn/trunk/ nixysa-read-only
cd nixysa-read-only
git clone git://github.com/nus/CommentViewerForGoogleChrome.git CommentViewerForGoogleChrome
start CommentViewerForGoogleChrome\NiconamaClientPlugin.sln
echo 'リリースモードでビルドしてください'
pause