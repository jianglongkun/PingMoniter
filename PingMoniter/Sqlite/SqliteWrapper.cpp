#include "SqliteWrapper.h"
#include <stdlib.h>
#include <utility>

static const bool DONT_DELETE_MSG = false;

static const char *const ALLOCATION_ERROR_MESSAGE = "Cannot allocate memory";

int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);

Sqlite3Memory::Sqlite3Memory()
	:mnBufferLen(0),
	mpBuf(nullptr)
{

}

Sqlite3Memory::Sqlite3Memory(int nBufferLen)
	:mnBufferLen(nBufferLen),
	mpBuf(sqlite3_malloc(nBufferLen))
{
	if (mpBuf && mnBufferLen > 0)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR, ALLOCATION_ERROR_MESSAGE, DONT_DELETE_MSG);
	}
}

Sqlite3Memory::Sqlite3Memory(const char *szFormat, va_list list)
	:mnBufferLen(0),
	mpBuf(sqlite3_vmprintf(szFormat, list))
{
	if (!mpBuf)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,ALLOCATION_ERROR_MESSAGE,DONT_DELETE_MSG);
	}
	mnBufferLen = strlen(static_cast<char const*>(mpBuf)) + 1;
}

Sqlite3Memory::~Sqlite3Memory()
{
	clear();
}

Sqlite3Memory::Sqlite3Memory(Sqlite3Memory const &other)
	:mnBufferLen(other.mnBufferLen),
	mpBuf(sqlite3_malloc(other.mnBufferLen))
{
	if (!mpBuf && mnBufferLen>0)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,ALLOCATION_ERROR_MESSAGE,DONT_DELETE_MSG);
	}
	memcpy(mpBuf, other.mpBuf, mnBufferLen);
}

Sqlite3Memory &Sqlite3Memory::operator=(Sqlite3Memory const &lhs)
{
	Sqlite3Memory tmp(lhs);
	swap(tmp);
	return *this;
}

Sqlite3Memory::Sqlite3Memory(Sqlite3Memory &&othre)
	:mnBufferLen(othre.mnBufferLen),
	mpBuf(othre.mpBuf)
{
	othre.mnBufferLen = 0;
	othre.mpBuf = nullptr;
}

Sqlite3Memory &Sqlite3Memory::operator=(Sqlite3Memory &&lhs)
{
	swap(lhs);
	return *this;
}

void Sqlite3Memory::swap(Sqlite3Memory &other)
{
	std::swap(mnBufferLen, other.mnBufferLen);
	std::swap(mpBuf, other.mpBuf);
}

void Sqlite3Memory::clear()
{
	sqlite3_free(mpBuf);
	mpBuf = nullptr;
	mnBufferLen = 0;
}

SqliteWrapperException::SqliteWrapperException(const int nErrCode, const char* szErrMess, bool bDeleteMsg)
	:mnErrCode(nErrCode)
{
	mpszErrMess = sqlite3_mprintf("%s[%d]: %s",errorCodeAsString(nErrCode),nErrCode,szErrMess ? szErrMess : "");
	if (bDeleteMsg && szErrMess)
	{
		sqlite3_free((void*)szErrMess);
	}
}

SqliteWrapperException::SqliteWrapperException(const SqliteWrapperException&  e)
	:mnErrCode(e.mnErrCode)
{
	mpszErrMess = 0;
	if (e.mpszErrMess)
	{
		mpszErrMess = sqlite3_mprintf("%s", e.mpszErrMess);
	}
}

SqliteWrapperException::~SqliteWrapperException()
{
	if (mpszErrMess)
	{
		sqlite3_free(mpszErrMess);
		mpszErrMess = 0;
	}
}

