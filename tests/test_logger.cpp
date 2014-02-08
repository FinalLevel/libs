#include "log.hpp"

using namespace fl;

int main()
{
	log::Fatal::L("Fatal erorr %s\n", "Fatal");
	log::Info::L("Info erorr %s\n", "Info");
	log::Error::L("Erorr %s\n", "Error");
	
	try
	{
		log::LogSystem::addTarget(new log::ScreenTarget());
		log::LogSystem::addTarget(new log::FileTarget("/tmp/testliblog"));
	}
	catch (...)
	{
		return -1;
	};
	log::Fatal::L("Target Fatal erorr %s\n", "Fatal");
	log::Info::L("Target Info erorr %s\n", "Info");
	log::Error::L("Target Erorr %s\n", "Error");

	return 0;
}
