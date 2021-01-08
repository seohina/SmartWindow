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

#define PACKETSIZE	32 //미세먼지

#define MOTOR_EN_A 0    //모터
#define MOTOR_IN_1 19
#define MOTOR_IN_2 13
#define MOTOR_IN_3 6
#define MOTOR_IN_4 5
#define MOTOR_EN_B 26

#define PROBEPATH    "/sys/bus/w1/devices"      // 온도
#define MAXPROBES    5                          
#define PROBENAMELEN 80                         
#define BUFSIZE      256        

#define CS_MCP3208 8  //MCP3208 (빗물감지센서)
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

#define MAXLINE  511  //서버
#define MAX_SOCK 1024 // 솔라리스의 경우 64

char probepath[MAXPROBES][PROBENAMELEN];
char probename[MAXPROBES][PROBENAMELEN];
char alias[MAXPROBES][BUFSIZE];
FILE *probefd;
int numOfSensor;   

int q=1;      //모터 상태값이 들어가는 변수

char *EXIT_STRING = "exit";	// 클라이언트의 종료요청 문자열
char *START_STRING = "Connected to chat_server \n";
// 클라이언트 환영 메시지
int maxfdp1;				// 최대 소켓번호 +1
int num_user = 0;			// 채팅 참가자 수
int num_chat = 0;			// 지금까지 오간 대화의 수
int clisock_list[MAX_SOCK];		// 채팅에 참가자 소켓번호 목록
char ip_list[MAX_SOCK][20];		//접속한 ip목록
int listen_sock;			// 서버의 리슨 소켓
void addClient(int s, struct sockaddr_in *newcliaddr); // 새로운 채팅 참가자 처리
int getmax();				// 최대 소켓 번호 찾기
void removeClient(int s);	// 채팅 탈퇴 처리 함수
int tcp_listen(int host, int port, int backlog); // 소켓 생성 및 listen
void errquit(char *mesg) { perror(mesg); exit(1); }



