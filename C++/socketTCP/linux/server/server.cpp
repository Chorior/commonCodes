#include "server.h"

void socketTCPServer::run()
{
	std::cout << "server start\n\n";

	sockSrv = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockSrv)
	{
		std::cout << "socket failed!\n";
		return;
	}

	struct sockaddr_in addrSrv;
	memset(&addrSrv,0,sizeof(addrSrv));
	addrSrv.sin_addr.s_addr = htonl(BIND_ADDR);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SOCKET_TCP_PORT);

	//http://www.cnblogs.com/qq78292959/archive/2012/03/22/2411390.html
	// reuse local address and port
	I4 on = 1;
	if(-1 == setsockopt(sockSrv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))
	{
		perror("bind");
		close(sockSrv);
		return;
	}

	if(-1 == bind(sockSrv,(struct sockaddr *)&addrSrv,sizeof(struct sockaddr_in)))
	{
		perror("bind");
		close(sockSrv);
		return;
	}

	if(-1 == listen(sockSrv,5))
	{
		perror("listen");
		close(sockSrv);
		return;
	}

	struct sockaddr_in addrClient;
	socklen_t clientAddrLen = 1;

	while(isRun)
	{
		if(!isConnected)
		{
			sockConn = accept(sockSrv,(struct sockaddr *)&addrClient,&clientAddrLen);
			if(-1 == sockConn)
			{
				perror("accept");
				close(sockSrv);
				return;
			}

			isConnected = true;
		}

		// receive data from socket and store it in recv_buf
		try
		{
			std::cout << "receive\n";

			char recv_sizeBuf[10] = { 0 };
			U4 u4_recv = recv(sockConn,recv_sizeBuf,10,0);
			if(u4_recv > 0)
			{
				if(0 == strncmp(recv_sizeBuf,"verify",6))
				{
					U4 u4_dataSize = 0;
					memcpy(&u4_dataSize,recv_sizeBuf + 6, 4);
					std::cout << "data size = "
										<< u4_dataSize
										<< std::endl;

					I1 buffer[] = { "OK" };
					if(-1 == send(sockConn,buffer,sizeof(buffer),0))
					{
						perror("send OK");
						quit();
						return;
					}

					U4 offset = 0;
					U4 u4_dataSizeTmp = u4_dataSize;

					I1 *recv_buf = new (std::nothrow)I1[u4_dataSize];
					if(nullptr == recv_buf)
					{
						std::cout << "the size of data is too large!\n";
						quit();
						return;
					}

					u4_recv = recv(sockConn,recv_buf,u4_dataSize,0);
					while(u4_dataSize > u4_recv)
					{
						if(-1 == u4_recv)
						{
							perror("recv");
							quit();
							return;
						}
						else
						{
							offset += u4_recv;
							u4_dataSize -= u4_recv;
							u4_recv = recv(sockConn,recv_buf + offset,u4_dataSize,0);
							std::cout << "whole message = \n"
												<< recv_buf
												<< std::endl;
						}
					}

					//save data
					isConnected = false;
					//saveData(recv_buf,u4_dataSizeTmp);
					//saveFixedStruct(recv_buf,u4_dataSizeTmp);
					saveMutableStruct(recv_buf,u4_dataSizeTmp);

					delete recv_buf;
				}
				else
				{
					std::cout << "received wrong data!\n";
					quit();
					return;
				}
			}
			else
			{
				perror("recv");
				quit();
				return;
			}
		}catch(...)
		{
			perror("error");
			quit();
			return;
		}
	}
}

void socketTCPServer::saveData(const I1 *data, const U4 &u4_dataSize)
{
	std::cout << "saveData() called!\n";

	using namespace std;
	ofstream fout(FILE_PATH,ios_base::out | ios_base::binary);
	if(!fout.is_open())
	{
		perror("ofstream open");
		return;
	}

	fout.write(data,u4_dataSize);
	fout.close();
}

void socketTCPServer::saveFixedStruct(const I1 *data,const U4 &u4_dataSize)
{
	FIXED_LENGTH_STRUCT structFixed;
	memcpy(&structFixed,data,u4_dataSize);

	{
		std::cout << "structFixed.f8 = " << structFixed.f8 << std::endl;
		std::cout << "structFixed.f4 = " << structFixed.f4 << std::endl;
		std::cout << "structFixed.i4 = " << structFixed.i4 << std::endl;
		std::cout << "structFixed.u4 = " << structFixed.u4 << std::endl;
		std::cout << "structFixed.i2 = " << structFixed.i2 << std::endl;
		std::cout << "structFixed.u2 = " << structFixed.u2 << std::endl;
		std::cout << "structFixed.i1 = " << structFixed.i1 << std::endl;
		std::cout << "structFixed.u1 = " << structFixed.u1 << std::endl;
	}

}

void socketTCPServer::saveMutableStruct(const I1 *data,const U4 &u4_dataSize)
{
	MUTABLE_LENGTH_STRUCT structMutable;
	U2 u2_strSize = 0;
	U4 u4_offset = 0;

	memcpy(&(structMutable.i4),data + u4_offset,sizeof(I4));
	u4_offset += sizeof(I4);

	memcpy(&(structMutable.u2),data + u4_offset,sizeof(U2));
	u4_offset += sizeof(U2);

	{
		memcpy(&u2_strSize,data + u4_offset,sizeof(U2));
		u4_offset += sizeof(U2);

		I1 *array_str = new (std::nothrow)I1[u2_strSize];
		if(nullptr == array_str)
		{
			std::cout << "new array_str error!\n";
			return;
		}

		memcpy(array_str,data + u4_offset,u2_strSize);
		u4_offset += u2_strSize;

		structMutable.str.insert(0,array_str);

		delete array_str;
	}

	while(u4_offset < u4_dataSize)
	{
		std::string str_tmp = "";

		memcpy(&u2_strSize,data + u4_offset,sizeof(U2));
		u4_offset += sizeof(U2);

		I1 *array_str = new (std::nothrow)I1[u2_strSize];
		if(nullptr == array_str)
		{
			std::cout << "new array_str error!\n";
			return;
		}

		memcpy(array_str,data + u4_offset,u2_strSize);
		u4_offset += u2_strSize;

		str_tmp.insert(0,array_str);
		structMutable.vector_strList.push_back(str_tmp);

		delete array_str;
	}

	{
		std::cout << "structMutable.i4 = "
							<< structMutable.i4 << std::endl;

		std::cout << "structMutable.u2 = "
							<< structMutable.u2 << std::endl;

		std::cout << "structMutable.str = "
							<< structMutable.str << std::endl;

		for_each(
			structMutable.vector_strList.begin(),
			structMutable.vector_strList.end(),
			[](std::string str)
			{
				std::cout << "structMutable.vector_strList.str = "
									<< str << std::endl;
			}
		);
	}
}

void socketTCPServer::quit()
{
	isRun = false;
	isConnected = false;
	close(sockConn);
	close(sockSrv);
}
