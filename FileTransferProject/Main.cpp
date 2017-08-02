#include<iostream>
#include<fstream>
#include<string>
#include<conio.h>
#include<Windows.h>
#include<iomanip>
#include<ctime>
#include<wininet.h>

#define BUF_SIZE 8192

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "wininet")

using namespace std;
using std::cout;

enum SizeType
{
	Kilo, Mega
};

struct CLIENT
{
	SOCKET sock;
	SOCKADDR_IN addr;
	int addrSize;
};

int getInt(int value); //정수의 자릿수 리턴
void updatelog();

int main(void)
{
	std::clock_t before;

	string sendport;
	string fileName;
	string ipinput;
	string portinput;

	WSADATA wsadata;
	ifstream fin;
	ofstream fout;

	SOCKET servSock;
	SOCKADDR_IN servAddr;
	CLIENT clnt;
	int fileSize, bufCount;
	char buf[BUF_SIZE];
	string bufNumStr;
	double sendTime;
	double sendSpeed;
	SizeType sendSize;

	SOCKET clntSock;
	SOCKADDR_IN servAddrRecv;
	char bufR[BUF_SIZE];
	int bufszR;
	string fileNameRecv;
	int bufCountRecv;

	cout << fixed;

MainMenu:

	int choose;
	cout << "=================================" << endl;
	cout << "       《 File Transfer 》       " << endl << endl;
	cout << "                         Ver 0.3a" << endl;
	cout << " # 1. File Send" << endl;
	cout << " # 2. File Recieve" << endl;
	cout << " # 3. Update Note" << endl;
	cout << " # 0. Exit" << endl;
	cout << "=================================" << endl;
	cout << " >> Choose : ";
	cin >> choose;

	switch (choose)
	{
	case 0:
		return 0;
	case 1:
		cout << "=================================" << endl;
		cout << " >> File Name : ";
		cin >> fileName;
		cout << " >> PORT : ";
		cin >> sendport;
		goto FileSend;
		break;
	case 2:
		cout << "=================================" << endl;
		cout << " >> IP Addr : ";
		cin >> ipinput;
		cout << " >> PORT : ";
		cin >> portinput;
		goto FileRecv;
		break;
	case 3:
		cout << "=================================" << endl;
		updatelog();
		cout << endl;
		cout << "[SYSTEM] Press any key." << endl;
		_getch();
		system("cls");
		goto MainMenu;
		break;
	default:
		cout << "[SYSTEM] Error. Press any key." << endl;
		_getch();
		system("cls");
		goto MainMenu;
		break;
	}


FileSend:

	fin.open(fileName.c_str(), ios::binary);

	if (fin.is_open() == false)
	{
		cout << "[SYSTEM] No File!" << endl;
		cout << "[SYSTEM] Press Any Key." << endl;
		_getch();
		system("cls");
		goto MainMenu;
	}

	try
	{

		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
			throw "WSAStartup Error.";

		servSock = socket(PF_INET, SOCK_STREAM, 0);

		if (servSock == INVALID_SOCKET)
			throw "Server Socket Error.";

		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servAddr.sin_port = htons(atoi(sendport.c_str()));

		bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr));

		cout << "[SYSTEM] Server Opened." << endl;
		cout << "[SYSTEM] Port : " << sendport << endl;

		cout << "[SYSTEM] Listening.." << endl;

		if (listen(servSock, 5) == SOCKET_ERROR)
			throw "Listen Error.";

		cout << "[SYSTEM] Accepting.." << endl;
		clnt.addrSize = sizeof(clnt.addr);
		clnt.sock = accept(servSock, (sockaddr*)&clnt.addr, &clnt.addrSize);
		if (clnt.sock == INVALID_SOCKET)
			throw "Accept Error.";

		// 연결 후 첫 번째 전송 : 파일 이름 전송.
		send(clnt.sock, fileName.c_str(), fileName.length(), 0);
	}
	catch (char* msg)
	{
		cout << "[SYSTEM] " << msg << endl;
	}

	/************************/

	fin.seekg(0, ios::end);
	fileSize = fin.tellg();
	fin.seekg(0, ios::beg);

	bufCount = (int)(fileSize / BUF_SIZE) + 1;

	int cnt;
	int tempint;
	double percent = 0;
	double percentTemp;
	bufNumStr = to_string(bufCount);
	cout << "[SYSTEM] File Size : " << fileSize << " bytes." << endl;
	cout << "[SYSTEM] Buffer Size : " << BUF_SIZE << " bytes." << endl;
	cout << "[SYSTEM] Pieces Number : " << bufCount << endl;

	// 연결 후 두 번째 전송 : 조각 개수 전송.
	send(clnt.sock, bufNumStr.c_str(), getInt(bufCount), 0);

	before = clock();

	percentTemp = percent;
	for (int i = 0; i < bufCount; i++)
	{
		if (i == bufCount - 1)
			tempint = fileSize%BUF_SIZE;
		else
			tempint = BUF_SIZE;
		percent = ((i + 1)*100) / bufCount;
		if(percentTemp != percent)
			cout << "[SYSTEM] " << setprecision(0) << percent << "% Sent. (" << i + 1 << "/" << bufCount << ")" << endl;
		percentTemp = percent;
		for (cnt = 0; cnt < tempint; cnt++)
			buf[cnt] = fin.get();
		if (tempint < BUF_SIZE)
			buf[tempint] = '\0';

		send(clnt.sock, buf, cnt, 0);
		memset(&buf, 0, sizeof(buf));
	}

	send(clnt.sock, "", 0, 0);
	cout << "[SYSTEM] File Transfer Finished." << endl;

	sendTime = (double)(clock() - before) / CLOCKS_PER_SEC;

	sendSpeed = (double)(fileSize / 1024) / sendTime;
	sendSize = SizeType::Kilo;

	if (sendSpeed > 1024)
	{
		sendSpeed = sendSpeed / 1024.0;
		sendSize = SizeType::Mega;
	}

	cout << "[SYSTEM] Time : " << setprecision(2) << sendTime << " sec"<< endl;
	cout << "[SYSTEM] Transfer Speed : " << setprecision(2) << sendSpeed;
	if (sendSize == SizeType::Kilo)
		cout << "KB/s" << endl;
	else if (sendSize == SizeType::Mega)
		cout << "MB/s" << endl;

	fin.close();

	closesocket(clnt.sock);
	closesocket(servSock);

	WSACleanup();

	cout << "[SYSTEM] Press Any Key." << endl;
	_getch();
	system("cls");
	goto MainMenu;

