#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <wiringPi.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <wiringPiSPI.h> 
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define PACKETSIZE	32 //�̼�����

#define MOTOR_EN_A 0    //����
#define MOTOR_IN_1 19
#define MOTOR_IN_2 13
#define MOTOR_IN_3 6
#define MOTOR_IN_4 5
#define MOTOR_EN_B 26

#define PROBEPATH    "/sys/bus/w1/devices"      // �µ�
#define MAXPROBES    5                          
#define PROBENAMELEN 80                         
#define BUFSIZE      256        

#define CS_MCP3208 8  //MCP3208 (������������)
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

#define MAXLINE  511  //����
#define MAX_SOCK 1024 // �ֶ󸮽��� ��� 64

char probepath[MAXPROBES][PROBENAMELEN];
char probename[MAXPROBES][PROBENAMELEN];
char alias[MAXPROBES][BUFSIZE];
FILE *probefd;
int numOfSensor;   

int q=1;      //���� ���°��� ���� ����

char *EXIT_STRING = "exit";	// Ŭ���̾�Ʈ�� �����û ���ڿ�
char *START_STRING = "Connected to chat_server \n";
// Ŭ���̾�Ʈ ȯ�� �޽���
int maxfdp1;				// �ִ� ���Ϲ�ȣ +1
int num_user = 0;			// ä�� ������ ��
int num_chat = 0;			// ���ݱ��� ���� ��ȭ�� ��
int clisock_list[MAX_SOCK];		// ä�ÿ� ������ ���Ϲ�ȣ ���
char ip_list[MAX_SOCK][20];		//������ ip���
int listen_sock;			// ������ ���� ����
void addClient(int s, struct sockaddr_in *newcliaddr); // ���ο� ä�� ������ ó��
int getmax();				// �ִ� ���� ��ȣ ã��
void removeClient(int s);	// ä�� Ż�� ó�� �Լ�
int tcp_listen(int host, int port, int backlog); // ���� ���� �� listen
void errquit(char *mesg) { perror(mesg); exit(1); }



time_t ct;
struct tm tm;
int findprobes(void)            //(�µ�����)
{
	struct dirent *pDirent;
	DIR *pDir;
	int count;
	count = 0;
	pDir = opendir(PROBEPATH);
	if (pDir == NULL) {
		printf("Cannot open directory '%s'\n", PROBEPATH);
		return 0;
	}
	while ((pDirent = readdir(pDir)) != NULL)
	{      //��� DS18B20�� ���̵�� 28-�� �����Ѵ�.
		if (pDirent->d_name[0] == '2' && pDirent->d_name[1] == '8' && pDirent->d_name[2] == '-')
		{
			snprintf(probepath[count], PROBENAMELEN-1, "%s/%s/w1_slave", PROBEPATH, pDirent->d_name);
			snprintf(probename[count], PROBENAMELEN-1, "%s", pDirent->d_name);
			printf ("Found DS18B20 compatible probe named '%s':\nDevice file '%s'\n",probename[count], probepath[count]);
			count++;
		}

	}
	closedir(pDir);
	return count;
}

//ADC ������ mcp3208 ������ (������������)
int ReadMcp3208ADC(unsigned char adcChannel)
{
	unsigned char buff[3];
	int nAdcValue = 0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
	buff[1] = ((adcChannel & 0x07)<<6);
	buff[2] = 0x00;
	digitalWrite(CS_MCP3208,0);
	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
	buff[1] = 0x0F & buff[1];
	nAdcValue = (buff[1]<<8) | buff[2];
	digitalWrite(CS_MCP3208, 1);
	return nAdcValue; 
}





int openi2c(int slave_addr) {	  //�̼�����
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	int fd = open(filename, O_RDWR);
	if (fd < 0) { printf("Failed to open the i2c bus"); return fd; }
	int io = ioctl(fd, I2C_SLAVE, slave_addr);
	if ( io < 0) printf("Failed to acquire bus access and/or talk to slave.\n");
	return fd;
}





