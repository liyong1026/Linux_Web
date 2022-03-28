#include <fcntl.h>
#include <sys/epoll.h>
#include "wrap.h"
#include <fcntl.h>
#include <iostream>
#define MAX_EVENT_NUMBER 1024
using namespace std;

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_optinon = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_optinon);
    return old_option;
}

void addfd(int epollfd, int fd, bool enable_et){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et){
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int main(int argc, char* argv[]){
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int lfd = tcp4bind(port, ip);

    Listen(lfd, 128);

    int epfd = epoll_create(1);
    
    struct epoll_event event, cliEvent[MAX_EVENT_NUMBER];
    event.events = EPOLLIN;
    event.events |= EPOLLET;
    event.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &event);

    while(1){
        int ret = epoll_wait(epfd, cliEvent, MAX_EVENT_NUMBER,-1);
        cout<< "epoll  wait"<<endl;
        if(ret < 0){
            perror("epollwait error!");
            break;
        }else if(ret == 0) {
            continue;
        }else{
            for(int i = 0; i< ret; i++){
                int sockfd = cliEvent[i].data.fd;
                if(sockfd == lfd && cliEvent[i].events & EPOLLIN){
                    struct sockaddr_in cliaddr;
                    char ip[16] = "";
                    socklen_t  len = sizeof(cliaddr);
                    int cfd =  Accept(lfd, (struct sockaddr*)&cliaddr, &len);
                    addfd(epfd,cfd, true);

                }else if(cliEvent[i].events & EPOLLIN){
                    cout<< "event trigger!"<<endl;
                    while(1){
                        char buf[4] = "";
                        //如果读一个缓冲区，缓冲区内没有数据，如果是阻塞的，将阻塞等待
                        //如果是非阻塞的，将返回-1 并且将error设为EAGAIN
                        int n = recv(sockfd, buf, sizeof(buf), 0);
                        if(n< 0){
                            if(errno == EAGAIN){
                                break;
                            }
                            perror("");
                            close(sockfd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &cliEvent[i]);
                            break;
                        }else if(n == 0){
                            cout<<"客户端关闭！！";
                            close(sockfd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &cliEvent[i]);
                            break;
                        }else{
                            write(STDOUT_FILENO, buf, 4);
                            send(sockfd, buf, n, 0);
                        }

                    }
                }
            }
        }
    }

}



