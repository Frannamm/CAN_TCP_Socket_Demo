#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>


int main( int argc, char *argv[]){

	if (argc != 2) {
    printf("Usage: %s <interface>\n", argv[0]);
    exit(1);
	}

	char *interfaceName = argv[1];

	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) { 
	perror("socket failed");
	exit(1); 
	}
	
	printf("CAN socket created successfully...\n");

	struct ifreq ifr;
	strcpy(ifr.ifr_name, interfaceName);
	ioctl(s, SIOCGIFINDEX, &ifr);

	printf("Interface index for %s: %d\n", interfaceName, ifr.ifr_ifindex);

	struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(1);
	}
	printf("Bound to %s successfully...\n", interfaceName);

	struct can_frame frame;
	int nbytes = read(s, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		perror("read failed");
		exit(1);
	}

	printf("Received CAN ID: %X\n", frame.can_id);
	printf("Data length: %d\n", frame.can_dlc);

	printf("Data: ");
	for(int i=0; i<frame.can_dlc; i++) {
		printf("%02X ", frame.data[i]);
	}
	printf("\n");	

	return 0 ;
}