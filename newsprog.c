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
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <poll.h>

#define SOH "\x01"
#define STX "\x02"
#define ETX "\x03"
#define EOT "\x04"
#define DEL "\x7F" // delimiter
#define FNT "\xFE" // font <x>
#define COL "\xFD" // color <x>

#define OFFSET_font  0x40
#define OFFSET_color 0x41
#define OFFSET_mode  0x41
#define OFFSET_speed 0x31
#define OFFSET_time  0x30
#define OFFSET_slot  0x30

#define TAB_SIZE(tab) (sizeof(tab)/sizeof(tab[0]))
#define TAB_ENTRY(str, tab) get_tab_entry(str, tab##Tab, TAB_SIZE(tab##Tab), OFFSET_##tab)
#define TAB_PRINT(tab) print_tab(#tab, tab##Tab, TAB_SIZE(tab##Tab))

#define DEFAULT_DEST  "192.168.39.215"
#define DEFAULT_PORT  "10001"
#define DEFAULT_FONT  "1516row_normal"
#define DEFAULT_COLOR "auto"
#define DEFAULT_MODE  "auto"
#define DEFAULT_SPEED "fastest"
#define DEFAULT_TIME  "0"
#define DEFAULT_SLOT  0

#define INTRO "NETZ39 Newsscanner Programmer v1 (c) yomin"

char *fontTab[] =
{
    "small",
    "fiverow_normal",
    "fiverow_bold",
    "fiverow_wide",
    "fiverow_wideshade",
    "sevenrownormal_normal",
    "sevenrownormal_bold",
    "sevenrownormal_wide",
    "sevenrownormal_wideshade",
    "sevenrownormal_colorshade",
    "sevenrowfancy_normal",
    "sevenrowfancy_bold",
    "sevenrowfancy_wide",
    "sevenrowfancy_wideshade",
    "sevenrowfancy_colorshade",
    "tenrow_normal",
    "tenrow_bold",
    "tenrow_wide",
    "tenrow_wideshade",
    "1516row_normal",
    "1516row_bold",
    "1516row_wide",
    "1516row_wideshade",
    "24row",
    "32row"
};

char *colorTab[] =
{
    "auto",
    "red",
    "green",
    "darkred",
    "darkgreen",
    "yellow",
    "orange",
    "lemon",
    "ocher",
    "gay1",
    "gay2",
    "gay3",
    "blue",
    "darkblue",
    "lightpink",
    "pink",
    "purple",
    "darkpurple",
    "lightblue",
    "cyan",
    "skyblue",
    "darkcyan",
    "white",
    "lightyellow",
    "brightpink",
    "skin",
    "lightblue",
    "lightgreen",
    "lightpurple",
    "gray",
    "gay4",
    "gay5"
};

char *modeTab[] =
{
    "auto",
    "flash",
    "hold",
    "interlock",
    "rolldown",
    "rollup",
    "rollin",
    "rollout",
    "rollleft",
    "rollright",
    "rotate",
    "slide",
    "snow",
    "sparkle",
    "spray",
    "starburst",
    "switch",
    "twinkle",
    "wipedown",
    "wipeup",
    "wipein",
    "wipeout",
    "wipeleft",
    "wiperight",
    "cyclecolor",
    "clock"
};

char *speedTab[] =
{
    "fastest",
    "faster",
    "normal",
    "slow",
    "slower"
};

