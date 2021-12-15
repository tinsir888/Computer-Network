#include "reliableudp.h"
SOCKET server;
string filename;
int pkgnolen[maxn];//ÿ�����ݰ��ĳ���
char downloadfile[maxn];
bool issendfinish(char* s) {
	return *s == 'f' && *(s + 1) == 'i' && *(s + 2) == 'n';
}
int main()
{
	//�������ն�
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup ����" << WSAGetLastError() << endl;
		return 0;
	}
	server = socket(AF_INET, SOCK_DGRAM, 0);

	if (server == SOCKET_ERROR)
	{
		cout << "�׽��ִ���" << WSAGetLastError() << endl;
		return 0;
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

	//�����Ϸ��Ͷ�
	cout << "�ȴ����Ͷ�����..." << endl;
	while (true)
	{
		char handshake[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(server, handshake, 2, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
		if (handshake[0] != '1')
			continue;
		cout << "���ն��յ���һ�����֡�" << endl;
		bool flag = true;
		while (true)
		{
			memset(handshake, 0, 2);
			char hsfeedback[2];
			hsfeedback[0] = '3';
			hsfeedback[1] = '2';
			sendto(server, hsfeedback, 2, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
			cout << "���ն˷��͵ڶ������֡�" << endl;
			while (recvfrom(server, handshake, 2, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
			if (handshake[0] == '1')
				continue;
			if (handshake[0] != '5' || handshake[1] != '4')
			{
				cout << "���Ӵ���\n���������Ͷˡ�" << endl;
				flag = false;
			}
			break;
		}
		if (!flag)
			continue;
		break;
	}
	cout << "�ɹ����ӵ����Ͷˣ�" << endl;
	//�����ļ���
	char name[100];
	int clientlen = sizeof(clientAddr);
	bool flag = true;
	int begin_time = clock();
	while (recvfrom(server, name, 100, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR)
	{
		int over_time = clock();
		if (over_time - begin_time > timeout)
		{
			flag = false;
			cout << "δ�յ��ļ�����" << endl;
			break;
		}
	}
	if (flag)
	{
		for (int i = 0; name[i] != '$'; i++)
		{
			filename += name[i];
		}
	}
	//�����ļ�
	printf("��ʼ�����ļ�����!\n");
	int curpkglen, curpkgno = 0, ackbefore = -1;
	while (true)
	{
		char receivepkg[pkglength];
		memset(receivepkg, '\0', sizeof receivepkg);
		while (true)
		{
			clientlen = sizeof(clientAddr);
			int begin_time = clock();
			bool flag = true;
			while (recvfrom(server, receivepkg, pkglength, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR)
			{
				int over_time = clock();
				if (over_time - begin_time > timeout)
				{
					flag = false;
					break;
				}
			}
			curpkgno = receivepkg[2] * 128 + receivepkg[3];
			curpkglen = receivepkg[4] * 128 + receivepkg[5];
			char feedback[3];
			memset(feedback, '\0', 3);
			// û�г�ʱ��ACK����ȷ���������ȷ
			unsigned int cks = checksum(receivepkg, curpkglen + pkgheaderlen);
			if (flag && cks == 0)
			{
				feedback[0] = '%';
				feedback[1] = receivepkg[2];
				feedback[2] = receivepkg[3];
				/*
				if (curpkgno == ackbefore + 1) {
					ackbefore++;
					feedback[1] = receivepkg[2];
					feedback[2] = receivepkg[3];
				}
				else {//������򣬽������ۻ�ȷ�ϵİ�
					cout << "����" << curpkgno << "�ŵ�ǰ����" << ackbefore << "��֮ǰȫ��ȷ��" << endl;
					feedback[1] = ackbefore / 128;
					feedback[2] = ackbefore % 128;
					curpkgno = ackbefore;
				}
				*/
				sendto(server, feedback, 3, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));

				printf("�ɹ����յ� %d �����ݰ�, ���ݶγ���Ϊ %d �ֽ�\n", curpkgno, curpkglen);
				break;
			}
			else
			{
				if (issendfinish(receivepkg))
				{
					cout << "�����ļ�������ϡ�" << endl;
					goto receivedone;
					break;
				}
				cout << "���ݰ��������� NAK �����Ͷˡ�У����: " << cks << endl;
				feedback[0] = '^';
				feedback[1] = receivepkg[2];
				feedback[2] = receivepkg[3];
				sendto(server, feedback, 3, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
				continue;
			}
		}
		// �յ��İ�����˳�򲻶ԣ���Ϊ���ڲ����֮����ش���
		// ��������index��ȷ������λ��
		int curpkgno = datalen * (receivepkg[2] * 128 + receivepkg[3]);
		pkgnolen[receivepkg[2] * 128 + receivepkg[3]] = curpkglen;
		for (int i = pkgheaderlen; i < curpkglen + pkgheaderlen; i++)
		{
			downloadfile[curpkgno + i - pkgheaderlen] = receivepkg[i];
		}
	}

receivedone:
	cout << "�ɹ������ļ�: " << filename << "!" << endl;
	int filelength = 0;
	int pkgnum;
	for (pkgnum = 0; pkgnolen[pkgnum] != '\0'; pkgnum++)
		filelength += pkgnolen[pkgnum];
	cout << "�ļ�����:" << filelength << "�ֽ�" << endl;
	cout << "���ݰ�����" << pkgnum << endl;
	ofstream fout(filename.c_str(), ofstream::binary);
	for (int i = 0; i < filelength; i++)
		fout << downloadfile[i];
	fout.close();
	//�ȴ����Ͷ˶Ͽ�����
	while (true)
	{
		char wavehand[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(server, wavehand, 2, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
		if (wavehand[0] != '7')
			continue;
		cout << "���ն��յ���һ�λ��֡�" << endl;
		char whfeedback[2];
		whfeedback[0] = '9';
		whfeedback[1] = '8';
		sendto(server, whfeedback, 2, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
		break;
	}
	cout << "���Ͷ˶Ͽ����ӡ�" << endl;
	closesocket(server);
	WSACleanup();
	return 0;
}