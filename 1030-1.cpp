#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WINSOCK2.h>
#pragma comment(lib,"ws2_32.lib")

int main()
{
	//1.��ʼ���������socket

	//����׽�����Ϣ�Ľṹ
	WSADATA wsaData = { 0 };
	SOCKET serverSocket = INVALID_SOCKET;//������׽���
	SOCKET clientSocket = INVALID_SOCKET;//�ͻ����׽���
	SOCKADDR_IN serverAddr = { 0 };//����˵�ַ
	SOCKADDR_IN clientAddr = { 0 };//�ͻ��˵�ַ
	int iClientAddrLen = sizeof(clientAddr);
	USHORT uPort = 18000;//���÷����������˿�

	//����WSAStartup������ʼ���׽��֣��ɹ��򷵻�0
	//MAKEWORD��ϵͳָ��ʹ�õ�winsock�汾���Ӷ�ʹ�ø߰汾��Winsock����ʹ��
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return 0;//ʧ�ܾ��˳�����
	}

	//�ж��׽��ְ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("wVersion was not 2.2!\n");
		return 0;
	}

	//������socket��ָ����ַ���͡������Э������
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Server socket creation failed with error code: %d\n", WSAGetLastError());
		return 0;
	}

	//���÷�������ַ
	serverAddr.sin_family = AF_INET;//���ӷ�ʽ
	serverAddr.sin_port = htons(uPort);//�����������˿�
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�κοͻ��˶����������������

	//����������ַ�󶨵���socket��ʹ�÷��������й̶���IP�Ͷ˿ں�
	if (SOCKET_ERROR == bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		printf("Binding failed with error code: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		return 0;
	}
	//��socket�������޿ͻ������ӣ��������ӵȴ����е���󳤶�Ϊ1
	if (SOCKET_ERROR == listen(serverSocket, 1))
	{
		printf("Listening failed with error code: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	//����Server���û�������������
	printf("Please input server's name:");
	char name[32] = { 0 };
	gets_s(name);

	printf("waiting to connect.....\n");
	//����пͻ����������ӣ���������������socket�Ķ˿ڽ��յ�һ��sockaddr��ַ�ṹ�����������socket�ͽ������ӣ���Ϊ֮����һ��������socket
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &iClientAddrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Accepting failed with error code: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	//inet_ntoa������ʮ��������ת���ɵ��ʮ����
	printf("Successfully got a connection from IP:%s Port:%d\n\n",
		inet_ntoa(clientAddr.sin_addr), htons(clientAddr.sin_port));
		//InetPton(AF_INET, _T("192.168.1.1"), &ClientAddr.sin_addr);

	char buffer[4096] = { 0 };//�������4096�ֽ�
	int iRecvLen = 0;
	int iSendLen = 0;

	//���ȷ����û������ͻ���
	iSendLen = send(clientSocket, name, strlen(name), 0);
	if (SOCKET_ERROR == iSendLen)
	{
		printf("Sending failed with error code: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}
	else printf("Successfully sent the server's name to the client!\n");
	//���տͻ��˵��û���
	char nameOther[32] = { 0 };
	iRecvLen = recv(clientSocket, nameOther, sizeof(nameOther), 0);
	if (SOCKET_ERROR == iRecvLen)
	{
		printf("Receiving failed with error code: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}
	else printf("Successfully received the client's name from the client!\n\n\n\n");
	strcat_s(nameOther, "\0");

	//��ȡ��ǰϵͳʱ��
	time_t Time_now = time(0);

	//������Ϣѭ����һֱ���տͻ��˷�������Ϣ ���Ұ���Ϣ���ͻ�ȥ
	while (1)
	{

		memset(buffer, 0, sizeof(buffer));
		//���տͻ�����Ϣ
		iRecvLen = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (SOCKET_ERROR == iRecvLen)
		{
			printf("Receiving failed with error code: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		}
		//printf("recv %d bytes from %s: ", iRecvLen, nameOther);
		strcat_s(buffer, "\0"); //���յ�������ĩβû�н��������޷�ֱ����Ϊ�ַ�����ӡ��Ϊ��Ҫ����һ��������
		printf("%s: %s\n", nameOther, buffer);

		//������Ϣ���ͻ���
		memset(buffer, 0, sizeof(buffer));
		printf("%s: ", name);
		gets_s(buffer);
		char* msg = ctime(&Time_now);
		char* sendingTime = msg;
		strncat(msg, buffer, sizeof(buffer));
		//����Է��������bye���ͽ������������˳����죩
		if (strcmp(buffer, "bye") == 0) break;
		iSendLen = send(clientSocket, msg, strlen(msg), 0);
		if (SOCKET_ERROR == iSendLen)
		{
			printf("Sending failed with error code: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		}
		printf("Successfully send message to client: %s\nSend time: %s\n", nameOther, sendingTime);

	}
	//�ȹرո��ͻ��˴�����socket���ٹر���socket
	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}


