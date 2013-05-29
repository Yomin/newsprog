/*
 * Copyright (c) 2013 Martin Rödel aka Yomin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

struct response
{
    char bla;
    struct ether_addr mac;
    struct in_addr ip, dns, gateway;
    char ipname[17];
    short com, console;
} __attribute__((packed));

void usage(char *name)
{
    printf("Usage: %s [-i <ip>] [-d <dns>] [-g <gateway>] [-c <comport>] [-p <consoleport>] [-n <ipname>] [-m <mac>] ip\n", name);
}

int isnumeric(char *str)
{
    return strspn(str, "1234567890") == strlen(str);
}

int main(int argc, char** argv)
{
    struct sockaddr_in dest;
    struct in_addr nip, dns, gateway;
    int com, console;
    char ipname[17];
    struct ether_addr mac;
    char set = 0, sets[7];
    
    memset(sets, 0, 7);
    
    int opt;
    while((opt = getopt(argc, argv, "i:d:g:c:p:n:m:")) != -1)
    {
        switch(opt)
        {
            case 'i':
                if(!inet_aton(optarg, &nip))
                {
                    printf("ip malformed\n");
                    return 1;
                }
                sets[0] = 1;
                break;
            case 'd':
                if(!inet_aton(optarg, &dns))
                {
                    printf("dns ip malformed\n");
                    return 1;
                }
                sets[1] = 1;
                break;
            case 'g':
                if(!inet_aton(optarg, &gateway))
                {
                    printf("gateway ip malformed\n");
                    return 1;
                }
                sets[2] = 1;
                break;
            case 'c':
                if(!isnumeric(optarg))
                {
                    printf("com port not numeric\n");
                    return 1;
                }
                com = atoi(optarg);
                if(com < 0 || com > 0xffff)
                {
                    printf("com port not in range 0-65535\n");
                    return 1;
                }
                sets[3] = 1;
                break;
            case 'p':
                if(!isnumeric(optarg))
                {
                    printf("console port not numeric\n");
                    return 1;
                }
                console = atoi(optarg);
                if(console < 0 || console > 0xffff)
                {
                    printf("console port not in range 0-65535\n");
                    return 1;
                }
                sets[4] = 1;
                break;
            case 'n':
                if(strlen(optarg) > 17)
                {
                    printf("only 17 characters for ip name allowed\n");
                    return 1;
                }
                strncpy(ipname, optarg, 17);
                sets[5] = 1;
                break;
            case 'm':
            {
                struct ether_addr *e = ether_aton(optarg);
                if(!e)
                {
                    printf("mac malformed\n");
                    return 1;
                }
                memcpy(&mac, e, sizeof(struct ether_addr));
                sets[6] = 1;
                break;
            }
            default:
                usage(argv[0]);
                return 1;
        }
        set = 1;
    }
    
    if(optind == argc)
    {
        printf("ip missing\n");
        return 2;
    }
    
    if(!inet_aton(argv[optind], &dest.sin_addr))
    {
        printf("ip malformed\n");
        return 1;
    }
    
    if(optind+1 < argc)
        printf("Superfluous argument O.o\n");
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
    {
        perror("Failed to create socket");
        return 2;
    }
    
    char buf[] = "www.wideled.com LED Controler Internet Discover v1.0";
    dest.sin_family = AF_INET;
    dest.sin_port = htons(41000);
    if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr*)&dest, sizeof(struct sockaddr_in)) == -1)
    {
        perror("Failed to send discover msg");
        return 3;
    }
    
    socklen_t len = sizeof(struct sockaddr_in);
    struct response res;
    
    if(recvfrom(sock, &res, sizeof(struct response), 0, (struct sockaddr*)&dest, &len) == -1)
    {
        perror("Failed to receive discover");
        return 4;
    }
    
    printf("mac:     %s\n", ether_ntoa(&res.mac));
    printf("ip:      %s\n", inet_ntoa(res.ip));
    printf("dns:     %s\n", inet_ntoa(res.dns));
    printf("gateway: %s\n", inet_ntoa(res.gateway));
    printf("com:     %hu\n", res.com);
    printf("console: %hu\n", res.console);
    printf("ipname:  %.17s\n", res.ipname);
    
    close(sock);
    
    if(set)
    {
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Failed to create socket");
            return 5;
        }
        
        dest.sin_port = htons(res.console);
        if(connect(sock, (struct sockaddr*)&dest, sizeof(struct sockaddr_in)) == -1)
        {
            perror("Failed to connect");
            return 6;
        }
        
        if(sets[0])
            memcpy(&res.ip, &nip, sizeof(struct in_addr));
        if(sets[1])
            memcpy(&res.dns, &dns, sizeof(struct in_addr));
        if(sets[2])
            memcpy(&res.gateway, &gateway, sizeof(struct in_addr));
        if(sets[3])
            res.com = com;
        if(sets[4])
            res.console = console;
        if(sets[5])
            strncpy(res.ipname, ipname, 17);
        if(sets[6])
            memcpy(&res.mac, &mac, sizeof(struct ether_addr));
        
        if(send(sock, &res, sizeof(struct response), 0) == -1)
        {
            perror("Failed to send configure");
            return 7;
        }
        
        char buf[2];
        if(recv(sock, buf, 2, 0) == -1)
        {
            perror("Failed to receive configure response");
            return 8;
        }
        
        printf("done\n");
        
        close(sock);
    }
    
    return 0;
}
