// ClientFTP.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "ClientFTP.h"
#include "afxsock.h"
#include<iostream>
#include <fstream>
#include<string> 
#include<list>
#include<thread>
#include<Windows.h>
#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SIZE 2048
// The one and only application object

CWinApp theApp;

using namespace std;


string tok1(const string& a)
{
	string result;
	int i = 0;
	while (a[i] != ' '&& i != a.length())
	{
		result.push_back(a[i++]);
	}
	return result;
}


string tok2(const string& a)
{
	string result;
	int i = 0;
	while (a[i] != ' ') i++;
	i++;
	while (a[i]) result.push_back(a[i++]);
	return result;
}



string ItoStr(const int& src)
{
	list<char> dest;
	int temp = src;
	while (temp > 0)
	{
		dest.push_front(temp % 10 + '0');
		temp /= 10;
	}
	string result(dest.begin(), dest.end());
	return result;
}

string IpSend(const string& src)
{
	string result = src;
	for (int i = 0; i < result.length(); i++)
	{
		if (result[i] == '.') result[i] = ',';
	}
	return result;
}

string getIpReceive(char* IpPo)
{
	string result;
	int count = 0;
	int i = 0;
	while (IpPo[i] != '(') i++;
	i++;
	while (1)
	{
		if (IpPo[i] == ',') count++;
		if (count == 4) return result;
		if(IpPo[i]==',')
		result.push_back('.');
		else result.push_back(IpPo[i]);
		i++;

	}

}


int str2Int(const string& str)
{
	int result=0;
	for (int i = 0; i < str.length(); i++)
	{
		result += (str[i] - '0')* pow(10, str.length() - i - 1);
	}
	return result;
}

int getPort(char* IpPo)
{
	int count = 0;
	int i = 0;
	while (IpPo[i] != '(') i++;
	while (count < 4) {
		if (IpPo[i++] == ',') count++;
	}
	int div, mod;
	string temp;
	while (IpPo[i] != ',')
	{
		temp.push_back(IpPo[i++]);
	}
	div = str2Int(temp);
	temp.clear();
	i++;
	while (IpPo[i] != ')')
	{
		temp.push_back(IpPo[i++]);
	}
	mod = str2Int(temp);
	int port = div * 256 + mod;
	return port;
}


void get(CSocket& data, char * rsp, string nameFile)
{
	//CSocket data;
	ofstream file{ nameFile, ofstream::binary };
//	connector.Accept(data);

	while (1)
	{
		int byteRec;
		memset(rsp, 0, SIZE);
		byteRec = data.Receive(rsp, SIZE - 1);
		if (byteRec == 0 || byteRec == SOCKET_ERROR)
		{

			file.close();
			break;
		}
		file.write(rsp, byteRec);

	}
	data.Close();
}




void put(CSocket& data, char* rsp, string nameFile)
{
	//CSocket data;
	FILE* file = fopen(nameFile.c_str(), "rb");
	if (file == NULL) {
		cout << "ERRROR OPEN FILE" << endl;
		return;
	}

	

//	connector.Accept(data);
	while (1)
	{
		memset(rsp, 0, SIZE);
		int byteRec = fread(rsp, 1, SIZE, file);
		if (byteRec == 0) {
			fclose(file);
			break;
		}
		data.Send(rsp, byteRec);
	}
	data.Close();
}

void List(CSocket& data, char * rsp)
{
	//CSocket data;
//	connector.Accept(data);
	while (1)
	{
		int byteRec;
		memset(rsp, 0, SIZE);
		byteRec = data.Receive(rsp, SIZE - 1);
		if (byteRec == 0 || byteRec == SOCKET_ERROR)
		{
			break;
		}

		cout << rsp;
	}
	data.Close();
}

void login(CSocket& client, string req, char* rsp)
{
	int x;
	memset(rsp, 0, SIZE);
	do {
		x = 0;// Đặt cờ để chạy vòng lặp khi nhập sai user và password
		cout << "User: ";
		getline(cin, req);
		req = "USER " + req + "\n";
		client.Send(req.c_str(), req.length());
		memset(rsp, 0, SIZE);
		client.Receive(rsp, SIZE - 1);
		cout << rsp;
		cout << "Password: ";
		getline(cin, req);
		req = "PASS " + req + "\n";
		client.Send(req.c_str(), req.length());
		memset(rsp, 0, SIZE);
		client.Receive(rsp, SIZE - 1);
		if (strstr(rsp, "230") == NULL) {
			cout << rsp;
			x = 1;
		}

	} while (x);
}


void cd(CSocket& client, string req, char* rsp)
{
	if (req == "cd") {
		string file;
		cout << "Remote directory ";
		getline(cin, file);
		req = "CWD " + file + "\n";
	}
	else {
		req = "CWD " + tok2(req) + "\n";
	}

	client.Send(req.c_str(), req.length());
	memset(rsp, 0, SIZE);
	client.Receive(rsp, SIZE);
	cout << rsp;
}

void quit(CSocket& client, string req, char* rsp)
{
	req = "QUIT\n";
	client.Send(req.c_str(), req.length());
	memset(rsp, 0, SIZE);
	client.Receive(rsp, SIZE - 1);
	cout << rsp;
}

