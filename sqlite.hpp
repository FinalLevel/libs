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
		
		class SQLiteStatement;
		class SQLiteAutoRollbackTransaction;
		
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
			SQLiteStatement createStatement(const BString &sql);
			SQLiteStatement createStatement(const char * const sql);
			SQLiteStatement createStatement(const std::string &sql);
			bool execute(const BString &sql);
			sqlite3_int64 insertId();
			int affectedRows();
			
			SQLiteAutoRollbackTransaction startAutoRollbackTransaction();
		private:
			std::string _filename;
			TSQLiteDescriptorSharedPtr _conn;
		};
		
		class SQLiteAutoRollbackTransaction
		{
		public:
			class TransactionError : public std::exception 
			{
			};
			SQLiteAutoRollbackTransaction(SQLiteAutoRollbackTransaction &&transaction);
			SQLiteAutoRollbackTransaction &operator= (SQLiteAutoRollbackTransaction &&src);
			SQLiteAutoRollbackTransaction(const SQLiteAutoRollbackTransaction &) = delete;
			
			~SQLiteAutoRollbackTransaction();
			
			void commit();
		private:
			friend class SQLite;
			SQLiteAutoRollbackTransaction(TSQLiteDescriptorSharedPtr &conn);
			TSQLiteDescriptorSharedPtr _conn;
			bool _rollback { true };
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
			int affectedRows();
			bool next();
			void reset();
			const char *get(const int iCol);
			
			int length(const int iCol);
			
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
						const char *data = reinterpret_cast<const char*>(sqlite3_column_text(stmt->_ppStmt, iCol));
						int size = sqlite3_column_bytes(stmt->_ppStmt, iCol);
						return std::string(data, size);
					}
			};
			
			void bind(const int iValue, const uint32_t val);
			void bind(const int iValue, const uint64_t val);
			void bind(const int iValue, const int val);
			void bind(const int iValue, const long int val);
			void bind(const int iValue, const double val);
			void bind(const int iValue, const std::string &val);
			void bind(const int iValue, const BString &val);
			void bind(const int iValue, const char * const text);
			void bind(const int iValue, const char * const text, const size_t length);
			void bind(const int iValue, const uint8_t * const data, const size_t length);
			void bindBlob(const int iValue, const void* data, const size_t length);
			
			void bindNull(const int iValue);

		private:
			friend class SQLite;
			friend class SQLiteAutoRollbackTransaction;
			SQLiteStatement(TSQLiteDescriptorSharedPtr &conn, const char * const sql, const size_t size);
			TSQLiteDescriptorSharedPtr _conn;
			sqlite3_stmt *_ppStmt;	
			const char *_sqlString;
		};
	};
};

#endif	// __FL_SQLITE_HPP
