#ifndef _SAVE_POCKET_H_
#define _SAVE_POCKET_H_

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include "state_machine.h"
#include "types.h"

class save_pocket
{
	messaging::receiver incoming;
	std::mutex iom;
  const std::string FILE_PATH = "receive_file";

  save_pocket(save_pocket const&)=delete;
	save_pocket& operator=(save_pocket const&)=delete;

public:

  save_pocket() = default;
  ~save_pocket() = default;

	void run();
	void saveFile(const I1 *data, const U4 &u4_dataSize);
	void saveFixedStruct(const I1 *data,const U4 &u4_dataSize);
	void saveMutableStruct(const I1 *data,const U4 &u4_dataSize);

  void done()
	{
		get_sender().send(messaging::close_queue());
	}

	messaging::sender get_sender()
	{
		return incoming;
	}
};

#endif
