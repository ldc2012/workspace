#include <stdio.h>
#include <pcap.h>


void 
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) 
{
	printf("callback packet ... ... \n");
}

int main(int argc, char *argv[]) {
	pcap_t *handle; /* 会话句柄 */
	char *dev /* 被嗅探的 设备 */, errbuf[PCAP_ERRBUF_SIZE]; /* 错误信息 */
	struct bpf_program fp; /* 编译后的过滤表达式 */
	char filter_exp[] = "ip"; /* 过滤表达式 */
	bpf_u_int32 mask; /* 嗅探设备的网络掩码 */
	bpf_u_int32 net; /* 嗅探设备 的IP */
	struct pcap_pkthdr header; /* pcap头 */
	const u_char *packet; /* 数据包 */
	int num_packets = 10; /* number of packets to capture */

	 /* 定义设备 */
	dev = pcap_lookupdev(errbuf);
	if (NULL == dev) {
		fprintf(stderr, "Couldn't find default device: %s ", errbuf);
		return 2;
	}
	printf("Device:%s\n", dev);
	/* 取得设备属性 */
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s ", dev, errbuf);
		net = 0;
		mask = 0;
	}
	/* 以混杂模式打开会话 */
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (NULL == handle) {
		fprintf(stderr, "Couldn't open device %s: %s ", dev, errbuf);
		return 2;
	}
	/* 编译并应用过滤器 */
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s ", filter_exp, pcap_geterr(handle));
		return 2;
	}
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s ", filter_exp, pcap_geterr(handle));
		return 2;
	}
	/* now we can set our callback function */
	pcap_loop(handle, num_packets, got_packet, NULL);

	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);

	return 0;
}