int main(int argc,char* argv[])
{
	if (argc != 2) {
		cout << "ERROR" << endl;
		return 0;
	}
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			wprintf(L"Fatal Error: MFC initialization failed\n");
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			AfxSocketInit(NULL);

			CSocket client;
			string req;// request message
			char rsp[SIZE];// respond message
			bool active = TRUE;
			//char *c;
			if (!client.Create())
			{
				cout << "ERROR, DON'T CREATE SOCKET" << endl;
				return 1;
			}
			char c[100];
			CString x(argv[1]);
			if (!client.Connect(x, 21))
			{
				cout << "ERROR, DON'T CONCECT TO SERVER" << endl;
				return 1;
			}
			memset(rsp, 0, SIZE);
			client.Receive(rsp, SIZE - 1);
			if (strstr(rsp, "220") == NULL) {
				cout << "Server don't accept connect" << endl;
				return 1;
			}
			cout << rsp;
			cout << "connect success, please enter User and Password" << endl;
			login(client, req, rsp);
			cout << rsp;
			while (1)
			{

				getline(cin, req);// Nhận lệnh
				//15)	Thoát khỏi Server (quit, exit)
				if (req == "quit")
				{
					quit(client, req, rsp);
					break;
				}
				if (req == "dir" || req == "ls" || tok1(req) == "get" || tok1(req) == "put" || tok1(req) == "mget" || tok1(req) == "mput")
				{
					string temp = req;
					if (active)//Chế độ active
					{
						CSocket  data;
						CSocket connector;
						if (!connector.Create()) {
							cout << "Don't create soket in active mode" << endl;
							break;
						}
						connector.Listen();
						UINT port;
						CString host;
						connector.GetSockName(host, port);// Lấy host và port của connector
						string div = ItoStr(int(port) / 256);//Chuyển dạng
						string mod = ItoStr(port % 256);
						string ip = IpSend("127.0.0.1");
						req = "PORT " + ip + "," + div + "," + mod + "\n";
						client.Send(req.c_str(), req.length());
						memset(rsp, 0, SIZE);
						client.Receive(rsp, SIZE);
						cout << rsp;
						if (temp == "dir" || temp == "ls")//Liệt kê danh sách file và folder
						{
							if (temp == "dir")	req = "LIST\n";
							else req = "NLST\n";
							client.Send(req.c_str(), req.length());
							connector.Accept(data);
							List(data, rsp);
						}
						else if (tok1(temp) == "get")// tải 1 file xuống
						{
							if (temp == "get") {
								string file;
								cout << "Remote directory ";
								getline(cin, file);
								req = "RETR " + file + "\n";
							}
							else
								req = "RETR " + tok2(temp) + "\n";
							client.Send(req.c_str(), req.length());
							connector.Accept(data);
							get(data, rsp, tok2(temp));

						}
						else if (tok1(temp) == "put")// tải 1 file xuống
						{
							if (temp == "put") {
								string file;
								cout << "Remote directory ";
								getline(cin, file);
								req = "STOR " + file + "\n";
							}
							else
								req = "STOR " + tok2(temp) + "\n";
							client.Send(req.c_str(), req.length());
							connector.Accept(data);
							put(data, rsp, tok2(temp));

						}


						cout << rsp;
						memset(rsp, 0, SIZE);
						client.Receive(rsp, SIZE);
						cout << rsp;

						connector.Close();


					}
					else {//Passive mode
						CSocket  data;
						if (!data.Create()) {
							cout << "Don't create soket in passive mode" << endl;
							break;
						}
						req = "PASV\n";
						client.Send(req.c_str(), req.length());
						memset(rsp, 0, SIZE);
						client.Receive(rsp, SIZE - 1);
						//cout << rsp;
						CString ip(getIpReceive(rsp).c_str());
						int port = getPort(rsp);
						if (!data.Connect(ip, port)) {
							cout << "Dont connect server in passive mode" << endl;
							break;
						}
						if (temp == "dir" || temp == "ls")//Liệt kê danh sách file và folder
						{
							if (temp == "dir")	req = "LIST\n";
							else req = "NLST\n";
							client.Send(req.c_str(), req.length());
							cout << rsp;
							List(data, rsp);
						}



						memset(rsp, 0, SIZE);
						client.Receive(rsp, SIZE);
						cout << rsp;



					}
					

				}

				if (req == "pass") {
					active = 0;
					cout << "227 Entering passive mode" << endl;
					continue;
				}

				//Thay đổi đường dẫn trên Server 

				if (tok1(req) == "cd")
				{
					cd(client, req, rsp);
					continue;
				}
				//Thay đổi đường dẫn dưới client (lcd)
				if (tok1(req) == "lcd")
				{
					if (req == "lcd") {
						string file;
						cout << "Remote directory ";
						getline(cin, file);
						req = file;
					}
					else {
						req =  tok2(req) ;
					}
					CString path(req.c_str());
					if (!SetCurrentDirectoryW(path))
					{
						cout << "Error set current directory" << endl;
					}
					else cout << "Success changing current directory" << endl;
					
					continue;
				}
			}






			client.Close();

		}
	}
	else
	{
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}

	return nRetCode;
}