char *timeTab[] =
{
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

void write_force(int fd, char *msg, int len, struct pollfd *pfds)
{
    int count;
    while(len>0)
    {
        if(poll(pfds, 1, -1) == -1)
        {
            perror("Failed to poll socket");
            exit(7);
        }
        if(pfds[0].revents != POLLOUT)
            continue;
        count = write(fd, msg, len);
        if(count == -1)
        {
            perror("Failed to write");
            exit(5);
        }
        len -= count;
    }
}

void usage(char *name)
{
    printf("Usage: %s [-d <destination>] [-p <port>] [-f <font>] [-c <color>] [-m <mode>] [-s speed] [-t stoptime] [-n <slot>] text\n", name);
}

void lower(char *str)
{
    while(*str)
        tolower(*(str++));
}

char get_tab_entry(char *str, char **tab, int tabsize, int offset)
{
    lower(str);
    int x;
    for(x=0; x<tabsize; x++)
        if(!strcmp(tab[x], str))
            return offset + x;
    return -1;
}

void print_tab(char *name, char **tab, int tabsize)
{
    printf("%ss:\n", name);
    int x = 0;
    while(x < tabsize)
        printf("\t%s\n", tab[x++]);
}

int isnumeric(char *str)
{
    return strspn(str, "1234567890") == strlen(str);
}

int main(int argc, char** argv)
{
    printf("%s\n", INTRO);
    
    if(argc == 1)
    {
        usage(argv[0]);
        return 0;
    }
    
    char *dest = DEFAULT_DEST;
    char *port = DEFAULT_PORT;
    char font = TAB_ENTRY(DEFAULT_FONT, font);
    char color = TAB_ENTRY(DEFAULT_COLOR, color);
    char mode = TAB_ENTRY(DEFAULT_MODE, mode);
    char speed = TAB_ENTRY(DEFAULT_SPEED, speed);
    char time = TAB_ENTRY(DEFAULT_TIME, time);
    char slot = OFFSET_slot + DEFAULT_SLOT;
    
    int opt;
    while((opt = getopt(argc, argv, "d:p:f:c:m:s:t:n:")) != -1)
    {
        switch(opt)
        {
            case 'd':
                dest = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'f':
                font = TAB_ENTRY(optarg, font);
                if(font == -1)
                {
                    printf("Font not found\n");
                    TAB_PRINT(font);
                    return 1;
                }
                break;
            case 'c':
                color = TAB_ENTRY(optarg, color);
                if(color == -1)
                {
                    printf("Color not found\n");
                    TAB_PRINT(color);
                    return 1;
                }
                break;
            case 'm':
                mode = TAB_ENTRY(optarg, mode);
                if(mode == -1)
                {
                    printf("Mode not found\n");
                    TAB_PRINT(mode);
                    return 1;
                }
                break;
            case 's':
                speed = TAB_ENTRY(optarg, speed);
                if(speed == -1)
                {
                    printf("Speed not found\n");
                    TAB_PRINT(speed);
                    return 1;
                }
                break;
            case 't':
                time = TAB_ENTRY(optarg, time);
                if(time == -1)
                {
                    printf("Time not found\n");
                    TAB_PRINT(time);
                    return 1;
                }
                break;
            case 'n':
                if(!isnumeric(optarg))
                {
                    printf("Slot number not numeric\n");
                    return 1;
                }
                slot = atoi(optarg);
                if(slot > 0xff)
                {
                    printf("Slot number out of range\n");
                    return 1;
                }
                slot += OFFSET_slot;
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    
    if(optind == argc)
    {
        printf("Text missing\n");
        return 2;
    }
    
    char *text = argv[optind];
    
    if(optind+1 < argc)
        printf("Superfluous argument O.o\n");
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    struct addrinfo hints, *res, *iter;
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int ret = getaddrinfo(dest, port, &hints, &res);
    
    if(ret != 0)
    {
        printf("Failed to resolve destination: %s\n", gai_strerror(ret));
        return 3;
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
        return 4;
    }
    
    freeaddrinfo(res);
    
    char *prefix = "\x00\x00\x00\x00\x00";
    // ~ char *header = SOH "\x46\x46\x30\x30";
    char *header = SOH "\x00\x00\x00\x00";
    char *config = STX "\x41";
    char *weekday = "\x37\x46";
    // ~ char *padding = "\x30\x30\x30\x30\x32\x33\x35\x39\xff\xff\xff\x33";
    char *padding = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    // ~ char *chksum = "\x30\x61\x39\x63";
    char *chksum = "\x00\x00\x00\x00";
    
    struct pollfd pfds[1];
    pfds[0].fd = sock;
    pfds[0].events = POLLOUT;
    
    write(sock, "0", 1); // wtf the first write gets losts :(
    
    write_force(sock, prefix, 5, pfds);
    write_force(sock, header, 5, pfds);
    write_force(sock, config, 2, pfds);
    write_force(sock, &slot, 1, pfds);
    write_force(sock, &mode, 1, pfds);
    write_force(sock, &speed, 1, pfds);
    write_force(sock, &time, 1, pfds);
    write_force(sock, weekday, 2, pfds);
    write_force(sock, padding, 12, pfds);
    write_force(sock, FNT, 1, pfds);
    write_force(sock, &font, 1, pfds);
    write_force(sock, COL, 1, pfds);
    write_force(sock, &color, 1, pfds);
    
    char *ptr1 = strchr(text, '\n');
    char *ptr2 = strchr(text, '\\');
    while(ptr1 || ptr2)
    {
        if(ptr1 && (!ptr2 || (ptr1 < ptr2)))
        {
            *ptr1 = 0;
            write_force(sock, text, ptr1-text, pfds);
            text = ptr1+1;
            write_force(sock, DEL, 1, pfds);
        }
        else 
        {
            if(ptr2[1] == 'n')
            {
                *ptr2 = 0;
                write_force(sock, text, ptr2-text, pfds);
                write_force(sock, DEL, 1, pfds);
            }
            else
                write_force(sock, text, ptr2-text+2, pfds);
            text = ptr2+2;
        }
        ptr1 = strchr(text, '\n');
        ptr2 = strchr(text, '\\');
    }
    write_force(sock, text, strlen(text), pfds);
    
    write_force(sock, ETX, 1, pfds);
    write_force(sock, chksum, 4, pfds);
    write_force(sock, EOT, 1, pfds);
    
    char buf;
    int count = 2;
    while(count)
    {
        ret = read(sock, &buf, 1);
        if(ret == -1)
        {
            perror("Failed to read answer");
            return 6;
        }
        count -= ret;
    }
    
    printf("done\n");
    
    close(sock);
    
    return 0;
}