FileRecv:

	int cnttmp = 0;
	double percenttmp;
	double percenttmptmp;

	try
	{
		cout << "[SYSTEM] WSAStartup." << endl;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
			throw "WSAStartup Error.";

		cout << "[SYSTEM] Generating Socket." << endl;
		clntSock = socket(PF_INET, SOCK_STREAM, 0);
		if (clntSock == INVALID_SOCKET)
			throw "Socket Error.";

		cout << "ip : " << ipinput << endl;
		cout << "port : " << portinput << endl;
		memset(&servAddrRecv, 0, sizeof(servAddrRecv));
		servAddrRecv.sin_family = AF_INET;
		servAddrRecv.sin_addr.s_addr = inet_addr(ipinput.c_str());
		servAddrRecv.sin_port = htons(atoi(portinput.c_str()));

		cout << "[SYSTEM] Connecting." << endl;
		if (connect(clntSock, (SOCKADDR*)&servAddrRecv, sizeof(servAddrRecv)) == SOCKET_ERROR)
			throw "Connect Error.";

		cout << "[SYSTEM] Connected!" << endl;

		bufszR = recv(clntSock, bufR, sizeof(bufR), 0);
		if (bufszR != BUF_SIZE)
			bufR[bufszR] = '\0';
		fileNameRecv = bufR;
		cout << "[SYSTEM] File Name : " << fileNameRecv << endl;

		bufszR = recv(clntSock, bufR, sizeof(bufR), 0);
		bufR[bufszR] = '\0';
		bufCountRecv = atoi(bufR);

		fout.open(fileNameRecv, ios::binary);
		cout << "[SYSTEM] File Opened." << endl;
		memset(&bufR, 0, sizeof(bufR));
		percenttmptmp = percenttmp;
		while (true)
		{
			percenttmp = ((cnttmp + 1)*100 / bufCountRecv);
			bufszR = recv(clntSock, bufR, sizeof(bufR), 0);
			if (bufszR == -1 || bufszR == 0)
				break;
			if (bufszR != BUF_SIZE)
				bufR[bufszR] = '\0';
			fout.write(bufR, bufszR);
			if (percenttmptmp != percenttmp)
				cout << "[SYSTEM] " << setprecision(0) << percenttmp << "% Received. (" << cnttmp + 1 << "/" << bufCountRecv << ")" << endl;
			percenttmptmp = percenttmp;
			memset(bufR, 0, sizeof(bufR));
			cnttmp++;
		}
		fout.close();
		cout << "[SYSTEM] File Recieve Finished." << endl;
	}
	catch (char* msg)
	{
		cout << "[SYSYEM] " << msg << endl;
	}

	closesocket(clntSock);
	WSACleanup();

	cout << "[SYSTEM] Press Any Key." << endl;
	_getch();
	system("cls");
	goto MainMenu;
}

int getInt(int value)
{
	int count = 0;
	do
	{
		value = int(value / 10);
		count++;
	} while (value > 0);
	return count;
}

void updatelog()
{
	/*
		Insert More Version Log Here
	*/
	cout << " # Ver 0.3a" << endl;
	cout << "  - Increased Buffer Size. (8192)" << endl;
	cout << "  - Print Transfer Speed by MB/s." << endl;
	cout << "  - Fixed Receive Percentage Bug." << endl;
	Sleep(200);
	cout << " # Ver 0.2a" << endl;
	cout << "  - Increased Buffer Size. (1024)" << endl;
	cout << "  - Fixed Printing Progress." << endl;
	Sleep(200);
	cout << " # Ver 0.1a" << endl;
	cout << "  - Increased Buffer Size. (512)" << endl;
	Sleep(200);
}