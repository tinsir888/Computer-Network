//server�˽����ļ� 
#include "reliableudp.h"
SOCKET server;
string filename;
char message[200000000];
static char pindex[2];		// ���ڱ�־�Ѿ��յ����ĸ���
// checksum����
unsigned char check_pkg(char *arr, int len)
{
	if (len == 0)
		return ~(0);
	unsigned char ret = arr[0];
	for (int i = 1; i < len; i++)
	{
		unsigned int tmp = ret + (unsigned char)arr[i];
		tmp = (tmp >> 8) + tmp % (1 << 8);
		tmp = (tmp >> 8) + tmp % (1 << 8);
		ret = tmp;
	}
	return ~ret;
}
// �ȴ�����
void WaitConnection()
{
	while (1)
	{
		char recv[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(server, recv, 2, 0, (sockaddr *)&clientAddr, &clientlen) == SOCKET_ERROR);
		if (recv[0] != SEQ1)
			continue;
		cout << "���ն��յ���һ�����֡�" << endl;
		bool flag = CONNECTSUCCESS;
		while (1)
		{
			memset(recv, 0, 2);
			char send[2];
			send[0] = SEQ2;
			send[1] = ACK2;
			sendto(server, send, 2, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			cout << "���ն˷��͵ڶ�������" << endl;
			while (recvfrom(server, recv, 2, 0, (sockaddr *)&clientAddr, &clientlen) == SOCKET_ERROR);
			if (recv[0] == SEQ1)
				continue;
			if (recv[0] != SEQ3 || recv[1] != ACK3)
			{
				cout << "����ʧ�ܡ���\n���������Ͷˡ�" << endl;
				flag = CONNECTFAIL;
			}
			break;
		}
		if (!flag)
			continue;
		break;
	}
}
// �������ն�
int StartServer()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup����" << WSAGetLastError() << endl;
		return -1;
	}
	server = socket(AF_INET, SOCK_DGRAM, 0);

	if (server == SOCKET_ERROR)
	{
		cout << "�׽��ִ���" << WSAGetLastError() << endl;
		return -1;
	}
	int Port = 1439;
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Port);

	if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "�󶨶˿ڴ���" << WSAGetLastError() << endl;
		return -1;
	}
	cout << "�ɹ��������նˣ�" << endl;
	return 1;
}
// �ȴ��Ͽ�����
void WaitDisconnection()
{
	while (1)
	{
		char recv[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(server, recv, 2, 0, (sockaddr *)&clientAddr, &clientlen) == SOCKET_ERROR);
		if (recv[0] != WAVE1)
			continue;
		cout << "���ն˽��յ�һ�λ��֡�" << endl;
		char send[2];
		send[0] = WAVE2;
		send[1] = ACKW2;
		sendto(server, send, 2, 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
		break;
	}
	cout << "���Ͷ����ӶϿ���" << endl;
}
// ����
void Recvmessage()
{
	pindex[0] = 0;
	pindex[1] = -1;
	int len = 0;
	while (1)
	{
		char recv[LENGTH + CheckSum];
		memset(recv, '\0', LENGTH + CheckSum);
		int length;
		while (1)
		{
			int clientlen = sizeof(clientAddr);
			while (recvfrom(server, recv, LENGTH + CheckSum, 0, (sockaddr *)&clientAddr, &clientlen) == SOCKET_ERROR);
			length = recv[4] * 128 + recv[5];
			char send[3];
			memset(send, '\0', 3);
			printf("���к�: %d ����: %dByte ", recv[2]*128 + recv[3] ,length);
			// ���ACK
			if (recv[6] == ACKMsg)
			{
				send[0] = ACKMsg;
				// ��һ�����ǲ���˳�����һ����
				if (((pindex[0] == recv[2] && pindex[1] + 1 == recv[3]) || (pindex[0] + 1 == recv[2] && recv[3] == 0 && pindex[1] == 127)) && check_pkg(recv, length + CheckSum) == 0)
				{
					pindex[0] = recv[2];
					pindex[1] = recv[3];
					send[1] = recv[2];
					send[2] = recv[3];
					printf("У����ȷ\n");
				}
				else
				{
					printf("�����ˡ�\n");
					send[1] = -1;
					send[2] = -1;
					sendto(server, send, 3, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
					continue;
				}
				sendto(server, send, 3, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
				break;
			}
			// ������ ����һ��NAK
			else
			{
				printf("�����ˡ�\n");
				send[0] = NAK;
				send[1] = recv[2];
				send[2] = recv[3];
				sendto(server, send, 3, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
				continue;
			}
		}
		for (int i = CheckSum; i < length + CheckSum; i++)
		{
			message[len] = recv[i];
			len++;
		}
		if (recv[1] == LAST)
			break;
	}
	ofstream fout(filename.c_str(), ofstream::binary);
	for (int i = 0; i < len; i++)
		fout << message[i];
	fout.close();
}
// �����ļ���
void RecvName()
{
	char name[100];
	int clientlen = sizeof(clientAddr);
	while (recvfrom(server, name, 100, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
	cout << "�ļ���Ϊ: ";
	for (int i = 0; name[i] != '$'; i++)
	{
		filename += name[i];
		putchar(name[i]);
	}
	cout << endl;
}
int main()
{
	StartServer();
	cout << "�ȴ����Ͷ�����..." << endl;
	WaitConnection();
	cout << "�ɹ����ӵ����Ͷˣ�" << endl;
	RecvName();
	Recvmessage();
	cout << "�����ļ�������" << endl;
	WaitDisconnection();
	closesocket(server);
	WSACleanup();
	system("pause");
	return 0;
}