const char *SqliteWrapperException::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
	case SQLITE_OK: return "SQLITE_OK";
	case SQLITE_ERROR: return "SQLITE_ERROR";
	case SQLITE_INTERNAL: return "SQLITE_INTERNAL";
	case SQLITE_PERM: return "SQLITE_PERM";
	case SQLITE_ABORT: return "SQLITE_ABORT";
	case SQLITE_BUSY: return "SQLITE_BUSY";
	case SQLITE_LOCKED: return "SQLITE_LOCKED";
	case SQLITE_NOMEM: return "SQLITE_NOMEM";
	case SQLITE_READONLY: return "SQLITE_READONLY";
	case SQLITE_INTERRUPT: return "SQLITE_INTERRUPT";
	case SQLITE_IOERR: return "SQLITE_IOERR";
	case SQLITE_CORRUPT: return "SQLITE_CORRUPT";
	case SQLITE_NOTFOUND: return "SQLITE_NOTFOUND";
	case SQLITE_FULL: return "SQLITE_FULL";
	case SQLITE_CANTOPEN: return "SQLITE_CANTOPEN";
	case SQLITE_PROTOCOL: return "SQLITE_PROTOCOL";
	case SQLITE_EMPTY: return "SQLITE_EMPTY";
	case SQLITE_SCHEMA: return "SQLITE_SCHEMA";
	case SQLITE_TOOBIG: return "SQLITE_TOOBIG";
	case SQLITE_CONSTRAINT: return "SQLITE_CONSTRAINT";
	case SQLITE_MISMATCH: return "SQLITE_MISMATCH";
	case SQLITE_MISUSE: return "SQLITE_MISUSE";
	case SQLITE_NOLFS: return "SQLITE_NOLFS";
	case SQLITE_AUTH: return "SQLITE_AUTH";
	case SQLITE_FORMAT: return "SQLITE_FORMAT";
	case SQLITE_RANGE: return "SQLITE_RANGE";
	case SQLITE_ROW: return "SQLITE_ROW";
	case SQLITE_DONE: return "SQLITE_DONE";
	case SQLITEWRAPPERERROR: return "SQLITEWRAPPERERROR";
	default: return "UNKNOWN_ERROR";
	}
}

const char* SqliteWrapperBuffer::format(const char* szFormat, ...)
{
	clear();
	va_list va;
	try
	{
		va_start(va, szFormat);
		mBuf = Sqlite3Memory(szFormat, va);
		va_end(va);
		return static_cast<const char*>(mBuf.GetBuffer());
	}
	catch (SqliteWrapperException&)
	{
		va_end(va);
		throw;
	}
}

void SqliteWrapperBuffer::clear()
{
	mBuf.clear();
}

SqliteWrapperBinary::SqliteWrapperBinary()
	:mpBuf(0),
	mnBinaryLen(0),
	mnBufferLen(0),
	mnEncodedLen(0),
	mbEncoded(false)
{

}

SqliteWrapperBinary::~SqliteWrapperBinary()
{
	clear();
}


void SqliteWrapperBinary::setBinary(const unsigned char* pBuf, int nLen)
{
	mpBuf = allocBuffer(nLen);
	memcpy(mpBuf, pBuf, nLen);
}

void SqliteWrapperBinary::setEncoded(const unsigned char* pBuf)
{
	clear();

	mnEncodedLen = strlen((const char*)pBuf);
	mnBufferLen = mnEncodedLen + 1; // Allow for NULL terminator

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,ALLOCATION_ERROR_MESSAGE,DONT_DELETE_MSG);
	}

	memcpy(mpBuf, pBuf, mnBufferLen);
	mbEncoded = true;
}

const unsigned char* SqliteWrapperBinary::getEncoded()
{
	if (!mbEncoded)
	{
		unsigned char* ptmp = (unsigned char*)malloc(mnBinaryLen);
		memcpy(ptmp, mpBuf, mnBinaryLen);
		mnEncodedLen = sqlite3_encode_binary(ptmp, mnBinaryLen, mpBuf);
		free(ptmp);
		mbEncoded = true;
	}

	return mpBuf;
}

