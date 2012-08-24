#include <stdio.h>
#include <pcap.h>
	 
int main(int argc, char *argv[]) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_if_t* devs;
	pcap_if_t* d;
	unsigned int i = 0;
	 
	//��ȡȫ����dev
	if (-1 == pcap_findalldevs(&devs, errbuf)) {
		fprintf(stderr, "Could not list device: %s\n", errbuf);
	} else {
		d = devs;
		while (d->next != NULL) {
			printf("%d:%s\n", i++, d->name);
			d = d->next;
		}
	}
	 
	//�ͷ����л�ȡ��dev
	pcap_freealldevs(devs);
	return (0);
}
