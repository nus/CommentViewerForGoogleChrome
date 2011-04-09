var nc;        // コメントを取得するオブジェクト
var tid;    // Timer
var addr;    // コメントサーバのアドレス
var port;    // コメントサーバのポート番号
var thread; // 放送コメントのスレッド番号
var initCommentsCount // 始めのコメントのカウント数
var commentsTableId = "comments_table";
var comments = [];  // コメントとその情報を格納
function initPlugin()
{
    var plugin = document.getElementById("plugin");
    nc = plugin.NiconamaClient();
    
    if(!nc) {
        alert("no plugin!");
    }
}

/*
 \brief 生放送のIDから必要な情報(addr, port, thread)を取得
 \param lvId 生放送のID(lv1234567)
*/
function getPlayerstatus(lvId)
{
    var res = getPlayerstatusXML(lvId);

    if(!res) {
        return ;
    }
    
    var dom = buildXMLDOM(res);
    
    var gp = dom.getElementsByTagName("getplayerstatus")[0];
    if(gp.getAttribute("status") == "fail") {
        return ;
    }
    
    initCommentsCount = dom.getElementsByTagName("comment_count")[0].textContent;
    initCommentsCount = (initCommentsCount < 500) ? initCommentsCount : "500";    // 一気に取得するコメント数に制限を掛ける
    var ms = dom.getElementsByTagName("ms")[0];
    addr = ms.getElementsByTagName("addr")[0].textContent;
    port = ms.getElementsByTagName("port")[0].textContent;
    thread = ms.getElementsByTagName("thread")[0].textContent;
}

function setCommentServerInfo()
{
    nc.connect(addr, port);
    nc.requestComment(thread, "500");
    
    Sleep(500); // ソケット通信のバッファに文字列を貯めるため
    for(var i = 0; i < (initCommentsCount - 1); i++) {
        addCommentInfo(commentsTableId);
    }
    tid = setInterval("timerInterval()", 30);
}

/*
 \brief 指定ミリ秒だけプロセスを停止
 \param ms ミリ秒
*/
function Sleep(ms) {
    var d1 = new Date().getTime();
    var d2 = new Date().getTime();
    
    while( d2 < (d1 + ms) ) {
        d2 = new Date().getTime();
    }
}

function timerInterval()
{
    addCommentInfo(commentsTableId);
}

/*
 \brief コメントをtableタグに追加
 \param tableのID
*/
function addCommentInfo(id)
{
    var comment = nc.getComment();
    
    // コメントが空だと、もうこれ以上コメントが来ないので、コメント欄更新を停止
    if(comment == "/failure") {
        close();
    }
    
    if(comment == "") {
        return ;
    }
    
    var comDom = transCommentInfo(comment);
    if(!comDom) {
        return ;
    }
    
    if((comDom.comment == "/disconnect") && (comDom.premium == "3")) {
        close();
    }
    
    $("#" + id).dataTable().fnAddData([
        comDom.no ? comDom.no : "",   // 公式生放送の場合だと番号がない
        commentFilter(comDom.comment),
        getNickNameByUserId(comDom.user_id),
        comDom.premium
    ]);
    
    comments.push({
        no: comDom.no,
        comment: comDom.comment,
        user_id: comDom.user_id,
        premium: comDom.premium
    });
}

function close()
{
    clearInterval(tid);
    nc.close();
}

function getPlayerstatusXML(lvId)
{
    return request("http://watch.live.nicovideo.jp/api/getplayerstatus?v=" + lvId);
}

/*
 \brief 指定されてURLのドキュメントをGETで取得しテキストで返却
 \param url ドキュメントのURL
 \retval テキスト
*/
function request(url)
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.send(null);
    return xhr.responseText;
}

