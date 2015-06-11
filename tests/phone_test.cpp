///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Phone utility function unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "phone.hpp"

using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( PhoneUtils )

BOOST_AUTO_TEST_CASE( testFormInternationalPhone )
{
	// check standard international number
	BOOST_CHECK( formInternationalPhone("+380989007373", 380) == 380989007373ULL);
	
	// check international number without "+"
	BOOST_CHECK( formInternationalPhone("380989007373", 380) == 380989007373ULL);

	// ckeck without prefix
	BOOST_CHECK( formInternationalPhone("380989007373", 0) == 380989007373ULL);
	BOOST_CHECK( formInternationalPhone("+380989007373", 0) == 380989007373ULL);
	
	// check national number
	BOOST_CHECK( formInternationalPhone("0989007373", 380) == 380989007373ULL);
	
	// ckeck without prefix
	BOOST_CHECK( formInternationalPhone("0989007373", 0) == 0);
	
	// check Aussie numbers
	BOOST_CHECK( formInternationalPhone("+61458852392", 61) == 61458852392ULL);
	BOOST_CHECK( formInternationalPhone("0458852392", 61) == 61458852392ULL);
	
	// ckeck without prefix
	BOOST_CHECK( formInternationalPhone("0458852392", 0) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