const unsigned char* SqliteWrapperBinary::getBinary()
{
	if (mbEncoded)
	{
		// in/out buffers can be the same
		mnBinaryLen = sqlite3_decode_binary(mpBuf, mpBuf);

		if (mnBinaryLen == -1)
		{
			throw SqliteWrapperException(SQLITEWRAPPERERROR,"Cannot decode binary",DONT_DELETE_MSG);
		}

		mbEncoded = false;
	}

	return mpBuf;
}

int SqliteWrapperBinary::getBinaryLength()
{
	getBinary();
	return mnBinaryLen;
}

unsigned char* SqliteWrapperBinary::allocBuffer(int nLen)
{
	clear();

	// Allow extra space for encoded binary as per comments in
	// SQLite encode.c See bottom of this file for implementation
	// of SQLite functions use 3 instead of 2 just to be sure ;-)
	mnBinaryLen = nLen;
	mnBufferLen = 3 + (257 * nLen) / 254;

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR, ALLOCATION_ERROR_MESSAGE, DONT_DELETE_MSG);
	}

	mbEncoded = false;

	return mpBuf;
}

void SqliteWrapperBinary::clear()
{
	if (mpBuf)
	{
		mnBinaryLen = 0;
		mnBufferLen = 0;
		free(mpBuf);
		mpBuf = 0;
	}
}

SqliteWrapperQuery::SqliteWrapperQuery()
{
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}

SqliteWrapperQuery::SqliteWrapperQuery(const SqliteWrapperQuery &rQuery)
{
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<SqliteWrapperQuery&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
}

SqliteWrapperQuery::SqliteWrapperQuery(sqlite3 *pDB, sqlite3_stmt *pVM, bool bEof, bool bOwnVM)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = sqlite3_column_count(mpVM);
	mbOwnVM = bOwnVM;
}

SqliteWrapperQuery &SqliteWrapperQuery::operator=(const SqliteWrapperQuery &rQuery)
{
	try
	{
		Finalize();
	}
	catch (...)
	{
	}
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<SqliteWrapperQuery&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	return *this;
}

SqliteWrapperQuery::~SqliteWrapperQuery()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
	}
}

int SqliteWrapperQuery::NumFields() const
{
	CheckVM();
	return mnCols;
}

int SqliteWrapperQuery::FieldIndex(const char *szField) const
{
	CheckVM();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			const char* szTemp = sqlite3_column_name(mpVM, nField);

			if (strcmp(szField, szTemp) == 0)
			{
				return nField;
			}
		}
	}

	//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field name requested",DONT_DELETE_MSG);
	printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field name requested");
	return -1;
}

const char *SqliteWrapperQuery::FieldName(int nCol) const
{
	CheckVM();

	if (nCol < 0 || nCol > mnCols - 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field index requested");
		return "";
	}

	return sqlite3_column_name(mpVM, nCol);
}

const char *SqliteWrapperQuery::FieldDeclType(int nCol) const
{
	CheckVM();

	if (nCol < 0 || nCol > mnCols - 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field index requested");
		return "";
	}

	return sqlite3_column_decltype(mpVM, nCol);
}

int SqliteWrapperQuery::FieldDataType(int nCol) const
{
	CheckVM();

	if (nCol < 0 || nCol > mnCols - 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field index requested");
		return -1;
	}

	return sqlite3_column_type(mpVM, nCol);
}

const char *SqliteWrapperQuery::FieldValue(int nField) const
{
	CheckVM();

	if (nField < 0 || nField > mnCols - 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field index requested");
		return "";
	}

	return (const char*)sqlite3_column_text(mpVM, nField);
}

const char *SqliteWrapperQuery::FieldValue(const char *szField) const
{
	int nField = FieldIndex(szField);
	return (const char*)sqlite3_column_text(mpVM, nField);
}

int SqliteWrapperQuery::GetIntField(int nField, int nNullValue) const
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return sqlite3_column_int(mpVM, nField);
	}
}

int SqliteWrapperQuery::GetIntField(const char *szField, int nNullValue) const
{
	int nField = FieldIndex(szField);
	return GetIntField(nField, nNullValue);
}