/*
 \brief chatXmlから必要な情報を返却
 \param chatXml xmlタグ
 例
 "<chat thread=​"1079536575" no=​"1" vpos=​"2300" date=​"1301065780" user_id=​"17831427">​コメント</chat>​"
 返却値
 {
 "thread":​"1079536575",
 "no":"1",
 "vpos":"2300",
 "date":"1301065780",
 "user_id":​"17831427",
 "comment":"コメント"
 }
 \retval chatタグでなければ  を返却
*/
function transCommentInfo(chatXml)
{
    var ret = Object();
    var dom = buildXMLDOM(chatXml);
    var chat = dom.getElementsByTagName("chat");
    
    if(chat.length == 0) {
        return ;
    }
    
    chat = chat[0];
    
    var needInfo = ["thread", "no", "vpos", "date", "user_id", "premium"];
    for(var i = 0; i < needInfo.length; i++) {
        var ni = needInfo[i];
        ret[ni] = chat.getAttribute(ni);
    }
    
    ret["comment"] = chat.textContent;
    
    return ret;
}

/*
 \brief XMLのテキストをDOMオブジェクトに変換
 \param text XMLのテキスト
 \retval DOMオブジェクト
*/
function buildXMLDOM(text)
{
    var parser = new DOMParser();
    var dom = parser.parseFromString(text, "text/xml");
    return dom;
}

/*
 \brief HTMLのテキストをDOMオブジェクトに変換
 \param text THMLのテキスト
 \retval DOMオブジェクト
*/
function buildHTMLDOM(text)
{
    var dummy = document.createElement("div");
    
    if(dummy) {
        dummy.innerHTML = text;
    }
    
    return dummy;
}

/*
 \bief コメントに書かれているXSSを除去
 \param commentText フィルタリング対象のコメント
 \retval フィルタリング済みコメント
*/
function commentFilter(commentText)
{
    var c = commentText;
    // XSS対策
    // scriptが含まれていたらタグをエスケープ
    if(c.indexOf("script") != -1) {
        c = c.replace(/</g, "&lt;");
        c = c.replace(/>/g, "&gt;");
    }
    
    c = c.replace(/\n/g, "<br>");
    
    return c;
}

/*
 \brief ユーザIDから匿名アカウントかどうか判定する
 \param userId 検査するユーザID
 \retval 匿名アカウントならtrueを返却
*/
function isAnonymousAccount(userId)
{
    // アルファベットを含んでいたら匿名アカウントと判断
    return (userId.search(/[a-zA-Z]/) != -1);
}

/*
 \brief ニックネームをユーザに尋ねる
 \userId ニックネームを付けられるユーザのID
 \retval ユーザが付けるニックネーム
*/
function hearNickNameFromUser(userId)
{
    var userInfoDOM = buildHTMLDOM(request("http://www.nicovideo.jp/user/" + userId));
    if(!userInfoDOM) {
        return false;
    }

    // デフォルトのユーザID
    var defaultId = userInfoDOM.getElementsByTagName("strong");

    // ユーザ名が取得できなかった場合
    if(defaultId.length == 0) {
        return window.prompt("ニックネームを変更できます", "");
    }
    
    return window.prompt("ニックネームを変更できます", defaultId[0].innerText ? defaultId[0].innerText : "");
}

/*
 \brief ユーザIDからニックネームを取得
*/
function getNickNameByUserId(userId)
{
    var nickName = localStorage[userId];
    
    if(!nickName) {
        return userId;
    }
    
    return nickName;
}

/*
 \brief ユーザIDからニックネームを付ける
*/
function setNickNameByUserId(userId, nickName)
{
    localStorage[userId] = nickName;
}

/*
 \brief ユーザID欄を更新
 \param userId 更新するユーザID
*/
function updateUserIdSection(userId, nickName)
{
    var d = oTable.fnSettings().aoData;
    for(var i = 0; i < d.length; i++) {
        if(d[i].nTr.cells[2].innerText == userId) {
            d[i].nTr.cells[2].innerHTML = nickName;
        }
    }
}

/*
 \biref コメント番号からユーザIDを返却
 \param コメント番号
 \retval ユーザID 無かったら空文字
*/
function getUserIdByNo(no)
{
    for(var i = 0; i < comments.length; i++) {
        if(comments[i].no == no) {
            return comments[i].user_id;
        }
    }
    
    return "";
}