int leftloop() //â���� �ݴ� �������� �Լ�
{
	int count=0;
	while(1)
	{
		count++;
		digitalWrite(MOTOR_IN_1,HIGH);
		digitalWrite(MOTOR_IN_2,LOW);
		digitalWrite(MOTOR_IN_3,HIGH);
		digitalWrite(MOTOR_IN_4,LOW);
		delay(5);

		digitalWrite(MOTOR_IN_1,HIGH);
		digitalWrite(MOTOR_IN_2,LOW);
		digitalWrite(MOTOR_IN_3,LOW);
		digitalWrite(MOTOR_IN_4,HIGH);
		delay(5);

		digitalWrite(MOTOR_IN_1,LOW);
		digitalWrite(MOTOR_IN_2,HIGH);
		digitalWrite(MOTOR_IN_3,LOW);
		digitalWrite(MOTOR_IN_4,HIGH);
		delay(5);

		digitalWrite(MOTOR_IN_1,LOW);
		digitalWrite(MOTOR_IN_2,HIGH);
		digitalWrite(MOTOR_IN_3,HIGH);
		digitalWrite(MOTOR_IN_4,LOW);
		delay(5);

		if(count==115)
		{
			return 0;	
		}  
	}
}

int rightloop() //â���� ���� �������� �Լ� 
{
	int count=0;
	while(1)
	{
		count++;
		digitalWrite(MOTOR_IN_1,LOW);
		digitalWrite(MOTOR_IN_2,HIGH);
		digitalWrite(MOTOR_IN_3,HIGH);	
		digitalWrite(MOTOR_IN_4,LOW);
		delay(5); 

		digitalWrite(MOTOR_IN_1,LOW);
		digitalWrite(MOTOR_IN_2,HIGH);
		digitalWrite(MOTOR_IN_3,LOW);
		digitalWrite(MOTOR_IN_4,HIGH);
		delay(5);

		digitalWrite(MOTOR_IN_1,HIGH);
		digitalWrite(MOTOR_IN_2,LOW);
		digitalWrite(MOTOR_IN_3,LOW);
		digitalWrite(MOTOR_IN_4,HIGH);
		delay(5);

		digitalWrite(MOTOR_IN_1,HIGH);
		digitalWrite(MOTOR_IN_2,LOW);
		digitalWrite(MOTOR_IN_3,HIGH);
		digitalWrite(MOTOR_IN_4,LOW);
		delay(5);

		if(count==115)
		{
			return 0;
		}
	}
}







void *thread_function(void *arg) { //��ɾ ó���� ������
	int i;
	printf("��ɾ� ��� : help, num_user, num_chat, ip_list\n");
	while (1) {
		char bufmsg[MAXLINE + 1];
		fprintf(stderr, "\033[1;32m"); //���ڻ��� ������� ����
		printf("server>"); //Ŀ�� ���
		fgets(bufmsg, MAXLINE, stdin); //��ɾ� �Է�
		if (!strcmp(bufmsg, "\n")) continue;   //���� ����
		else if (!strcmp(bufmsg, "help\n"))    //��ɾ� ó��
			printf("help, num_user, num_chat, ip_list\n");
		else if (!strcmp(bufmsg, "num_user\n"))//��ɾ� ó��
			printf("���� ������ �� = %d\n", num_user);
		else if (!strcmp(bufmsg, "num_chat\n"))//��ɾ� ó��
			printf("���ݱ��� ���� ��ȭ�� �� = %d\n", num_chat);
		else if (!strcmp(bufmsg, "ip_list\n")) //��ɾ� ó��
			for (i = 0; i < num_user; i++)
				printf("%s\n", ip_list[i]);
		else //���� ó��
			printf("�ش� ��ɾ �����ϴ�.help�� �����ϼ���.\n");
	}
}



