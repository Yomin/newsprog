
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

int write_force(int fd, char* msg, int len)
{
    int count;
    while(len>0)
    {
        count = write(fd, msg, len);
        if(count == -1)
        {
            perror("Failed to write");
            return 3;
        }
        len -= count;
    }
    return 0;
}

int main(int argc, char** argv)
{
    char *ip = "192.168.39.215";
    char *port = "10001";
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    struct addrinfo hints, *res, *iter;
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int ret = getaddrinfo(ip, port, &hints, &res);
    
    if(ret != 0)
    {
        printf("Failed to resolve servername: %s\n", gai_strerror(ret));
        return 1;
    }
    
    for(iter = res; iter != 0; iter = iter->ai_next)
    {
        ret = connect(sock, iter->ai_addr, iter->ai_addrlen);
        if(!ret)
            break;
    }
    
    if(ret == -1)
    {
        perror("Failed to connect");
        freeaddrinfo(res);
        return 2;
    }
    
    freeaddrinfo(res);
    
    printf("Connection established\n");
    
    char *test = "\x00\x00\x00\x00\x00\x01\x46\x46\x30\x30\x02\x41\x30\x41\x32\x32\x37\x46\x30\x30\x30\x30\x32\x33\x35\x39\xff\xff\xff\x33\xfe\x45\xfd\x41\x74\x65\x73\x74\x03\x30\x61\x39\x63\x04";
    
    ret = write_force(sock, test, 44);
    
    char buf;
    int x;
    
    for(x=0; x<2; x++)
        read(sock, &buf, 1);
    
    printf("done\n");
    
    close(sock);
    
    return ret;
}