long long SqliteWrapperQuery::GetInt64Field(int nField, long long nNullValue) const
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return sqlite3_column_int64(mpVM, nField);
	}
}

long long SqliteWrapperQuery::GetInt64Field(const char* szField, long long nNullValue) const
{
	int nField = FieldIndex(szField);
	return GetInt64Field(nField, nNullValue);
}

double SqliteWrapperQuery::GetFloatField(int nField, double fNullValue) const
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return sqlite3_column_double(mpVM, nField);
	}
}

double SqliteWrapperQuery::GetFloatField(const char *szField, double fNullValue) const
{
	int nField = FieldIndex(szField);
	return GetFloatField(nField, fNullValue);
}

const char *SqliteWrapperQuery::GetStringField(int nField, const char *szNullValue) const
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (const char*)sqlite3_column_text(mpVM, nField);
	}
}

const char *SqliteWrapperQuery::GetStringField(const char* szField, const char *szNullValue) const
{
	int nField = FieldIndex(szField);
	return GetStringField(nField, szNullValue);
}

const unsigned char *SqliteWrapperQuery::GetBlobField(int nField, int &nLen) const
{
	CheckVM();

	if (nField < 0 || nField > mnCols - 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid field index requested");
		return (const unsigned char *)"";
	}

	nLen = sqlite3_column_bytes(mpVM, nField);
	return (const unsigned char*)sqlite3_column_blob(mpVM, nField);
}

const unsigned char *SqliteWrapperQuery::GetBlobField(const char *szField, int &nLen) const
{
	int nField = FieldIndex(szField);
	return GetBlobField(nField, nLen);
}

bool SqliteWrapperQuery::FieldIsNull(int nField) const
{
	return (FieldDataType(nField) == SQLITE_NULL);
}

bool SqliteWrapperQuery::FieldIsNull(const char *szField) const
{
	int nField = FieldIndex(szField);
	return (FieldDataType(nField) == SQLITE_NULL);
}

bool SqliteWrapperQuery::Eof() const
{
	CheckVM();
	return mbEof;
}

void SqliteWrapperQuery::NextRow()
{
	CheckVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = sqlite3_finalize(mpVM);
		mpVM = 0;
		const char* szError = sqlite3_errmsg(mpDB);
		//throw SqliteWrapperException(nRet,(char*)szError,DONT_DELETE_MSG);
		printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
	}
}

void SqliteWrapperQuery::Finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
			//throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
			printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
		}
	}
}

void SqliteWrapperQuery::CheckVM() const
{
	if (mpVM == 0)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Null Virtual Machine pointer",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Null Virtual Machine pointer");
	}
}

SqliteWrapperTable::SqliteWrapperTable()
{
	mpaszResults = 0;
	mnRows = 0;
	mnCols = 0;
	mnCurrentRow = 0;
}

SqliteWrapperTable::SqliteWrapperTable(const SqliteWrapperTable &rTable)
{
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<SqliteWrapperTable&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
}

SqliteWrapperTable::SqliteWrapperTable(char **paszResults, int nRows, int nCols)
{
	mpaszResults = paszResults;
	mnRows = nRows;
	mnCols = nCols;
	mnCurrentRow = 0;
}

SqliteWrapperTable::~SqliteWrapperTable()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
	}
}

SqliteWrapperTable &SqliteWrapperTable::operator=(const SqliteWrapperTable &rTable)
{
	try
	{
		Finalize();
	}
	catch (...)
	{
	}
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<SqliteWrapperTable&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
	return *this;
}

int SqliteWrapperTable::NumFields() const
{
	CheckResults();
	return mnCols;
}

int SqliteWrapperTable::NumRows() const
{
	CheckResults();
	return mnRows;
}

const char *SqliteWrapperTable::FieldName(int nCol) const
{
	CheckResults();

	if (nCol < 0 || nCol > mnCols - 1)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
	}

	return mpaszResults[nCol];
}

