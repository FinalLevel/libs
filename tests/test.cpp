#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "log.hpp"

struct InitTests {
    InitTests()
		{ 
			srand(time(NULL));
			fl::log::LogSystem::defaultLog().clearTargets();
		}
    ~InitTests()  
		{ 
		}
};
BOOST_GLOBAL_FIXTURE( InitTests );