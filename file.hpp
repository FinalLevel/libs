#pragma once
#ifndef __FL_FILE_HPP
#define	__FL_FILE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Direct file IO wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace fl {
	namespace fs {
		
		class File
		{
		public:
			static const int DEFAULT_OPEN_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
			typedef int TDescriptor;
			
			File()
				: _descr(0)
			{
			}
			~File()
			{
				close();
			}
			File(const File &file) = delete;
			File(File &&file)
				: _descr(file._descr)
			{
				file._descr = 0;
			}
			File& operator=(File &&moveFrom);
			
			TDescriptor descr()
			{
				return _descr;
			}
			
			
			bool open(const char *name, const int flags, const mode_t mode = DEFAULT_OPEN_MODE);
			void close();
			ssize_t read(void *buf, const size_t size);
			ssize_t write(const void *buf, const size_t size);
			ssize_t fileSize();
			int64_t fileSize64();
			off_t seek(off_t offset, int whence);
			int64_t seek64(int64_t offset, int whence);
			
			bool createUnlinkedTmpFile(const char* tmpDir);
			
			ssize_t pread(void *buf, const size_t size, const off_t offset);
			ssize_t pwrite(const void *buf, const size_t size, const off_t offset);
			
			bool truncate(const off_t size);
			static bool touch(const char *fileName, const time_t modTime);
		private:
			TDescriptor _descr;
		};
	};
};

#endif	// __FL_FILE_HPP
