    /*
    * sniffex.c
    *
    * Sniffer example of TCP/IP packet capture using libpcap.
    *
    * Version 0.1.1 (2005-07-05)
    * Copyright (c) 2005 The Tcpdump Group
    *
    * This software is intended to be used as a practical example and
    * demonstration of the libpcap library; available at:
    * http://www.tcpdump.org/
    *
    ****************************************************************************
    *
    * ..... 字数太长了，此处版权信息就省了
    *
    ****************************************************************************
    *
    * Example compiler command-line for GCC:
    *   gcc -Wall -o sniffex sniffex.c -lpcap
    *
    ****************************************************************************
    *
    * Code Comments
    *
    * This section contains additional information and explanations regarding
    * comments in the source code. It serves as documentaion and rationale
    * for why the code is written as it is without hindering readability, as it
    * might if it were placed along with the actual code inline. References in
    * the code appear as footnote notation (e.g. [1]).
    *
    * 1. Ethernet headers are always exactly 14 bytes, so we define this
    * explicitly with "#define". Since some compilers might pad structures to a
    * multiple of 4 bytes - some versions of GCC for ARM may do this -
    * "sizeof (struct sniff_ethernet)" isn't used.
    *
    * 2. Check the link-layer type of the device that's being opened to make
    * sure it's Ethernet, since that's all we handle in this example. Other
    * link-layer types may have different length headers (see [1]).
    *
    * 3. This is the filter expression that tells libpcap which packets we're
    * interested in (i.e. which packets to capture). Since this source example
    * focuses on IP and TCP, we use the expression "ip", so we know we'll only
    * encounter IP packets. The capture filter syntax, along with some
    * examples, is documented in the tcpdump man page under "expression."
    * Below are a few simple examples:
    *
    * Expression                        Description
    * ----------                        -----------
    * ip                                        Capture all IP packets.
    * tcp                                        Capture only TCP packets.
    * tcp port 80                        Capture only TCP packets with a port equal to 80.
    * ip host 10.1.2.3                Capture all IP packets to or from host 10.1.2.3.
    *
    ****************************************************************************
    *
    */

    #define APP_NAME                "sniffex"
    #define APP_DESC                "Sniffer example using libpcap"
    #define APP_COPYRIGHT        "Copyright (c) 2005 The Tcpdump Group"
    #define APP_DISCLAIMER        "THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM."

    #include <pcap.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    /* 默认捕获长度 (每个包捕获的最大长度)
       default snap length (maximum bytes per packet to capture) */
    #define SNAP_LEN 1518

    /* 以太网头部14个字节
       ethernet headers are always exactly 14 bytes [1] */
    #define SIZE_ETHERNET 14

    /* 以太网地址6个字节
       Ethernet addresses are 6 bytes */
    #define ETHER_ADDR_LEN        6

    /* Ethernet header */
    struct sniff_ethernet {
            u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
            u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
            u_short ether_type;                     /* IP? ARP? RARP? etc */
    };

    /* IP header */
    struct sniff_ip {
            u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
            u_char  ip_tos;                 /* type of service */
            u_short ip_len;                 /* total length */
            u_short ip_id;                  /* identification */
            u_short ip_off;                 /* fragment offset field */
            #define IP_RF 0x8000            /* reserved fragment flag */
            #define IP_DF 0x4000            /* dont fragment flag */
            #define IP_MF 0x2000            /* more fragments flag */
            #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
            u_char  ip_ttl;                 /* time to live */
            u_char  ip_p;                   /* protocol */
            u_short ip_sum;                 /* checksum */
            struct  in_addr ip_src,ip_dst;  /* source and dest address */
    };
    #define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
    #define IP_V(ip)                (((ip)->ip_vhl) >> 4)

    /* TCP header */
    typedef u_int tcp_seq;

    struct sniff_tcp {
            u_short th_sport;               /* source port */
            u_short th_dport;               /* destination port */
            tcp_seq th_seq;                 /* sequence number */
            tcp_seq th_ack;                 /* acknowledgement number */
            u_char  th_offx2;               /* data offset, rsvd */
    #define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
            u_char  th_flags;
            #define TH_FIN  0x01
            #define TH_SYN  0x02
            #define TH_RST  0x04
            #define TH_PUSH 0x08
            #define TH_ACK  0x10
            #define TH_URG  0x20
            #define TH_ECE  0x40
            #define TH_CWR  0x80
            #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
            u_short th_win;                 /* window */
            u_short th_sum;                 /* checksum */
            u_short th_urp;                 /* urgent pointer */
    };

    void
    got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

    void
    print_payload(const u_char *payload, int len);

    void
    print_hex_ascii_line(const u_char *payload, int len, int offset);

    void
    print_app_banner(void);

    void
    print_app_usage(void);

    /*
    * app name/banner
    */
    void
    print_app_banner(void)
    {

            printf("%s - %s\n", APP_NAME, APP_DESC);
            printf("%s\n", APP_COPYRIGHT);
            printf("%s\n", APP_DISCLAIMER);
            printf("\n");

    return;
    }

    /*
    * print help text
    */
    void
    print_app_usage(void)
    {

            printf("Usage: %s [interface]\n", APP_NAME);
            printf("\n");
            printf("Options:\n");
            printf("    interface    Listen on <interface> for packets.\n");
            printf("\n");

    return;
    }

    /*
    * print data in rows of 16 bytes: offset   hex   ascii
    *
    * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
    */
    void
    print_hex_ascii_line(const u_char *payload, int len, int offset)
    {

            int i;
            int gap;
            const u_char *ch;

            /* offset */
            printf("%05d   ", offset);
           
            /* hex */
            ch = payload;
            for(i = 0; i < len; i++) {
                    printf("%02x ", *ch);
                    ch++;
                    /* print extra space after 8th byte for visual aid */
                    if (i == 7)
                            printf(" ");
            }
            /* print space to handle line less than 8 bytes */
            if (len < 8)
                    printf(" ");
           
            /* fill hex gap with spaces if not full line */
            if (len < 16) {
                    gap = 16 - len;
                    for (i = 0; i < gap; i++) {
                            printf("   ");
                    }
            }
            printf("   ");
           
            /* ascii (if printable) */
            ch = payload;
            for(i = 0; i < len; i++) {
                    if (isprint(*ch))
                            printf("%c", *ch);
                    else
                            printf(".");
                    ch++;
            }

            printf("\n");

    return;
    }

    /*
    * 打印包的有效载荷数据（避免打印二进制数据）
    * print packet payload data (avoid printing binary data)
    */
    void
    print_payload(const u_char *payload, int len)
    {

            int len_rem = len;
            int line_width = 16;                        /* 每行的字节数 | number of bytes per line */
            int line_len;
            int offset = 0;                                        /* 从0开始的偏移计数器 | zero-based offset counter */
            const u_char *ch = payload;

            if (len <= 0)
                    return;

            /* data fits on one line */
            if (len <= line_width) {
                    print_hex_ascii_line(ch, len, offset);
                    return;
            }

            /* 数据跨越多行 data spans multiple lines */
            for ( ;; ) {
                    /* 计算当前行的长度 | compute current line length */
                    line_len = line_width % len_rem;

                    /* 显示分割线 | print line */
                    print_hex_ascii_line(ch, line_len, offset);

                    /* 计算总剩余 | compute total remaining */
                    len_rem = len_rem - line_len;

                    /* 转移到打印的剩余字节的指针
                       shift pointer to remaining bytes to print */
                    ch = ch + line_len;

                    /* 添加偏移 | add offset */
                    offset = offset + line_width;

                    /* 检查是否有线宽字符或更少
                       check if we have line width chars or less */
                    if (len_rem <= line_width) {
                            /* print last line and get out */
                            print_hex_ascii_line(ch, len_rem, offset);
                            break;
                    }
            }

    return;
    }

    /*
    * 解析/显示 包
    * dissect/print packet
    */
    void
    got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
    {

            static int count = 1;                   /* 包计数器                | packet counter */
           
            /* declare pointers to packet headers */
            const struct sniff_ethernet *ethernet;  /* 以太网头部        | The ethernet header [1] */
            const struct sniff_ip *ip;              /* IP 头部                | The IP header */
            const struct sniff_tcp *tcp;            /* TCP 头部                | The TCP header */
            const char *payload;                    /* Packet payload */

            int size_ip;
            int size_tcp;
            int size_payload;
           
            /* 显示包总数 */
            printf("\nPacket number %d:\n", count);
            count++;
           
            /* 定义以太网头部
               define ethernet header */
            ethernet = (struct sniff_ethernet*)(packet);

			printf("   Src MAC: %X:%X:%X:%X:%X:%X\n",
				(ethernet->ether_shost)[0],
				(ethernet->ether_shost)[1],
				(ethernet->ether_shost)[2],
				(ethernet->ether_shost)[3],
				(ethernet->ether_shost)[4],
				(ethernet->ether_shost)[5]);
			printf("   Dst MAC: %X:%X:%X:%X:%X:%X\n",
				(ethernet->ether_dhost)[0],
				(ethernet->ether_dhost)[1],
				(ethernet->ether_dhost)[2],
				(ethernet->ether_dhost)[3],
				(ethernet->ether_dhost)[4],
				(ethernet->ether_dhost)[5]);
           
            /* 定义/计算 IP 头部偏移
               define/compute ip header offset */
            ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
            size_ip = IP_HL(ip)*4;
            if (size_ip < 20) {
                    printf("   * Invalid IP header length: %u bytes\n", size_ip);
                    return;
            }

            /* 显示源IP和目的IP
               print source and destination IP addresses */
            printf("       From: %s\n", inet_ntoa(ip->ip_src));
            printf("         To: %s\n", inet_ntoa(ip->ip_dst));
           
            /* 确定协议
               determine protocol */       
            switch(ip->ip_p) {
                    case IPPROTO_TCP:
                            printf("   Protocol: TCP\n");
                            break;
                    case IPPROTO_UDP:
                            printf("   Protocol: UDP\n");
                            return;
                    case IPPROTO_ICMP:
                            printf("   Protocol: ICMP\n");
                            return;
                    case IPPROTO_IP:
                            printf("   Protocol: IP\n");
                            return;
                    default:
                            printf("   Protocol: unknown\n");
                            return;
            }
           
            /*
             *  好，这个包是TCP.
             *  OK, this packet is TCP.
             */
           
            /* 定义/计算 TCP 头部偏移
               define/compute tcp header offset */
            tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
            size_tcp = TH_OFF(tcp)*4;
            if (size_tcp < 20) {
                    printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
                    return;
            }
           
            printf("   Src port: %d\n", ntohs(tcp->th_sport));
            printf("   Dst port: %d\n", ntohs(tcp->th_dport));
           
            /* 定义/计算TCP有效载荷（段）偏移
               define/compute tcp payload (segment) offset */
            payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
           
            /* 计算TCP有效载荷（段）的大小
               compute tcp payload (segment) size */
            size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
           
            /*
             * 打印有效载荷数据，它可能是二进制的，所以不要只把它作为一个字符串。
             * Print payload data; it might be binary, so don't just
             * treat it as a string.
             */
            if (size_payload > 0) {
                    printf("   Payload (%d bytes):\n", size_payload);
                    print_payload(payload, size_payload);
            }

    return;
    }

    int main(int argc, char **argv)
    {

            char *dev = NULL;                                        /* 捕获设备的名称 | capture device name */
            char errbuf[PCAP_ERRBUF_SIZE];                /* 错误的缓冲区   | error buffer */
            pcap_t *handle;                                                /* 数据包捕获句柄 | packet capture handle */

			//char filter_exp[] = "ip host www.baidu.com";
			char filter_exp[] = "src 192.168.3.129 and tcp";
            //char filter_exp[] = "ip";                        /* 过滤表达示          | filter expression [3] */
            struct bpf_program fp;                                /* 编译过滤表达示 | compiled filter program (expression) */
            bpf_u_int32 mask;                                        /* 子网掩码                  | subnet mask */
            bpf_u_int32 net;                                        /* IP 地址                  | ip */
            int num_packets = 10;                                /* 捕获的数据包数量 | number of packets to capture */

            /* 显示程序版本信息 */
            print_app_banner();

            /* 检查来自命令行参数需要捕获设备的名称
               check for capture device name on command-line */
            if (argc == 2) {
                    dev = argv[1];
            }
            else if (argc > 2) {
                    fprintf(stderr, "error: unrecognized command-line options\n\n");
                    print_app_usage();
                    exit(EXIT_FAILURE);
            }
            else {
                    /* 如果命令行参数没有指定, 则自动找到一个设备
                       find a capture device if not specified on command-line */
                    dev = pcap_lookupdev(errbuf);
                    if (dev == NULL) {
                            fprintf(stderr, "Couldn't find default device: %s\n",
                                errbuf);
                            exit(EXIT_FAILURE);
                    }
            }
           
            /* 获得捕获设备的网络号和掩码
               get network number and mask associated with capture device */
            if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
                    fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
                        dev, errbuf);
                    net = 0;
                    mask = 0;
            }

            /* 显示捕获设备信息
               print capture info */
            printf("Device: %s\n", dev);
            printf("Number of packets: %d\n", num_packets);
            printf("Filter expression: %s\n", filter_exp);

            /* 打开捕获设备
               @1        捕获的设备
               @2        每次捕获数据的最大长度
               @3        1 启用混杂模式
               @4        捕获时间, 单位ms
               @5        错误缓冲区
               open capture device */
            handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
            if (handle == NULL) {
                    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
                    exit(EXIT_FAILURE);
            }

            /*        pcap_datalink();
                            返回数据链路层类型，例如DLT_EN10MB;

               确保我们对以太网设备捕获
               make sure we're capturing on an Ethernet device [2] */
            if (pcap_datalink(handle) != DLT_EN10MB) {
                    fprintf(stderr, "%s is not an Ethernet\n", dev);
                    exit(EXIT_FAILURE);
            }

            /* 编译过滤表达式
               compile the filter expression */
            if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
                    fprintf(stderr, "Couldn't parse filter %s: %s\n",
                        filter_exp, pcap_geterr(handle));
                    exit(EXIT_FAILURE);
            }

            /* 应用过滤规则
               apply the compiled filter */
            if (pcap_setfilter(handle, &fp) == -1) {
                    fprintf(stderr, "Couldn't install filter %s: %s\n",
                        filter_exp, pcap_geterr(handle));
                    exit(EXIT_FAILURE);
            }

            /* 设置回高函数并开始捕获包
               now we can set our callback function */
            pcap_loop(handle, num_packets, got_packet, NULL);

            /* cleanup */
            pcap_freecode(&fp);
            pcap_close(handle);

            printf("\nCapture complete.\n");

    return 0;
}

