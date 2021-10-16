#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <WINSOCK2.h>
#pragma comment(lib,"ws2_32.lib")
int main()
{


	//1.��ʼ���������ͻ����׽���

	WSADATA wsaData = { 0 };//����׽�����Ϣ�����ݽṹ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))//�׽��ֳ�ʼ��
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}
	//�ж��׽��ְ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("wVersion was not 2.2\n");
		return -1;
	}

	//�����ͻ����׽��֣�ָ����ַ���͡��������ͺ�Э�飨TCP��
	SOCKET ClientSocket = INVALID_SOCKET;//�����������ӵĿͻ����׽���
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//��ʼ�����׽���
	if (ClientSocket == INVALID_SOCKET)//�жϳ�ʼ���Ƿ�ɹ��������˳�����
	{
		printf("socket failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	//2.�û��������в���������Ҫ���ӵ��ķ�������IP�Ͷ˿ں�

	//���������IP��Ҫ���ӵ���IP��ַ��
	printf("Please input server IP:");
	char IP[32] = { 0 };//IP��ַ�ַ���������
	gets_s(IP);//��ȡ���뵽���з�Ϊֹ
	//����������û���
	printf("Please input your name:");
	char name[32] = { 0 };
	gets_s(name);
	USHORT uPort = 18000;//����˶˿�

	//����Ҫ���ӵ��ķ������ĵ�ַ������IP�Ͷ˿ں�
	SOCKADDR_IN ServerAddr = { 0 };//��ŷ���˵�ַ�����ݽṹ
	ServerAddr.sin_family = AF_INET;//ָ����ַ���ͣ�������IPv4
	ServerAddr.sin_port = htons(uPort);//ָ���˿�
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);//ָ����������ַ��һ��UINT����unsigned long������

	printf("connecting......\n");
	//���ӷ�������ʧ�����˳�����
	if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		printf("connect failed with error code: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
	}
	//���ӳɹ����ӡ����˵ĵ�ַ�Ͷ˿ں�
	printf("connecting server successfully IP:%s Port:%d\n\n",
		inet_ntoa(ServerAddr.sin_addr), htons(ServerAddr.sin_port));

	
	//3.�շ���Ϣ��TCPʹ��send��recv����

	char buffer[4096] = { 0 };//�����Ϣ�Ļ�����
	int iRecvLen = 0;//�յ����ֽ���
	int iSnedLen = 0;//���͵��ֽ���
	//�ʼ�����û������Է�������ʵ�ʷ��͵��ֽ���
	iSnedLen = send(ClientSocket, name, strlen(name), 0);
	if (SOCKET_ERROR == iSnedLen)
	{
		printf("send failed with error code: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
	}
	else printf("Successfully sent your name to the server!\n");
	//Ȼ����նԷ����û�����Ҳ����ʵ�ʽ��յ��ֽ���
	char nameOther[32] = { 0 };
	iRecvLen = recv(ClientSocket, nameOther, sizeof(nameOther), 0);
	if (SOCKET_ERROR == iRecvLen)
	{
		printf("send failed with error code: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
	}
	else printf("Successfully sent the server's name to you!\n\n\n\n");
	//�����ֽ�����ĩβҪ����"\0"���ܹ����ַ���
	strcat_s(nameOther, "\0");

	time_t Time_now = time(0);//��ȡʱ��

	//������������Ϣѭ�������ͺͽ���������Ϣ
	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		//������Ϣ
		printf("%s: ", name);
		gets_s(buffer);
		char* msg = ctime(&Time_now);
		char* sendingTime = msg;
		strncat(msg, buffer, sizeof(buffer));
		//����"bye"ʱ�˳�����
		if (strcmp(buffer, "bye") == 0) {
			iSnedLen = send(ClientSocket, buffer, strlen(buffer), 0);
			if (SOCKET_ERROR == iSnedLen)
			{
				printf("send failed with error code: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 0;
			}
			break;
		}
		//������Ϣ
		iSnedLen = send(ClientSocket, msg, strlen(msg), 0);
		if (SOCKET_ERROR == iSnedLen)
		{
			printf("send failed with error code: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 0;
		}
		printf("Successfully send message to Server: %s\nSend time: %s\n", nameOther, sendingTime);
		memset(buffer, 0, sizeof(buffer));
		//������Ϣ����ӡ��stdout
		iRecvLen = recv(ClientSocket, buffer, sizeof(buffer), 0);
		if (SOCKET_ERROR == iRecvLen)
		{
			printf("send failed with error code: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 0;
		}
		strcat_s(buffer, "\0");
		printf("%s: %s\n", nameOther, buffer);

	}
	//�ر�socket����
	closesocket(ClientSocket);
	//�ͷ�socket DLL��Դ
	WSACleanup();
	return 0;
}
