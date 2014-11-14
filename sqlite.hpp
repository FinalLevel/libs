#pragma once
#ifndef __FL_SQLITE_HPP
#define	__FL_SQLITE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SQLite wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <sqlite3.h>
#include <string>
#include "bstring.hpp"

namespace fl {
	namespace db {
		using fl::strings::BString;
		
		typedef std::shared_ptr<sqlite3> TSQLiteDescriptorSharedPtr;
		class SQLite
		{
		public:
			SQLite();
			SQLite(const SQLite &) = delete;
			const std::string &filename() const
			{
				return _filename;
			}	
			bool open(const char * const filename, const int flags);
			class SQLiteStatement createStatement(const BString &sql);
			bool execute(const BString &sql);
		private:
			std::string _filename;
			TSQLiteDescriptorSharedPtr _conn;
		};
		
		class SQLiteStatement
		{
		public:
			class Error : public std::exception 
			{
			public:
				Error(const int result)
					: _result(result)
				{
				}
			private:
				int _result;
			};
			SQLiteStatement(SQLiteStatement &&stmt);
			SQLiteStatement &operator= (SQLiteStatement &&src);
			SQLiteStatement(const SQLiteStatement &) = delete;
			
			~SQLiteStatement();
			
			bool execute();
			bool next();
			void reset();
			const char *get(const int iCol);
			
			template <typename R>
			R get(const int iCol) { return get_impl<R>::get(this, iCol); }

			template<typename R, typename = void>
			struct get_impl
			{
			};

			template<typename S>
			struct get_impl<uint8_t, S>
			{
					static uint8_t get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int(stmt->_ppStmt, iCol);
					}
			};

			template<typename S>
			struct get_impl<uint16_t, S>
			{
					static uint16_t get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int(stmt->_ppStmt, iCol);
					}
			};


			template<typename S>
			struct get_impl<uint32_t, S>
			{
					static uint32_t get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int(stmt->_ppStmt, iCol);
					}
			};

			template<typename S>
			struct get_impl<uint64_t, S>
			{
					static uint64_t get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int64(stmt->_ppStmt, iCol);
					}
			};
			
			template<typename S>
			struct get_impl<int64_t, S>
			{
					static int64_t get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int64(stmt->_ppStmt, iCol);
					}
			};

			template<typename S>
			struct get_impl<int, S>
			{
					static int get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_int(stmt->_ppStmt, iCol);
					}
			};

			template<typename S>
			struct get_impl<double, S>
			{
					static double get(SQLiteStatement *stmt, const int iCol)
					{ 
						return sqlite3_column_double(stmt->_ppStmt, iCol);
					}
			};
			template<typename S>
			struct get_impl<std::string, S>
			{
					static std::string get(SQLiteStatement *stmt, const int iCol)
					{ 
						const unsigned char *data = sqlite3_column_text(stmt->_ppStmt, iCol);
						int size = sqlite3_column_bytes(stmt->_ppStmt, iCol);
						return std::string(data, size);
					}
			};
			
			void bind(const int iValue, const int val);
			void bind(const int iValue, const double val);
			void bind(const int iValue, const std::string &val);
			void bind(const int iValue, const BString &val);
			void bind(const int iValue, const char * const text);
			void bind(const int iValue, const char * const text, const size_t length);
			
			void bindNull(const int iValue);

		private:
			friend class SQLite;
			SQLiteStatement(TSQLiteDescriptorSharedPtr &conn, const BString &sql);
			TSQLiteDescriptorSharedPtr _conn;
			sqlite3_stmt *_ppStmt;		
		};
	};
};

#endif	// __FL_SQLITE_HPP
