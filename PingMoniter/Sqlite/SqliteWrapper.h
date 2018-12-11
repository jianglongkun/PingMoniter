#pragma once
#ifndef __SQLITEWRAPPER__H_
#define __SQLITEWRAPPER__H_

#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
//#include "CharacterConvert.h"
#define SQLITEWRAPPERERROR 1000

class Sqlite3Memory
{
public:
	Sqlite3Memory();
	Sqlite3Memory(int nBufferLen);
	Sqlite3Memory(const char *szFormat, va_list list);
	~Sqlite3Memory();

	Sqlite3Memory(Sqlite3Memory const &other);
	Sqlite3Memory &operator=(Sqlite3Memory const &lhs);

	Sqlite3Memory(Sqlite3Memory &&othre);
	Sqlite3Memory &operator=(Sqlite3Memory &&lhs);

	void swap(Sqlite3Memory &other);

	int GetLength()const
	{
		return mnBufferLen;
	}

	void *GetBuffer()const
	{
		return mpBuf;
	}

	void clear();
private:
	int mnBufferLen;
	void* mpBuf;
};

class SqliteWrapperException
{
public:
	SqliteWrapperException(const int nErrCode,const char* szErrMess,bool bDeleteMsg = true);

	SqliteWrapperException(const SqliteWrapperException&  e);

	virtual ~SqliteWrapperException();

	const int errorCode() const { return mnErrCode; }

	const char* errorMessage() const { return mpszErrMess; }

	static const char* errorCodeAsString(int nErrCode);

private:
	int mnErrCode;
	char* mpszErrMess;
};

class SqliteWrapperBuffer
{
public:
	const char* format(const char* szFormat, ...);

	operator const char*() { return static_cast<char const*>(mBuf.GetBuffer()); }

	void clear();

private:
	Sqlite3Memory mBuf;
};

class SqliteWrapperBinary
{
public:

	SqliteWrapperBinary();

	~SqliteWrapperBinary();

	void setBinary(const unsigned char* pBuf, int nLen);
	void setEncoded(const unsigned char* pBuf);

	const unsigned char* getEncoded();
	const unsigned char* getBinary();

	int getBinaryLength();

	unsigned char* allocBuffer(int nLen);

	void clear();

private:
	unsigned char* mpBuf;
	int mnBinaryLen;
	int mnBufferLen;
	int mnEncodedLen;
	bool mbEncoded;
};

class SqliteWrapperQuery
{
public:

	SqliteWrapperQuery();

	SqliteWrapperQuery(const SqliteWrapperQuery &rQuery);

	SqliteWrapperQuery(sqlite3 *pDB,sqlite3_stmt *pVM,bool bEof,bool bOwnVM = true);

	SqliteWrapperQuery& operator=(const SqliteWrapperQuery &rQuery);

	virtual ~SqliteWrapperQuery();

	int NumFields() const;

	int FieldIndex(const char *szField) const;

	const char* FieldName(int nCol) const;

	const char* FieldDeclType(int nCol) const;
	int FieldDataType(int nCol) const;

	const char* FieldValue(int nField) const;
	const char* FieldValue(const char *szField) const;

	int GetIntField(int nField, int nNullValue = 0) const;
	int GetIntField(const char *szField, int nNullValue = 0) const;

	long long GetInt64Field(int nField, long long nNullValue = 0) const;
	long long GetInt64Field(const char* szField, long long nNullValue = 0) const;

	double GetFloatField(int nField, double fNullValue = 0.0) const;
	double GetFloatField(const char *szField, double fNullValue = 0.0) const;

	const char *GetStringField(int nField, const char *szNullValue = "") const;
	const char *GetStringField(const char* szField, const char *szNullValue = "") const;

	const unsigned char *GetBlobField(int nField, int &nLen) const;
	const unsigned char *GetBlobField(const char *szField, int &nLen) const;

	bool FieldIsNull(int nField) const;
	bool FieldIsNull(const char *szField) const;

	bool Eof() const;

	void NextRow();

	void Finalize();

private:

	void CheckVM() const;

	sqlite3* mpDB;
	sqlite3_stmt* mpVM;
	bool mbEof;
	int mnCols;
	bool mbOwnVM;
};

class SqliteWrapperTable
{
public:

	SqliteWrapperTable();

	SqliteWrapperTable(const SqliteWrapperTable &rTable);

	SqliteWrapperTable(char **paszResults, int nRows, int nCols);

	virtual ~SqliteWrapperTable();

	SqliteWrapperTable &operator=(const SqliteWrapperTable &rTable);

	int NumFields() const;

	int NumRows() const;

	const char *FieldName(int nCol) const;

	const char *FieldValue(int nField) const;
	const char *FieldValue(const char *szField) const;

	int GetIntField(int nField, int nNullValue = 0) const;
	int GetIntField(const char *szField, int nNullValue = 0) const;

	double GetFloatField(int nField, double fNullValue = 0.0) const;
	double GetFloatField(const char *szField, double fNullValue = 0.0) const;

	const char *GetStringField(int nField, const char *szNullValue = "") const;
	const char *GetStringField(const char *szField, const char *szNullValue = "") const;

	bool FieldIsNull(int nField) const;
	bool FieldIsNull(const char *szField) const;

	void SetRow(int nRow);

	void Finalize();

private:

	void CheckResults() const;

	int mnCols;
	int mnRows;
	int mnCurrentRow;
	char **mpaszResults;
};

class SqliteWrapperStatement
{
public:

	SqliteWrapperStatement();

	SqliteWrapperStatement(const SqliteWrapperStatement &rStatement);

	SqliteWrapperStatement(sqlite3 *pDB, sqlite3_stmt *pVM);

	virtual ~SqliteWrapperStatement();

	SqliteWrapperStatement &operator=(const SqliteWrapperStatement &rStatement);

	int ExecDML();

	SqliteWrapperQuery ExecQuery();

	void Bind(int nParam, const char *szValue);
	void Bind(int nParam, const int nValue);
	void Bind(int nParam, const long long nValue);
	void Bind(int nParam, const double dwValue);
	void Bind(int nParam, const unsigned char *blobValue, int nLen);
	void BindNull(int nParam);

	void Reset();

	void Finalize();

private:

	void CheckDB() const;
	void CheckVM() const;
	sqlite3 *mpDB;
	sqlite3_stmt *mpVM;
};

class SqliteWrapperDB
{
public:

	SqliteWrapperDB();

	virtual ~SqliteWrapperDB();

	void Open(const char *szFile);

	void Close();

	bool TableExists(const char *szTable);

	int ExecDML(const char *szSQL);

	SqliteWrapperQuery ExecQuery(const char *szSQL);

	int ExecScalar(const char *szSQL);

	SqliteWrapperTable GetTable(const char *szSQL);

	SqliteWrapperStatement CompileStatement(const char *szSQL);

	sqlite_int64 LastRowId() const;

	void Interrupt() { sqlite3_interrupt(mpDB); }

	void SetBusyTimeout(int nMillisecs);

	static const char* SQLiteVersion() { return SQLITE_VERSION; }

private:

	SqliteWrapperDB(const SqliteWrapperDB &db);
	SqliteWrapperDB &operator=(const SqliteWrapperDB &db);

	sqlite3_stmt* Compile(const char *szSQL);

	void CheckDB() const;

	sqlite3* mpDB;
	int mnBusyTimeoutMs;
};

#endif