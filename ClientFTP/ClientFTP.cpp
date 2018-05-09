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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SIZE 2048
// The one and only application object

CWinApp theApp;

using namespace std;


string tok1(const string& a)
{
	string results;
	int i = 0;
	while (a[i] != ' '&& i != a.length())
	{
		results.push_back(a[i++]);
	}
	return results;
}


string tok2(const string& a)
{
	string results;
	int i = 0;
	while (a[i] != ' ') i++;
	i++;
	while (a[i]) results.push_back(a[i++]);
	return results;
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

string Ip(const string& src)
{
	string result = src;
	for (int i = 0; i < result.length(); i++)
	{
		if (result[i] == '.') result[i] = ',';
	}
	return result;
}




void get(CSocket& connector, char * rsp, string nameFile)
{
	CSocket data;
	ofstream file{ nameFile, ofstream::binary };
	connector.Accept(data);

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

void List(CSocket& connector, char * rsp)
{
	CSocket data;
	connector.Accept(data);
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

int main()
{
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
			if (!client.Connect(_T("127.0.0.1"), 21))
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

						CSocket connector;
						if (!connector.Create()) {
							cout << "Don't create soket in active mode" << endl;
							break;
						}
						connector.Listen();
						UINT port;
						CString host;
						connector.GetSockName(host, port);// Lấy host và port của server
						string div = ItoStr(int(port) / 256);//Chuyển dạng
						string mod = ItoStr(port % 256);
						string ip = Ip("127.0.0.1");
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
							List(connector, rsp);
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
							get(connector, rsp, tok2(temp));

						}


						cout << rsp;
						memset(rsp, 0, SIZE);
						client.Receive(rsp, SIZE);
						cout << rsp;

						connector.Close();


					}
					else {
						
					}
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
						req = "cd " + file + "\n";
					}
					else {
						req = "cd" + tok2(req) + "\n";
					}

					//	client.Send(req.c_str(), req.length());
						//memset(rsp, 0, SIZE);
						//client.Receive(rsp, SIZE);
					//	cout << rsp;
					system(req.c_str());
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
