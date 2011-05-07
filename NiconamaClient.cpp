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
     \brief �j�R���̃R�����g�T�[�o�ւ̐ڑ�
     \param addr �A�h���X
                 <addr> msgNNN.live.nicovideo.jp </addr>
     \param port �|�[�g�ԍ�
                 <port> �|�[�g�ԍ� </port>
     \retval �ڑ��ł�����true  �ڑ��ł��Ȃ�������false��ԋp
             �j�R���̃z�X�g�����܂�ł��Ȃ���false��ԋp
    */
    bool connect(const std::string &addr, const std::string &port)
    {
        bool ret = true;

        // �j�R����p�Ȃ̂ŁA�j�R���̃z�X�g�����܂�ł��Ȃ��Ɛڑ������Ȃ�
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
     \brief �R�����g�̗v��
     \param thread �R�����g�̃X���b�hID
     \param numOfComments ���߂Ɏ擾����R�����g�̌�
                   <thread> �R�����g�̃X���b�hID </thread>
    */
    void requestComment(const std::string &thread_elem, const std::string &numOfComments)
    {
        socket << "<thread thread=\"" << thread_elem << "\" version=\"20061206\" res_from=\"-" << numOfComments << "\"/>" << '\0';
        socket.flush();

		// boost::thread_group::create_thread�̎������Q�l�ɂ���
		com_thread = new boost::thread(boost::bind(&NiconamaClient::impl::getCommentsThread, this));
    }
    
	/*
     \brief �R�����g�̎擾
     \retval �R�����g (���[�U���Ȃǂ��܂�xml�`��)
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
     \brief �R�����g�T�[�o�Ƃ̐ؒf
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
	std::queue<std::string> comments_qu;	// �R�����g���i�[
	boost::mutex guard;	// �X���b�h��
	volatile bool end_flag;
	boost::thread *com_thread;

	void getCommentsThread()
	{
		while(1) {
			// �X���b�h���N�����Asocket�����o�͂��̊֐������炵���Ă΂�Ȃ��̂�mutex�͂���Ȃ�
			if(socket.fail()) {
				// comments_qu�͑��̃����o�֐�����Ă΂��̂�mutex���K�v
				boost::mutex::scoped_lock lk(guard);

				comments_qu.push("/failure");
				break;
			}

			std::string comment_xml;    // �擾�����R�����g��XML �߂�l
			getline(socket, comment_xml, '\0');
			// *****</chat>\0<chat thread=******
			// '\0'��1�R�����g�̋�؂肾����

			{
				boost::mutex::scoped_lock lk(guard);
				
				if(end_flag) {
					break;
				}
				
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
