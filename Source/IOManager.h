#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <fstream>
#include <string>
#include <vector>

namespace luna
{
	namespace IO
	{
		extern void LoadFile(const std::string & filename, std::vector<char>& buffer);
	}
}

#endif

