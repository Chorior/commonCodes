#include "client.h"

#define FILE_PATH "send_file"  	    // send file's path

int main()
{
  socketTCPClient client;
  client.sendFile(FILE_PATH);
  client.quit();

  return EXIT_SUCCESS;
}