const char *SqliteWrapperTable::FieldValue(int nField) const
{
	CheckResults();

	if (nField < 0 || nField > mnCols - 1)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field index requested",DONT_DELETE_MSG);
	}

	int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
	return mpaszResults[nIndex];
}

const char *SqliteWrapperTable::FieldValue(const char *szField) const
{
	CheckResults();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			if (strcmp(szField, mpaszResults[nField]) == 0)
			{
				int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
				return mpaszResults[nIndex];
			}
		}
	}

	throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid field name requested",DONT_DELETE_MSG);
}

int SqliteWrapperTable::GetIntField(int nField, int nNullValue) const
{
	if (FieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(FieldValue(nField));
	}
}

int SqliteWrapperTable::GetIntField(const char *szField, int nNullValue) const
{
	if (FieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(FieldValue(szField));
	}
}

double SqliteWrapperTable::GetFloatField(int nField, double fNullValue) const
{
	if (FieldIsNull(nField))
	{
		return fNullValue;
	}
	else
	{
		return atof(FieldValue(nField));
	}
}

double SqliteWrapperTable::GetFloatField(const char *szField, double fNullValue) const
{
	if (FieldIsNull(szField))
	{
		return fNullValue;
	}
	else
	{
		return atof(FieldValue(szField));
	}
}

const char *SqliteWrapperTable::GetStringField(int nField, const char *szNullValue) const
{
	if (FieldIsNull(nField))
	{
		return szNullValue;
	}
	else
	{
		return FieldValue(nField);
	}
}

const char *SqliteWrapperTable::GetStringField(const char *szField, const char *szNullValue) const
{
	if (FieldIsNull(szField))
	{
		return szNullValue;
	}
	else
	{
		return FieldValue(szField);
	}
}

bool SqliteWrapperTable::FieldIsNull(int nField) const
{
	CheckResults();
	return (FieldValue(nField) == 0);
}

bool SqliteWrapperTable::FieldIsNull(const char *szField) const
{
	CheckResults();
	return (FieldValue(szField) == 0);
}

void SqliteWrapperTable::SetRow(int nRow)
{
	CheckResults();

	if (nRow < 0 || nRow > mnRows - 1)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid row index requested",DONT_DELETE_MSG);
	}

	mnCurrentRow = nRow;
}

void SqliteWrapperTable::Finalize()
{
	if (mpaszResults)
	{
		sqlite3_free_table(mpaszResults);
		mpaszResults = 0;
	}
}

void SqliteWrapperTable::CheckResults() const
{
	if (mpaszResults == 0)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Null Results pointer",DONT_DELETE_MSG);
	}
}

SqliteWrapperStatement::SqliteWrapperStatement()
{
	mpDB = 0;
	mpVM = 0;
}

SqliteWrapperStatement::SqliteWrapperStatement(const SqliteWrapperStatement &rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<SqliteWrapperStatement&>(rStatement).mpVM = 0;
}

SqliteWrapperStatement::SqliteWrapperStatement(sqlite3 *pDB, sqlite3_stmt *pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}

SqliteWrapperStatement::~SqliteWrapperStatement()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
	}
}

SqliteWrapperStatement &SqliteWrapperStatement::operator=(const SqliteWrapperStatement &rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<SqliteWrapperStatement&>(rStatement).mpVM = 0;
	return *this;
}

int SqliteWrapperStatement::ExecDML()
{
	CheckDB();
	CheckVM();

	const char* szError = 0;

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(mpDB);

		nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			szError = sqlite3_errmsg(mpDB);
			throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
		}

		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		szError = sqlite3_errmsg(mpDB);
		throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}

