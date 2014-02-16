#include <sstream>
#include <iostream>
#include <boost/format.hpp>

int main()
{
	std::ostringstream ss;

	//boost::format hitFormat("(%u,%u,%u,%u),");
	for (int i = 0; i < 1000000; i++)
	{
		if (ss.tellp() <= 0)
			ss << "INSERT INTO hit (id,name) VALUES(";
		
		ss << "(" << i << ","  << (i + 1) << "," << 3 << ","  << 4 << "),";
		static const int MAX_SQL_LENGTH = 1000;
		if (ss.tellp() > MAX_SQL_LENGTH)
		{
			ss << " ON DUPLICATE KEY UPDATE id=VALUES(ID)";
			//std::cout << ss.str() << std::endl;
			ss.seekp(0);
		}
	}
}
