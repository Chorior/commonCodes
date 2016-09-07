#include "client.h"

#define FILE_PATH "send_file"  	    // send file's path

int main()
{
  socketTCPClient client;

  //client.sendFile(FILE_PATH);

  //{
  //  FIXED_LENGTH_STRUCT fixedStruct
  //  {
  //    123.321,
  //    456.654,
  //    123,
  //    321,
  //    456,
  //    654,
  //    'F',
  //    'K'
  //  };
  //
  //  client.sendFixedStruct(fixedStruct);
  //}

  {
    MUTABLE_LENGTH_STRUCT mutableStruct
    {
      110,
      120,
      "hello!"
    };
    mutableStruct.vector_strList.push_back("this");
    mutableStruct.vector_strList.push_back("is");
    mutableStruct.vector_strList.push_back("a");
    mutableStruct.vector_strList.push_back("test");

    client.sendMutableStruct(mutableStruct);
  }

  client.quit();

  return EXIT_SUCCESS;
}
