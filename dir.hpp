#pragma once
#ifndef __FL_DIR_HPP
#define	__FL_DIR_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Directory functions wrapper class
///////////////////////////////////////////////////////////////////////////////
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "exception.hpp"

namespace fl {
	namespace fs {

		class Directory
		{
		public:
			class Error : public fl::exceptions::Error 
			{
			public:
				Error(const char *what)
					: fl::exceptions::Error(what)
				{

				}
			};

			Directory(const char *path);
			~Directory();
			bool next();
			void rewind();
			const char *name() const
			{
				return _entry.d_name;
			}
			bool isDirectory() const
			{
				return (_entry.d_type == DT_DIR);
			}
			static const int DEFAULT_DIR_MODE = S_IRWXU;
			static bool makeDirRecursive(const char *path, int mode = DEFAULT_DIR_MODE);
			static bool rmDirRecursive(const char *path);
		private:
			DIR *_dir;
			struct dirent _entry;
		};
	};
};

#endif	// __FL_DIR_HPP
