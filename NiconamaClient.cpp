/*
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
*/

#include "NiconamaClient.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <iostream>
#include <queue>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

class NiconamaClient::impl {
public:
    /*
     \brief ニコ生のコメントサーバへの接続
     \param addr アドレス
                 <addr> msgNNN.live.nicovideo.jp </addr>
     \param port ポート番号
                 <port> ポート番号 </port>
     \retval 接続できたらtrue  接続できなかったらfalseを返却
             ニコ生のホスト名を含んでいないとfalseを返却
    */
    bool connect(const std::string &addr, const std::string &port)
    {
        bool ret = true;

        // ニコ生専用なので、ニコ生のホスト名を含んでいないと接続させない
        if(!boost::contains(addr, "live.nicovideo.jp")) {
            ret = false;
            goto end;
        }
    
        socket.connect(addr, port);
    
        if(socket.fail()) {
            ret = false;
        }
    
    end:
        return ret;
    }


    /*
     \brief コメントの要請
     \param thread コメントのスレッドID
     \param numOfComments 初めに取得するコメントの個数
                   <thread> コメントのスレッドID </thread>
    */
    void requestComment(const std::string &thread_elem, const std::string &numOfComments)
    {
        socket << "<thread thread=\"" << thread_elem << "\" version=\"20061206\" res_from=\"-" << numOfComments << "\"/>" << '\0';
        socket.flush();

		// boost::thread_group::create_threadの実装を参考にした
		//std::auto_ptr<boost::thread> tmp_thr(new boost::thread(boost::bind(&NiconamaClient::impl::getCommentsThread, this)));
		com_thread = new boost::thread(boost::bind(&NiconamaClient::impl::getCommentsThread, this));
    }
    
	/*
     \brief コメントの取得
     \retval コメント (ユーザ情報などを含んだxml形式)
    */
	std::string getComment()
	{
		std::string ret;
		boost::mutex::scoped_lock lk(guard);
		
		if(comments_qu.empty()) {
			ret = "";
			goto end;
		}

		ret = comments_qu.front();
		comments_qu.pop();

	end:
		return ret;
	}
    
    /*
     \brief コメントサーバとの切断
    */
    void close()
    {
		boost::mutex::scoped_lock lk(guard);

		end_flag = true;
        socket.close();
    }

	impl() : end_flag(false), com_thread(NULL)
	{
	}

    ~impl()
    {
        close();
		if(com_thread) {
			com_thread->join();
			delete com_thread;
			com_thread = NULL;
		}
    }

private:
    boost::asio::ip::tcp::iostream socket;
	std::queue<std::string> comments_qu;	// コメントを格納
	boost::mutex guard;	// スレッドの
	volatile bool end_flag;
	boost::thread *com_thread;

	void getCommentsThread()
	{
		while(1) {
			// スレッドを起動中、socketメンバはこの関数内からしか呼ばれないのでmutexはいらない
			if(socket.fail()) {
				// comments_quは他のメンバ関数から呼ばれるのでmutexが必要
				boost::mutex::scoped_lock lk(guard);

				comments_qu.push("/failure");
				break;
			}

			std::string comment_xml;    // 取得したコメントのXML 戻り値
			getline(socket, comment_xml, '\0');
			// *****</chat>\0<chat thread=******
			// '\0'で1コメントの区切りだから

			{
				boost::mutex::scoped_lock lk(guard);
				
				if(end_flag) {
					break;
				}
				
				/*if(boost::contains(comment_xml, "premium=\"2\"") && boost::contains(comment_xml, ">/disconnect</chat>")) {
					socket.close();
					comments_qu.push("/disconnect");
					break;
				}*/

				comments_qu.push(comment_xml);
			}
		}
	}
};

NiconamaClient::NiconamaClient() : pImpl(new NiconamaClient::impl())
{
}

bool NiconamaClient::connect(const std::string &addr, const std::string &port)
{
    return pImpl->connect(addr, port);
}

void NiconamaClient::requestComment(const std::string &thread_elem, const std::string &numOfComments)
{
    pImpl->requestComment(thread_elem, numOfComments);
}

std::string NiconamaClient::getComment()
{
    return pImpl->getComment();
}

void NiconamaClient::close()
{
    pImpl->close();
}
