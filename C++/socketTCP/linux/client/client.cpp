#include "client.h"

bool socketTCPClient::connectToServer()
{
  sockClient = socket(AF_INET,SOCK_STREAM,0);
  if(-1 == sockClient)
  {
    perror("socket");
    return false;
  }

  struct sockaddr_in addrSrv;
  memset(&addrSrv,0,sizeof(addrSrv));
  addrSrv.sin_addr.s_addr = inet_addr(SERVER_ADDR);
  addrSrv.sin_family = AF_INET;
  addrSrv.sin_port = htons(SOCKET_TCP_PORT);

  try
  {
    if(isRun)
    {
      while(!isConnected)
      {
        if(-1 == connect(sockClient,(struct sockaddr*)&addrSrv,sizeof(addrSrv)))
        {
          perror("connect");
          close(sockClient);
          return false;
        }
        else
        {
          isConnected = true;
        }
      }
    }
  }catch(...)
  {
    perror("error");
    close(sockClient);
    return false;
  }

  return true;
}

void socketTCPClient::sendFile(const I1 *path)
{
  std::cout << "sendFile() called!\n";

  U4 u4_fileLen = 0;
  I1 *file_buf;
  {
    using namespace std;

    ifstream fin(path,ios_base::in | ios_base::binary);
    if(!fin.is_open())
    {
      perror("ifstream open");
      return;
    }

    fin.seekg(0,ios_base::end); // set fin to the end of the file
    if(UINT_MAX < fin.tellg())
    {
      std::cout << "the file is too large!\n";
    }

    u4_fileLen = fin.tellg();      // set fileLen
    cout << "file length = "
         << u4_fileLen << endl;

    fin.seekg(0,ios_base::beg); // return to the beginning of the file
    file_buf = new char[u4_fileLen];
    fin.read(file_buf,u4_fileLen);
    fin.close();
  }

  sendData(file_buf,u4_fileLen);

  delete file_buf;
}

void socketTCPClient::sendData(const I1* data,U4 u4_dataSize)
{
  std::cout << "sendData() called!\n";

  isConnected = false;
  if(!connectToServer())
  {
    std::cout << "connect to server failed!\n";
    return;
  }

  char buffer[10] = { "verify" };
  memcpy(buffer + 6, &u4_dataSize, sizeof(u4_dataSize));

  try
  {
    if(-1 == send(sockClient,buffer,sizeof(buffer),0))
    {
      perror("send");
      close(sockClient);
      return;
    }

    memset(buffer,0,sizeof(buffer));
    if(-1 == recv(sockClient,buffer,3,0))
    {
      perror("recv");
      close(sockClient);
      return;
    }
    else if(0 == strncmp(buffer,"OK",2))
    {
      if(-1 == send(sockClient,data,u4_dataSize,0))
      {
        perror("send");
        close(sockClient);
        return;
      }
    }

  }catch(...)
  {
    perror("error");
    return;
  }
}

void socketTCPClient::quit()
{
  isRun = false;
  isConnected = false;
  close(sockClient);
}
