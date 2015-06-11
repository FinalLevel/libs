///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Phone numbers manipulation functions implementation
///////////////////////////////////////////////////////////////////////////////

#include "phone.hpp"
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/phonenumber.pb.h"
using namespace i18n::phonenumbers;

namespace fl {
	namespace utils {	
		uint64_t parsePhone(PhoneNumberUtil *util, const std::string &phone, const std::string &region_code)
		{
			PhoneNumber pn;
			if (util->Parse(phone, region_code, &pn) == PhoneNumberUtil::NO_PARSING_ERROR)
				if (util->IsValidNumber(pn) && util->GetNumberType(pn) == PhoneNumberUtil::MOBILE)
				{
					std::string fn;
					util->Format(pn, PhoneNumberUtil::E164, &fn);
					return strtoull(fn.c_str(), NULL, 10);
				}
			return 0;
		}

		uint64_t formInternationalPhone(const std::string &phone, const uint32_t countryPrefix)
		{
			std::string region_code;
			PhoneNumberUtil *util = PhoneNumberUtil::GetInstance();

			if (!countryPrefix && *phone.c_str() != '+')
			{
				std::string pPhone = "+" + phone;
				return parsePhone(util, pPhone, region_code);
			}
			if (countryPrefix)
				util->GetRegionCodeForCountryCode(countryPrefix, &region_code);
			return parsePhone(util, phone, region_code);
		}
	}
}

