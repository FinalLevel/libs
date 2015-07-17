#pragma once
#ifndef __FL_MYSQL_HPP
#define	__FL_MYSQL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mysql wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <mysql.h>
#include <errmsg.h>
#include <string>
#include <memory>
#include <vector>
#include "exception.hpp"
#include "bstring.hpp"

#ifdef PRINT_DB_TYPE
#define DB_MYSQL 1
#endif

namespace fl {
	namespace db {
		using fl::strings::BString;
		class MysqlError : public fl::exceptions::Error
		{
		public:
			MysqlError(const char *what)
				: Error(what)
			{
			}
		};
		
		
		typedef std::shared_ptr<MYSQL> TMysqlDescriptorSharedPtr;
		typedef std::unique_ptr<class MysqlResult> TMysqlResultPtr;
		
		class Mysql
		{
		public:
			static const unsigned long MAX_SHOW_ERROR_SQL = 1024;
			Mysql();
			
			Mysql(const Mysql &) = delete;
			
			bool connect(const char *hostName, const char *userName, const char *password, const char *dbName, 
				const unsigned int port, const char *socketName=NULL, const unsigned int flags = 0);
			
			bool setCharacterSet(const char *characterSet);
			const char *getCharacterSet() const;
			static void setDefaultCharacterSet(const char *characterSet);
			bool setServerOption(const enum enum_mysql_set_option option);

			TMysqlResultPtr query(const std::string &queryStr);
			TMysqlResultPtr query(const char *queryStr);
			TMysqlResultPtr query(const class MysqlQuery &mysqlQuery);
			TMysqlResultPtr query(const char *queryStr, const unsigned long length);

			TMysqlResultPtr queryUse(const char *queryStr);
			TMysqlResultPtr queryUse(const class MysqlQuery &mysqlQuery);
			TMysqlResultPtr queryUse(const char *queryStr, const unsigned long length);

			bool execute(const char *queryStr);
			bool execute(const class MysqlQuery &mysqlQuery);
			bool execute(const char *queryStr, const unsigned long length, const char *cmd = "execute");

			unsigned long insertID() const;
			unsigned long affectedRows() const;

			void addRealEscape(BString &buf, const char *value, const long length);
			void addRealEscape(BString &buf, const std::string &value);
			void addRealEscape(BString &buf, const BString &value);
			
			bool nextResult();
			
			class MysqlQuery createQuery(const BString::TSize reserved = BString::DEFAULT_RESERVED_SIZE);
		private:
			void _errorQuery(const char *operation, const char *queryStr, const unsigned long length);
			bool _repeatQuery(const char *queryStr, const unsigned long length);
			static std::string _characterSet;
			TMysqlDescriptorSharedPtr _mysql;
		};
		
		class MysqlResult
		{
		public:
			MysqlResult(const MysqlResult &) = delete;
			MysqlResult(MysqlResult &&) = delete;
			MysqlResult& operator=(const MysqlResult &) = delete;
			MysqlResult& operator=(MysqlResult &&moveFrom) = delete;
			
			~MysqlResult();
			bool next();
			void rewind();
			
			const char *get(const int pos)
			{
				return _currentRow[pos];
			}
			
			template <typename R>
			R get(const int pos) { return get_impl<R>::get(this, pos); }

			template<typename R, typename = void>
			struct get_impl
			{
			};

			template<typename S>
			struct get_impl<uint8_t, S>
			{
					static uint8_t get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return strtoul(result->_currentRow[pos], NULL, 10);
						else
							return 0;
					}
			};

			template<typename S>
			struct get_impl<uint16_t, S>
			{
					static uint16_t get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return strtoul(result->_currentRow[pos], NULL, 10);
						else
							return 0;
					}
			};


			template<typename S>
			struct get_impl<uint32_t, S>
			{
					static uint32_t get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return strtoul(result->_currentRow[pos], NULL, 10);
						else
							return 0;
					}
			};

			template<typename S>
			struct get_impl<uint64_t, S>
			{
					static uint64_t get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return strtoull(result->_currentRow[pos], NULL, 10);
						else
							return 0;
					}
			};
			
			template<typename S>
			struct get_impl<int64_t, S>
			{
					static int64_t get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return strtoll(result->_currentRow[pos], NULL, 10);
						else
							return 0;
					}
			};

			template<typename S>
			struct get_impl<int, S>
			{
					static int get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return atoi(result->_currentRow[pos]);
						else
							return 0;
					}
			};

			template<typename S>
			struct get_impl<double, S>
			{
					static double get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return atof(result->_currentRow[pos]);
						else
							return 0;
					}
			};
			template<typename S>
			struct get_impl<std::string, S>
			{
					static std::string get(MysqlResult *result, const int pos)
					{ 
						if (result->_currentRow[pos])
							return std::string(result->_currentRow[pos]);
						else
							return std::string();
					}
			};

			unsigned long getFieldLength(const int pos);
			unsigned long numRows();
			const char *getFieldName(const int pos);
		private:
			MysqlResult(MYSQL_RES *resultSet);
			
			friend class Mysql; // used only to prevent construction from other sources

			MYSQL_RES *_result;
			MYSQL_ROW _currentRow;
		};
		
		struct MysqlQueryEscape {};
		const MysqlQueryEscape ESC = {};
		
		using fl::strings::CLR;
		
		class MysqlQuery : public BString
		{
		public:
			MysqlQuery(MysqlQuery &&src);
			MysqlQuery &operator= (MysqlQuery &&src);
		
			MysqlQuery &operator<< (const MysqlQueryEscape)
			{
				_needEscape = true;
				return *this;
			}
			MysqlQuery &operator<< (const char *str)
			{
				if (_needEscape) {
					_escape(str, strlen(str));
					_needEscape = false;
				}
				else
					BString::operator<<(str);
				return *this;
			}
			MysqlQuery &operator<< (const std::string &str)
			{
				if (_needEscape) {
					_escape(str.c_str(), str.size());
					_needEscape = false;
				}
				else
					BString::operator<<(str);
				return *this;
			}
			
			MysqlQuery &operator<< (const BString &str)
			{
				if (_needEscape) {
					_escape(str.c_str(), str.size());
					_needEscape = false;
				}
				else
					BString::operator<<(str);
				return *this;
			}
			
			template <typename T>
			MysqlQuery& operator<< (const T value)
			{
				_needEscape = false;
				BString::operator<<(value);
				return *this;
			}
			template <typename T>
			MysqlQuery& operator<< (const std::vector<T> &array)
			{
				auto needEscape = _needEscape;
				operator<<('"');
				for (auto v = array.begin(); v != array.end(); v++) {
					_needEscape = needEscape;
					operator<<(*v);
					operator<<(',');
				}
				if (!array.empty())
					trimLast();
				operator<<('"');
				_needEscape = false;
				return *this;
			}
			void clear();
		private:
			friend class Mysql; // used only to prevent construction from other sources
			MysqlQuery(TMysqlDescriptorSharedPtr &mysql, const BString::TSize reserved);
			void _escape(const char *value, const size_t length);

			TMysqlDescriptorSharedPtr _mysql;
			bool _needEscape;
		};
	};
};

#endif	// __FL_MYSQL_HPP
