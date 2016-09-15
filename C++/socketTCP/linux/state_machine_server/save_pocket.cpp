#include "save_pocket.h"

void save_pocket::run()
{
	try
	{
		for(;;)
		{
			incoming.wait()
				.handle<file_received>(
					[&](file_received const& msg)
					{
						std::lock_guard<std::mutex> lk(iom);
						std::cout << "received a file\n";
						saveFile(msg.str_file.c_str(),msg.str_file.size());
					}
					)
				.handle<fixed_struct_received>(
					[&](fixed_struct_received const& msg)
					{
						std::lock_guard<std::mutex> lk(iom);
						std::cout << "received a fixed_struct\n";
						saveFixedStruct(msg.str_fixed_struct.c_str(),msg.str_fixed_struct.size());
					}
					)
				.handle<mutable_struct_received>(
					[&](mutable_struct_received const& msg)
					{
						std::lock_guard<std::mutex> lk(iom);
						std::cout << "received a mutable_struct\n";
						saveMutableStruct(msg.str_mutable_struct.c_str(),msg.str_mutable_struct.size());
					}
					);
		}
	}
	catch(messaging::close_queue const&)
	{
	}
}

void save_pocket::saveFile(const I1 *data, const U4 &u4_dataSize)
{
	using namespace std;
	ofstream fout(FILE_PATH.c_str(),ios_base::out | ios_base::binary);
	if(!fout.is_open())
	{
		perror("ofstream open");
		return;
	}

	fout.write(data,u4_dataSize);
	fout.close();
}

void save_pocket::saveFixedStruct(const I1 *data,const U4 &u4_dataSize)
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

void save_pocket::saveMutableStruct(const I1 *data,const U4 &u4_dataSize)
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

		std::unique_ptr<I1 []> array_str(new (std::nothrow)I1[u2_strSize]);
		if(nullptr == array_str.get())
		{
			std::cout << "new array_str error!\n";
			return;
		}

		memcpy(array_str.get(),data + u4_offset,u2_strSize);
		u4_offset += u2_strSize;

		structMutable.str.insert(0,array_str.get(),u2_strSize);
	}

	while(u4_offset < u4_dataSize)
	{
		std::string str_tmp = "";

		memcpy(&u2_strSize,data + u4_offset,sizeof(U2));
		u4_offset += sizeof(U2);

		std::unique_ptr<I1 []> array_str(new (std::nothrow)I1[u2_strSize]);
		if(nullptr == array_str.get())
		{
			std::cout << "new array_str error!\n";
			return;
		}

		memcpy(array_str.get(),data + u4_offset,u2_strSize);
		u4_offset += u2_strSize;

		str_tmp.insert(0,array_str.get(),u2_strSize);
		structMutable.vector_strList.push_back(str_tmp);
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
