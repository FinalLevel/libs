///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different text processing utility functions implementation
///////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include "text_util.hpp"

namespace fl {
	namespace utils {
		inline char hex2int(unsigned char c)
		{
			if ( isdigit(c) )
			{
				return c - '0';
			}
			else if ( c >= 'A' && c <= 'F' )
			{
				return c - 'A' + 10;
			}
			else if ( c >= 'a' && c <= 'f' )
			{
				return c - 'a' + 10;
			}
			else
			{
				return -1;
			}
		}
		
		void quotedPrintableDecode(fl::strings::BString &result, const char *input, const size_t size, 
			const char delim)
		{
			const char *end = input + size;
			char *res = result.reserveBuffer(size);
			const char *startRes = res;
			while (input < end) {
				if (*input == delim) {
					const char *next1 = input + 1;
					const char *next2 = input + 2;
					if (next2 < end && isxdigit(*next1) && isxdigit(*next2)) {
						unsigned char ch = (hex2int(*next1) << 4) + hex2int(*next2);
						*res = ch;
						res++;
						input += 3;
					} else {  /* check for soft line break according to RFC 2045*/
						const char *lb = input + 1;
						while (lb < end && (*lb == ' ' || *lb == '\t')) {
							 /* Possibly, skip spaces/tabs at the end of line */
							lb++;
						}
						if (lb >= end) {
							/* End of line reached */
							input = end;
						} else if (*lb == '\r' && (lb + 1 < end) && *(lb + 1) == '\n') {
							/* CRLF */
							input = lb + 2;
						} else if (*lb == '\r' || *lb == '\n') {
							/* CR or LF */
							input = lb + 1;
						} else {
							*res = *input;
							res++;
							input++;
						}
					}
				} else {
					*res = *input;
					res++;
					input++;
				}
			}
			result.trim(res - startRes);
		}
	};
};