SqliteWrapperQuery SqliteWrapperStatement::ExecQuery()
{
	CheckDB();
	CheckVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return SqliteWrapperQuery(mpDB, mpVM, true/*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return SqliteWrapperQuery(mpDB, mpVM, false/*eof*/, false);
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		const char* szError = sqlite3_errmsg(mpDB);
		throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Bind(int nParam, const char *szValue)
{
	CheckVM();
	int nRes = sqlite3_bind_text(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding string param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Bind(int nParam, const int nValue)
{
	CheckVM();
	int nRes = sqlite3_bind_int(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding int param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Bind(int nParam, const long long nValue)
{
	CheckVM();
	int nRes = sqlite3_bind_int64(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding int64 param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Bind(int nParam, const double dwValue)
{
	CheckVM();
	int nRes = sqlite3_bind_double(mpVM, nParam, dwValue);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding double param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Bind(int nParam, const unsigned char *blobValue, int nLen)
{
	CheckVM();
	int nRes = sqlite3_bind_blob(mpVM, nParam,
		(const void*)blobValue, nLen, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding blob param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::BindNull(int nParam)
{
	CheckVM();
	int nRes = sqlite3_bind_null(mpVM, nParam);

	if (nRes != SQLITE_OK)
	{
		throw SqliteWrapperException(nRes,"Error binding NULL param",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::Reset()
{
	if (mpVM)
	{
		int nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
			throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}

void SqliteWrapperStatement::Finalize()
{
	if (mpVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = 0;

		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
			throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}

void SqliteWrapperStatement::CheckDB() const
{
	if (mpDB == 0)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Database not open",DONT_DELETE_MSG);
	}
}

void SqliteWrapperStatement::CheckVM() const
{
	if (mpVM == 0)
	{
		throw SqliteWrapperException(SQLITEWRAPPERERROR,"Null Virtual Machine pointer",DONT_DELETE_MSG);
	}
}

SqliteWrapperDB::SqliteWrapperDB()
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
}

SqliteWrapperDB::SqliteWrapperDB(const SqliteWrapperDB &db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}

SqliteWrapperDB::~SqliteWrapperDB()
{
	Close();
}

void SqliteWrapperDB::Open(const char *szFile)
{
	int nRet = sqlite3_open(szFile, &mpDB);

	if (nRet != SQLITE_OK)
	{
		const char* szError = sqlite3_errmsg(mpDB);
		//throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
		printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
	}

	SetBusyTimeout(mnBusyTimeoutMs);
}

void SqliteWrapperDB::Close()
{
	if (mpDB)
	{
		sqlite3_close(mpDB);
		mpDB = 0;
	}
}

bool SqliteWrapperDB::TableExists(const char *szTable)
{
	SqliteWrapperBuffer sql;
	sql.format("select count(*) from sqlite_master where type='table' and name=%Q",
		szTable);
	int nRet = ExecScalar(sql);
	return (nRet > 0);
}

int SqliteWrapperDB::ExecDML(const char *szSQL)
{
	CheckDB();

	char* szError = 0;

	int nRet = sqlite3_exec(mpDB, szSQL, 0, 0, &szError);

	if (nRet == SQLITE_OK)
	{
		return sqlite3_changes(mpDB);
	}
	else
	{
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, szError);
		return -1;
		//throw SqliteWrapperException(nRet, szError);
	}
}

SqliteWrapperQuery SqliteWrapperDB::ExecQuery(const char *szSQL)
{
	CheckDB();

	sqlite3_stmt* pVM = Compile(szSQL);

	int nRet = sqlite3_step(pVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return SqliteWrapperQuery(mpDB, pVM, true/*eof*/);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return SqliteWrapperQuery(mpDB, pVM, false/*eof*/);
	}
	else
	{
		nRet = sqlite3_finalize(pVM);
		const char* szError = sqlite3_errmsg(mpDB);
		//throw SqliteWrapperException(nRet, (char*)szError, DONT_DELETE_MSG);
		printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
		return SqliteWrapperQuery();
	}
}

int SqliteWrapperDB::ExecScalar(const char *szSQL)
{
	SqliteWrapperQuery q = ExecQuery(szSQL);

	if (q.Eof() || q.NumFields() < 1)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Invalid scalar query",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Invalid scalar query");
		return -1;
	}
	return atoi(q.FieldValue(0));
}

SqliteWrapperTable SqliteWrapperDB::GetTable(const char *szSQL)
{
	CheckDB();

	char* szError = 0;
	char** paszResults = 0;
	int nRet;
	int nRows(0);
	int nCols(0);

	nRet = sqlite3_get_table(mpDB, szSQL, &paszResults, &nRows, &nCols, &szError);

	if (nRet == SQLITE_OK)
	{
		return SqliteWrapperTable(paszResults, nRows, nCols);
	}
	else
	{
		//throw SqliteWrapperException(nRet, szError);
		printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
		return SqliteWrapperTable();
	}
}

SqliteWrapperStatement SqliteWrapperDB::CompileStatement(const char *szSQL)
{
	CheckDB();

	sqlite3_stmt* pVM = Compile(szSQL);
	return SqliteWrapperStatement(mpDB, pVM);
}

sqlite_int64 SqliteWrapperDB::LastRowId() const
{
	return sqlite3_last_insert_rowid(mpDB);
}

void SqliteWrapperDB::SetBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	sqlite3_busy_timeout(mpDB, mnBusyTimeoutMs);
}

static const char* SQLiteVersion() { return SQLITE_VERSION; }

SqliteWrapperDB &SqliteWrapperDB::operator=(const SqliteWrapperDB &db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

sqlite3_stmt *SqliteWrapperDB::Compile(const char *szSQL)
{
	CheckDB();

	char* szError = 0;
	const char* szTail = 0;
	sqlite3_stmt* pVM;

	int nRet = sqlite3_prepare(mpDB, szSQL, -1, &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{
		//throw SqliteWrapperException(nRet, szError);
		printf("[%s][%d]:%d,%s\n", __FUNCTION__, __LINE__, nRet, szError);
		return NULL;
	}

	return pVM;
}

void SqliteWrapperDB::CheckDB() const
{
	if (!mpDB)
	{
		//throw SqliteWrapperException(SQLITEWRAPPERERROR,"Database not open",DONT_DELETE_MSG);
		printf("[%s][%d]:%s\n", __FUNCTION__, __LINE__, "Database not open");
	}
}

int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out) 
{
	int i, j, e, m;
	int cnt[256];
	if (n <= 0) 
	{
		out[0] = 'x';
		out[1] = 0;
		return 1;
	}
	memset(cnt, 0, sizeof(cnt));
	for (i = n - 1; i >= 0; i--) { cnt[in[i]]++; }
	m = n;
	for (i = 1; i<256; i++) 
	{
		int sum;
		if (i == '\'') continue;
		sum = cnt[i] + cnt[(i + 1) & 0xff] + cnt[(i + '\'') & 0xff];
		if (sum<m) 
		{
			m = sum;
			e = i;
			if (m == 0) break;
		}
	}
	out[0] = e;
	j = 1;
	for (i = 0; i<n; i++) 
	{
		int c = (in[i] - e) & 0xff;
		if (c == 0) 
		{
			out[j++] = 1;
			out[j++] = 1;
		}
		else if (c == 1) 
		{
			out[j++] = 1;
			out[j++] = 2;
		}
		else if (c == '\'') 
		{
			out[j++] = 1;
			out[j++] = 3;
		}
		else 
		{
			out[j++] = c;
		}
	}
	out[j] = 0;
	return j;
}

int sqlite3_decode_binary(const unsigned char *in, unsigned char *out) 
{
	int i, c, e;
	e = *(in++);
	i = 0;
	while ((c = *(in++)) != 0) 
	{
		if (c == 1) 
		{
			c = *(in++);
			if (c == 1) 
			{
				c = 0;
			}
			else if (c == 2) 
			{
				c = 1;
			}
			else if (c == 3) 
			{
				c = '\'';
			}
			else 
			{
				return -1;
			}
		}
		out[i++] = (c + e) & 0xff;
	}
	return i;
}