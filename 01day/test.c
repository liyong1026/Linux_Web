/* 手撕最简单的服务器 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;

int main(int argc, char* agrv[]){
    char* ip = agrv[1];
    int port = stoi(agrv[2]);
/* 创建套接字 */
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
/* 绑定套接字 */
    sockaddr_in  my_addr;
    my_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &my_addr.sin_addr.s_addr);
    my_addr.sin_port = htons(port);

    bind(lfd,(struct sockaddr*)&my_addr, sizeof(my_addr));
/* 监听套接字 */
    listen(lfd, 10);
    struct sockaddr_in cliaddr;
    socklen_t client_addrlen = sizeof(cliaddr);
 

/* 接受数据 */  
    int cfd = accept(lfd,(struct sockaddr*)&cliaddr, &client_addrlen );
    char ip1[16] = "";
    printf("新的客户端连接 IP = %s, PORT = %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip1, 16), ntohs(cliaddr.sin_port));
    
  /* 回射数据 */
    char buf[1024] = "";
    while(1){
        bzero(buf, sizeof(buf));
        int n = 0;
        n = read(cfd, buf, sizeof(buf));
        if(n == 0){
            cout<< "客户端关闭"<<endl;
            break;
        }else if(n == -1){
            cout<< "read error!!!"<< endl;
        }else{
            cout<<buf << endl;
            send(cfd, buf, 1024, 0);
        }
    }
    close(lfd);
    close(cfd);
    return 0;


}