time_t ct;
struct tm tm;
int findprobes(void)            //(온도센서)
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
	{      //모든 DS18B20의 아이디는 28-로 시작한다.
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

//ADC 컨버터 mcp3208 센서값 (빗물감지센서)
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





int openi2c(int slave_addr) {	  //미세먼지
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	int fd = open(filename, O_RDWR);
	if (fd < 0) { printf("Failed to open the i2c bus"); return fd; }
	int io = ioctl(fd, I2C_SLAVE, slave_addr);
	if ( io < 0) printf("Failed to acquire bus access and/or talk to slave.\n");
	return fd;
}





int leftloop() //창문을 닫는 모터제어 함수
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

int rightloop() //창문을 여는 모터제어 함수 
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







void *thread_function(void *arg) { //명령어를 처리할 스레드
	int i;
	printf("명령어 목록 : help, num_user, num_chat, ip_list\n");
	while (1) {
		char bufmsg[MAXLINE + 1];
		fprintf(stderr, "\033[1;32m"); //글자색을 녹색으로 변경
		printf("server>"); //커서 출력
		fgets(bufmsg, MAXLINE, stdin); //명령어 입력
		if (!strcmp(bufmsg, "\n")) continue;   //엔터 무시
		else if (!strcmp(bufmsg, "help\n"))    //명령어 처리
			printf("help, num_user, num_chat, ip_list\n");
		else if (!strcmp(bufmsg, "num_user\n"))//명령어 처리
			printf("현재 참가자 수 = %d\n", num_user);
		else if (!strcmp(bufmsg, "num_chat\n"))//명령어 처리
			printf("지금까지 오간 대화의 수 = %d\n", num_chat);
		else if (!strcmp(bufmsg, "ip_list\n")) //명령어 처리
			for (i = 0; i < num_user; i++)
				printf("%s\n", ip_list[i]);
		else //예외 처리
			printf("해당 명령어가 없습니다.help를 참조하세요.\n");
	}
}



void *thread_function2(void *arg) { //센서들의 값을 가져오고 모터를 제어하는
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
		probefd = fopen(probepath[0], "r"); //probepath에 저장된 위치로부터 센서값을 읽어올 수 있다.(온도센서)
		
		nAdcValue = ReadMcp3208ADC(nAdcChannel); // 빗물감지센서
		if (probefd == NULL)
		{
			printf( "Error: Unable to open '%s': %s\n",
				probepath[0], strerror(errno));
			exit(-1);
		}
		fgets(buf2, sizeof(buf2)-1, probefd); // 첫번째 줄은 무시 함
		memset(buf2, 0, sizeof(buf2));
		fgets(buf2, sizeof(buf2)-1, probefd); //두번째 줄을 불러와서
		temp = strtok(buf2, "t="); //"t="뒤에 오는 온도값을 파싱 한다.
		temp = strtok(NULL, "t=");
		temperature = atof(temp)/1000; //1000으로 나누어야 원하 는 온도값이 됨   
		t = localtime(&now);


		printf("온도 : %2.3f\n",temperature);
		printf("빗물감지 : %u\n", nAdcValue);
		if(((256*data[23]+data[24]>80)||(256*data[25]+data[26]>100)||(temperature<=15.0)||(nAdcValue<=3000))&&(q==1))
		{
			leftloop();
			q = 0;
		}
		printf("미세먼지 : %d , %d\n",256*data[23]+data[24], 256*data[25]+data[26]);
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
	char buf[MAXLINE + 1]; //클라이언트에서 받은 메시지
	int i, j, nbyte, accp_sock, addrlen = sizeof(structsockaddr_in);
	fd_set read_fds;	//읽기를 감지할 fd_set 구조체
	pthread_t a_thread, b_thread;

	pinMode(MOTOR_EN_A,OUTPUT); //모터 핀 
	pinMode(MOTOR_EN_B,OUTPUT);
	pinMode(MOTOR_IN_1,OUTPUT);
	pinMode(MOTOR_IN_2,OUTPUT);
	pinMode(MOTOR_IN_3,OUTPUT);
	pinMode(MOTOR_IN_4,OUTPUT);

	digitalWrite(MOTOR_EN_A,1);
	digitalWrite(MOTOR_EN_B,1);

	if (argc != 2) {
		printf("사용법 :%s port\n", argv[0]);
		exit(0);
	}

	// tcp_listen(host, port, backlog) 함수 호출
	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
	//스레드 생성
	pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
	pthread_create(&b_thread, NULL, thread_function2, (void *)NULL);
	while (1) {		

		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);

		for (i = 0; i < num_user; i++)
			FD_SET(clisock_list[i], &read_fds);	

		maxfdp1 = getmax() + 1;	// maxfdp1 재 계산
		if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");

		if (FD_ISSET(listen_sock, &read_fds)) {
			accp_sock = accept(listen_sock,
				(struct sockaddr*)&cliaddr, &addrlen);
			if (accp_sock == -1) errquit("accept fail");
			addClient(accp_sock, &cliaddr);
			send(accp_sock, START_STRING, strlen(START_STRING), 0);
			ct = time(NULL);			//현재 시간을 받아옴
			tm = *localtime(&ct);
			write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
			printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(stderr, "\033[33m");//글자색을 노란색으로 변경
			printf("사용자 1명 추가. 현재 참가자 수 = %d\n", num_user);
			fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
			fprintf(stderr, "server>"); //커서 출력
		}

		// 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
		for (i = 0; i < num_user; i++) {
			if (FD_ISSET(clisock_list[i], &read_fds)) {
				num_chat++;				//총 대화 수 증가
				nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
				if (nbyte <= 0) {
					removeClient(i);	// 클라이언트의 종료
					continue;
				}
				buf[nbyte] = 0;
				// 종료문자 처리
				if (strstr(buf, EXIT_STRING) != NULL) {
					removeClient(i);	// 클라이언트의 종료
					continue;
				}
				// 모든 채팅 참가자에게 메시지 방송
				for (j = 0; j < num_user; j++)
					send(clisock_list[j], buf, nbyte, 0);
				printf("\033[0G");		//커서의 X좌표를 0으로 이동
				fprintf(stderr, "\033[97m");//글자색을 흰색으로 변경
				printf("%s", buf);			//메시지 출력		
				if(nbyte == 5 && q == 0){
					rightloop();
					q = 1;
				}
				else if(nbyte == 6 && q == 1){
					leftloop();
					q = 0;
				}
				fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
				fprintf(stderr, "server>"); //커서 출력
			}		
		}
	}  // end of while
	return 0;
}



// 새로운 채팅 참가자 처리
void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
	write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");	//글자색을 노란색으로 변경
	printf("new client: %s\n", buf);//ip출력
	// 채팅 클라이언트 목록에 추가
	clisock_list[num_user] = s;
	strcpy(ip_list[num_user], buf);
	num_user++; //유저 수 증가
}



// 채팅 탈퇴 처리
void removeClient(int s) {
	close(clisock_list[s]);
	if (s != num_user - 1) { //저장된 리스트 재배열
		clisock_list[s] = clisock_list[num_user - 1];
		strcpy(ip_list[s], ip_list[num_user - 1]);
	}
	num_user--; //유저 수 감소
	ct = time(NULL);			//현재 시간을 받아옴
	tm = *localtime(&ct);
	write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");//글자색을 노란색으로 변경
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_user);
	fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
	fprintf(stderr, "server>"); //커서 출력
}



// 최대 소켓번호 찾기

int getmax() {
	// Minimum 소켓번호는 가정 먼저 생성된 listen_sock
	int max = listen_sock;
	int i;
	for (i = 0; i < num_user; i++)
		if (clisock_list[i] > max)
			max = clisock_list[i];
	return max;
}



// listen 소켓 생성 및 listen
int  tcp_listen(int host, int port, int backlog) {
	int sd;
	struct sockaddr_in servaddr;
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket fail");
		exit(1);
	}
	// servaddr 구조체의 내용 세팅
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind fail");  exit(1);
	}
	// 클라이언트로부터 연결요청을 기다림
	listen(sd, backlog);
	return sd;
}