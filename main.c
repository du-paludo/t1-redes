int sock_r;
sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
if (sock_r < 0) {
    print("error in socket \n");
    return -1;
}

unsigned char *buffer = (unsigned char *) malloc(65536); //to receive data
memset(buffer,0,65536);
struct sockaddr saddr;
int saddr_len = sizeof (saddr);
 
//Receive a network packet and copy in to buffer
buflen = recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);
if (buflen<0) {
    printf(error in reading recvfrom function\n);
    return -1;
}

