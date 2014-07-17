#pragma once
#ifndef __FL_TEST_PATH_HPP
#define	__FL_TEST_PATH_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: TestPath unit tests helper class
///////////////////////////////////////////////////////////////////////////////
#include "dir.hpp"
#include "bstring.hpp"

namespace fl {
	namespace tests {
		using fl::strings::BString;
		using fl::fs::Directory;
		
		class TestPath
		{
		public:
			TestPath(const char *base)
			{
				static int call = 0;
				call++;
				BString path;
				path.sprintfSet("/tmp/test_%s_%u_%u", base, call, rand());
				_path = path.c_str();
				Directory::rmDirRecursive(_path.c_str());
				Directory::makeDirRecursive(_path.c_str());
			}

			~TestPath()
			{
				Directory::rmDirRecursive(_path.c_str());
			}

			const char *path() const
			{
				return _path.c_str();
			}
			const int countFiles(const char *subdir)
			{
				BString path;
				path.sprintfSet("%s/%s", _path.c_str(), subdir);
				int count = 0;
				Directory dir(path.c_str());
				BString fileName;
				while (dir.next())
				{
					fileName.sprintfSet("%s/%s", path.c_str(), dir.name());
					if (dir.isDirectory(fileName.c_str()))
						continue;
					count++;
				}
				return count;
			}
		private:
			std::string _path;
		};
	};
};

#endif	// __FL_TEST_PATH_HPP