void *thread_function2(void *arg) { //�������� ���� �������� ���͸� �����ϴ�
	int nAdcChannel = 0;
	int nAdcValue = 0;
	double temperature;
	char *temp;
	time_t now;
	struct tm *t;
	char buf2[BUFSIZE];
	numOfSensor = findprobes();

	int fd = openi2c(0x28);

	while(1){
		unsigned char data[PACKETSIZE];
		int n = read(fd, data, PACKETSIZE);
		probefd = fopen(probepath[0], "r"); //probepath�� ����� ��ġ�κ��� �������� �о�� �� �ִ�.(�µ�����)
		
		nAdcValue = ReadMcp3208ADC(nAdcChannel); // ������������
		if (probefd == NULL)
		{
			printf( "Error: Unable to open '%s': %s\n",
				probepath[0], strerror(errno));
			exit(-1);
		}
		fgets(buf2, sizeof(buf2)-1, probefd); // ù��° ���� ���� ��
		memset(buf2, 0, sizeof(buf2));
		fgets(buf2, sizeof(buf2)-1, probefd); //�ι�° ���� �ҷ��ͼ�
		temp = strtok(buf2, "t="); //"t="�ڿ� ���� �µ����� �Ľ� �Ѵ�.
		temp = strtok(NULL, "t=");
		temperature = atof(temp)/1000; //1000���� ������� ���� �� �µ����� ��   
		t = localtime(&now);


		printf("�µ� : %2.3f\n",temperature);
		printf("�������� : %u\n", nAdcValue);
		if(((256*data[23]+data[24]>80)||(256*data[25]+data[26]>100)||(temperature<=15.0)||(nAdcValue<=3000))&&(q==1))
		{
			leftloop();
			q = 0;
		}
		printf("�̼����� : %d , %d\n",256*data[23]+data[24], 256*data[25]+data[26]);
		sleep(2);
		fclose(probefd);
	}
}





int main(int argc, char *argv[]) {
	int count;
	int nAdcChannel = 0;
	int nAdcValue = 0;
	double temperature;
	char *temp;
	time_t now;
	struct tm *t;
	char buf2[BUFSIZE];
	numOfSensor = findprobes();

	if (numOfSensor == 0)
	{
		printf("Error: No DS18B20 compatible probes located.\n");
		exit(-1);
	}  

	int fd = openi2c(0x28);

	if (fd < 0) 
	{
		printf("Error in opening i2c. fd=%d\n", fd);
		return fd;
	}
	if(wiringPiSetupGpio()==-1)
	{
		return 1;
	}
	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
		fprintf(stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
		return 1;
	}
	struct sockaddr_in cliaddr;
	char buf[MAXLINE + 1]; //Ŭ���̾�Ʈ���� ���� �޽���
	int i, j, nbyte, accp_sock, addrlen = sizeof(structsockaddr_in);
	fd_set read_fds;	//�б⸦ ������ fd_set ����ü
	pthread_t a_thread, b_thread;

	pinMode(MOTOR_EN_A,OUTPUT); //���� �� 
	pinMode(MOTOR_EN_B,OUTPUT);
	pinMode(MOTOR_IN_1,OUTPUT);
	pinMode(MOTOR_IN_2,OUTPUT);
	pinMode(MOTOR_IN_3,OUTPUT);
	pinMode(MOTOR_IN_4,OUTPUT);

	digitalWrite(MOTOR_EN_A,1);
	digitalWrite(MOTOR_EN_B,1);

	if (argc != 2) {
		printf("���� :%s port\n", argv[0]);
		exit(0);
	}

	// tcp_listen(host, port, backlog) �Լ� ȣ��
	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
	//������ ����
	pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
	pthread_create(&b_thread, NULL, thread_function2, (void *)NULL);
	while (1) {		

		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);

		for (i = 0; i < num_user; i++)
			FD_SET(clisock_list[i], &read_fds);	

		maxfdp1 = getmax() + 1;	// maxfdp1 �� ���
		if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");

		if (FD_ISSET(listen_sock, &read_fds)) {
			accp_sock = accept(listen_sock,
				(struct sockaddr*)&cliaddr, &addrlen);
			if (accp_sock == -1) errquit("accept fail");
			addClient(accp_sock, &cliaddr);
			send(accp_sock, START_STRING, strlen(START_STRING), 0);
			ct = time(NULL);			//���� �ð��� �޾ƿ�
			tm = *localtime(&ct);
			write(1, "\033[0G", 4);		//Ŀ���� X��ǥ�� 0���� �̵�
			printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(stderr, "\033[33m");//���ڻ��� ��������� ����
			printf("����� 1�� �߰�. ���� ������ �� = %d\n", num_user);
			fprintf(stderr, "\033[32m");//���ڻ��� ������� ����
			fprintf(stderr, "server>"); //Ŀ�� ���
		}

		// Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ���
		for (i = 0; i < num_user; i++) {
			if (FD_ISSET(clisock_list[i], &read_fds)) {
				num_chat++;				//�� ��ȭ �� ����
				nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
				if (nbyte <= 0) {
					removeClient(i);	// Ŭ���̾�Ʈ�� ����
					continue;
				}
				buf[nbyte] = 0;
				// ���Ṯ�� ó��
				if (strstr(buf, EXIT_STRING) != NULL) {
					removeClient(i);	// Ŭ���̾�Ʈ�� ����
					continue;
				}
				// ��� ä�� �����ڿ��� �޽��� ���
				for (j = 0; j < num_user; j++)
					send(clisock_list[j], buf, nbyte, 0);
				printf("\033[0G");		//Ŀ���� X��ǥ�� 0���� �̵�
				fprintf(stderr, "\033[97m");//���ڻ��� ������� ����
				printf("%s", buf);			//�޽��� ���		
				if(nbyte == 5 && q == 0){
					rightloop();
					q = 1;
				}
				else if(nbyte == 6 && q == 1){
					leftloop();
					q = 0;
				}
				fprintf(stderr, "\033[32m");//���ڻ��� ������� ����
				fprintf(stderr, "server>"); //Ŀ�� ���
			}		
		}
	}  // end of while
	return 0;
}



