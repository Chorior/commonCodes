#include "client.h"

#define FILE_PATH "send_file"  	    // send file's path

int main()
{
  socketTCPClient client;
  //client.sendFile(FILE_PATH);
  {
    FIXED_LENGTH_STRUCT fixedStruct
    {
      123.321,
      456.654,
      123,
      321,
      456,
      654,
      'F',
      'K'
    };

    client.sendFixedStruct(fixedStruct);
  }
  client.quit();

  return EXIT_SUCCESS;
}
