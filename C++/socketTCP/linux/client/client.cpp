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
  std::unique_ptr<I1 []> file_buf;
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
      return;
    }

    u4_fileLen = fin.tellg();      // set fileLen
    cout << "file length = "
         << u4_fileLen << endl;

    fin.seekg(0,ios_base::beg); // return to the beginning of the file

    file_buf = std::unique_ptr<I1 []> (new (std::nothrow)I1[u4_fileLen]);
    if(nullptr == file_buf.get())
    {
      std::cout << "the file is too large!\n";
      return;
    }

    fin.read(file_buf.get(),u4_fileLen);
    fin.close();
  }

  sendData(file_buf.get(),u4_fileLen);
}

void socketTCPClient::sendData(const I1* data,const U4 &u4_dataSize)
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

void socketTCPClient::sendFixedStruct(const FIXED_LENGTH_STRUCT &fixedStruct)
{
  std::unique_ptr<I1 []> buffer(new (std::nothrow)I1[sizeof(FIXED_LENGTH_STRUCT)]);
  if(nullptr == buffer.get())
  {
    std::cout << "new buffer error!\n";
    return;
  }

  memcpy(buffer.get(),&fixedStruct,sizeof(fixedStruct));

  sendData(buffer.get(),sizeof(FIXED_LENGTH_STRUCT));
}

void socketTCPClient::sendMutableStruct(const MUTABLE_LENGTH_STRUCT &mutableStruct)
{
  U4 u4_bufferSize =
    sizeof(I4) +
    sizeof(U2) +
    sizeof(U2) + mutableStruct.str.size();

  for_each(
    mutableStruct.vector_strList.begin(),
    mutableStruct.vector_strList.end(),
    [&u4_bufferSize](std::string str)
    {
      u4_bufferSize += (sizeof(U2) + str.size());
    }
  );

  std::cout << "buffer size = "
            << u4_bufferSize
            << std::endl;

  std::unique_ptr<I1 []> buffer(new (std::nothrow)I1[u4_bufferSize]);
  if(nullptr == buffer.get())
  {
    std::cout << "new buffer error!\n";
    return;
  }

  U4 u4_offset = 0;

  memcpy(buffer.get() + u4_offset,&(mutableStruct.i4),sizeof(I4));
  u4_offset += sizeof(I4);

  memcpy(buffer.get() + u4_offset,&(mutableStruct.u2),sizeof(U2));
  u4_offset += sizeof(U2);

  u4_offset += arrangeString(buffer.get() + u4_offset,mutableStruct.str);

  for_each(
    mutableStruct.vector_strList.begin(),
    mutableStruct.vector_strList.end(),
    [&](std::string str)
    {
      u4_offset += arrangeString(buffer.get() + u4_offset,str);
    }
  );

  std::cout << "offset = "
            << u4_offset
            << std::endl;

  if(u4_offset != u4_bufferSize)
  {
    std::cout << "something error! please check!\n";
    return;
  }

  sendData(buffer.get(),u4_bufferSize);
}

U4 socketTCPClient::arrangeString(I1 *buffer,const std::string &str)
{
  U4 u4_offset = 0;
  U2 u2_strSize = str.size();

  memcpy(buffer,&u2_strSize,sizeof(u2_strSize));
  u4_offset += sizeof(u2_strSize);

  memcpy(buffer + u4_offset,str.c_str(),u2_strSize);
  u4_offset += u2_strSize;

  return u4_offset;
}

void socketTCPClient::quit()
{
  isRun = false;
  isConnected = false;
  close(sockClient);
}