// ���ο� ä�� ������ ó��
void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
	write(1, "\033[0G", 4);		//Ŀ���� X��ǥ�� 0���� �̵�
	fprintf(stderr, "\033[33m");	//���ڻ��� ��������� ����
	printf("new client: %s\n", buf);//ip���
	// ä�� Ŭ���̾�Ʈ ��Ͽ� �߰�
	clisock_list[num_user] = s;
	strcpy(ip_list[num_user], buf);
	num_user++; //���� �� ����
}



// ä�� Ż�� ó��
void removeClient(int s) {
	close(clisock_list[s]);
	if (s != num_user - 1) { //����� ����Ʈ ��迭
		clisock_list[s] = clisock_list[num_user - 1];
		strcpy(ip_list[s], ip_list[num_user - 1]);
	}
	num_user--; //���� �� ����
	ct = time(NULL);			//���� �ð��� �޾ƿ�
	tm = *localtime(&ct);
	write(1, "\033[0G", 4);		//Ŀ���� X��ǥ�� 0���� �̵�
	fprintf(stderr, "\033[33m");//���ڻ��� ��������� ����
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("ä�� ������ 1�� Ż��. ���� ������ �� = %d\n", num_user);
	fprintf(stderr, "\033[32m");//���ڻ��� ������� ����
	fprintf(stderr, "server>"); //Ŀ�� ���
}



// �ִ� ���Ϲ�ȣ ã��

int getmax() {
	// Minimum ���Ϲ�ȣ�� ���� ���� ������ listen_sock
	int max = listen_sock;
	int i;
	for (i = 0; i < num_user; i++)
		if (clisock_list[i] > max)
			max = clisock_list[i];
	return max;
}



// listen ���� ���� �� listen
int  tcp_listen(int host, int port, int backlog) {
	int sd;
	struct sockaddr_in servaddr;
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket fail");
		exit(1);
	}
	// servaddr ����ü�� ���� ����
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind fail");  exit(1);
	}
	// Ŭ���̾�Ʈ�κ��� �����û�� ��ٸ�
	listen(sd, backlog);
	return sd;
}