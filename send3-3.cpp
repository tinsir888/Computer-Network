#include "reliableudp.h"
SOCKET client;
char uploadfile[maxn];
int filelen = 0;
int totpkg;
int curwindow = 0;
const int maxwindowsize = 6;//��󴰿ڳ���
int acknum = 0;
void sendpkg(char* pkgdata, int curpkgdatalen, int curpkgno, int last)
{
	char *sendbuffer = new char[curpkgdatalen + pkgheaderlen];
	if (last)
		sendbuffer[1] = '$';//���һ�����ݰ�
	else sendbuffer[1] = '@';//�������һ�����ݰ�
	sendbuffer[2] = curpkgno / 128;
	sendbuffer[3] = curpkgno % 128;
	sendbuffer[4] = curpkgdatalen / 128;
	sendbuffer[5] = curpkgdatalen % 128;
	for (int i = 0; i < curpkgdatalen; i++)
		sendbuffer[i + pkgheaderlen] = pkgdata[i];
	sendbuffer[0] = checksum(sendbuffer + 1, curpkgdatalen + pkgheaderlen - 1);
	sendto(client, sendbuffer, curpkgdatalen + pkgheaderlen, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
}
int main()
{
	//�������Ͷ�
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup ����: " << WSAGetLastError() << endl;
		return 0;
	}
	client = socket(AF_INET, SOCK_DGRAM, 0);
	if (client == SOCKET_ERROR)
	{
		cout << "�׽��ִ���: " << WSAGetLastError() << endl;
		return 0;
	}
	int Port = 1439;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Port);

	cout << "�ɹ��������Ͷ�!" << endl;
	cout << "���ӵ����ն�..." << endl;
	//���ӵ����ն�
	while (true)
	{
		char handshake[2];
		handshake[0] = '1';
		handshake[1] = '#';
		sendto(client, handshake, 2, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		cout << "���Ͷ������һ�����֡�" << endl;
		int begtimer = clock();
		bool flag = true;
		char hsfeedback[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(client, hsfeedback, 2, 0, (sockaddr*)&serverAddr, &clientlen) == SOCKET_ERROR)
		{
			int overtimer = clock();
			if (overtimer - begtimer > timeout)
			{
				flag = false;
				break;
			}
		}
		if (flag && hsfeedback[0] == '3' && hsfeedback[1] == '2')
		{
			cout << "���Ͷ��յ��ڶ������֡�" << endl;
			handshake[0] = '5';
			handshake[1] = '4';
			sendto(client, handshake, 2, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
			cout << "���Ͷ˷��͵��������֡�" << endl;
			break;
		}
	}
	cout << "�ɹ����ӵ����նˣ�" << endl;
	int time_out = 50;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&time_out, sizeof(time_out));
	string filename;
	cin >> filename;
	//�����ļ���
	char* sendfilename = new char[filename.length() + 1];
	for (int i = 0; i < filename.length(); i++)
	{
		sendfilename[i] = filename[i];
	}
	sendfilename[filename.length()] = '$';
	sendto(client, sendfilename, filename.length() + 1, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	delete sendfilename;
	//���ļ����浽���ͻ�����
	ifstream fin(filename.c_str(), ifstream::binary);
	if (!fin)
	{
		cout << "�ļ�������! " << endl;
		return 0;
	}
	unsigned char ch = fin.get();
	while (fin)
	{
		uploadfile[filelen] = ch;
		filelen++;
		ch = fin.get();
	}
	fin.close();
	totpkg = filelen / datalen + (filelen % datalen != 0);
	cout << "�ļ������ݰ�������" << totpkg << endl;
	//��ʼ�����ļ�
	cout << "��ʼ���ͣ�" << endl;
	int begin_send_time = clock(), finish_send_time;
	bool status = 0;// 0 ��ʾ������״̬�� 1 ��ʾӵ��״̬
	int basepkgno = 0;
	while (true)
	{
		if (!curwindow){
			curwindow = 1;
		}
		else if (curwindow < maxwindowsize && status == 0)
		{
			if ((curwindow << 1) > maxwindowsize){
				status = 1;
				curwindow++;
			}
			else curwindow <<= 1;
			
		}
		else if (status == 1)
			curwindow++;
		printf("��ǰ���ڴ�С: %d\n", curwindow);
		int i;
		for (i = 0; i < curwindow && i + basepkgno < totpkg; i++)
		{
			int curpkgno = i + basepkgno;
			if(curpkgno == totpkg - 1)
				sendpkg(uploadfile + curpkgno * datalen, 
					filelen - (totpkg - 1)* datalen, curpkgno, true);
			else sendpkg(uploadfile + curpkgno * datalen, datalen, curpkgno, false);
		}
		printf("�ѷ��͵� %d �����ݰ�\n", i + basepkgno - 1);
		int begintimer = clock(), endtimer;
		//���ն˵ķ���
		while (true)
		{
			int clientlen = sizeof(clientAddr);
			bool flag = true;
			char feedback[3];
			while (recvfrom(client, feedback, 3, 0, (sockaddr*)&serverAddr, &clientlen) == SOCKET_ERROR)
			{
				endtimer = clock();
				if (endtimer - begintimer > timeout)
				{
					flag = false;
					//printf("��ʱ��\n");
					break;
				}
			}
			if (flag && feedback[0] == '%')
			{
				printf("�յ����նˣ��� %d �����ݰ� ACK��\n", feedback[1] * 128 + feedback[2]);
				acknum++;
			}
			else if (!flag){
				break;
			}
		}
		if (acknum == curwindow)
		{
			acknum = 0;
			basepkgno += curwindow;
		}
		else
		{
			//basepkgno = feedback;
			acknum = 0;
			curwindow = 0;
			status = 0;
			if (i + basepkgno >= totpkg)
				break;
		}
	}
	//��֪�ͻ��˷����ļ�����
	char finishsend[3] = { 'f', 'i', 'n' };
	sendto(client, finishsend, 3, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	finish_send_time = clock();
	int tot_time = finish_send_time - begin_send_time;
	printf("����ʱ�䣺%dms\n", tot_time);
	printf("�����ʣ�%.3fKpbs\n", (double)filelen * 8 / tot_time / 1024);
	cout << "�ɹ������ļ�" << filename << "!" << endl;
	//���֣��Ͽ�����
	while (true)
	{
		char wavehand[2];
		wavehand[0] = '7';
		wavehand[1] = '#';
		sendto(client, wavehand, 2, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		cout << "���Ͷ˻�һ���֡�" << endl;
		int begtimer = clock();
		bool flag = true;
		char whfeedback[2];
		int clientlen = sizeof(clientAddr);
		while (recvfrom(client, whfeedback, 2, 0, (sockaddr*)&serverAddr, &clientlen) == SOCKET_ERROR)
		{
			int overtimer = clock();
			if (overtimer - begtimer > timeout)
			{
				flag = false;
				break;
			}
		}
		if (flag && whfeedback[0] == '9' && whfeedback[1] == '8')
			break;
	}
	cout << "�ѶϿ�����" << endl;
	closesocket(client);
	WSACleanup();
	return 0;
}