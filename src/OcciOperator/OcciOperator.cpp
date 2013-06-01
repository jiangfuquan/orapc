
#include "OcciOperator.h"

#include <stdlib.h>

#define OCCI_TRY try{
#define OCCI_CATCH(ret) }\
catch(SQLException &excp)\
{\
    wchar_t str[10240];\
    mbstowcs(str, excp.what(), 10240);\
    ErrorMessage = wstring(str);\
    return ret;\
}\
catch(std::wstring &str)\
{\
    ErrorMessage = str;\
    return ret;\
}\
catch(wchar_t *str)\
{\
    ErrorMessage = str;\
    return ret;\
}\
catch(...)\
{\
    ErrorMessage = L"未知错误";\
    return ret;\
}

DWORD WINAPI ClipProc(LPVOID pPara)
{
	StatelessConnectionPool *pPool = (StatelessConnectionPool*)(((LPVOID*)pPara)[0]);
	std::wstring sqlStr = *(std::wstring*)(((LPVOID*)pPara)[1]);
	int row = *(int*)(((LPVOID*)pPara)[2]);
	double x1 = *(double*)(((LPVOID*)pPara)[2]);
	double x2 = *(double*)(((LPVOID*)pPara)[3]);
	double y1 = *(double*)(((LPVOID*)pPara)[4]);
	double y2 = *(double*)(((LPVOID*)pPara)[5]);

	try{
		Connection *pConnection = pPool->getConnection();
		Statement *pStatement = pConnection->createStatement();
		pStatement->setSQL(OcciOperator::WcsToMbs(sqlStr));
		pStatement->setInt(1, row+1);
		pStatement->setDouble(2, x1);
		pStatement->setDouble(3, y1);

		pStatement->setDouble(4, x2);
		pStatement->setDouble(5, y2);
		pStatement->execute();
		pConnection->commit();

		pConnection->terminateStatement(pStatement);
		pPool->releaseConnection(pConnection);
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

OcciOperator* OcciOperator::pInstance = NULL;
OcciOperator::Garbo OcciOperator::garbo;

OcciOperator* OcciOperator::GetInstance()
{
    if(pInstance == NULL)
        pInstance = new OcciOperator;
    return pInstance;
}

OcciOperator::Garbo::~Garbo()
{
    delete pInstance;
}

OcciOperator::OcciOperator()
{
    ErrorMessage = L"";
    pEnvironment = NULL;
    pConnectionPool = NULL;
	pCommonConnection = NULL;
    pEnvironment = Environment::createEnvironment(Environment::OBJECT);
}

OcciOperator::~OcciOperator()
{
	if (pCommonConnection != NULL)
		pConnectionPool->releaseConnection(pCommonConnection);
    if(pConnectionPool != NULL)
        pEnvironment->terminateStatelessConnectionPool(pConnectionPool);
    if(pEnvironment != NULL)
        Environment::terminateEnvironment(pEnvironment);
}

const std::wstring OcciOperator::GetErrorMessage()
{
    std::wstring str = ErrorMessage;
    ErrorMessage.clear();
    return str;
}

bool OcciOperator::Connect(const wstring &ConnectString,
        const wstring &UserName, const wstring &Password)
{
	OCCI_TRY;
	char un[1024];
	char pw[1024];
	char cs[1024];
	wcstombs(cs, ConnectString.c_str(), 1024);
	wcstombs(un, UserName.c_str(), 1024);
	wcstombs(pw, Password.c_str(), 1024);
	pConnectionPool = pEnvironment->createStatelessConnectionPool(un, pw, cs);
	pCommonConnection = pConnectionPool->getConnection();
	OCCI_CATCH(false);
	return true;
}

bool OcciOperator::IsConnected()
{
    if(pConnectionPool == NULL)
        return false;
    else
        return true;
}

bool OcciOperator::DisConnect()
{
    OCCI_TRY;
	if (pCommonConnection != NULL);
	{
		pConnectionPool->releaseConnection(pCommonConnection);
		pCommonConnection = NULL;
	}
    if(pConnectionPool != NULL)
    {
        pEnvironment->terminateStatelessConnectionPool(pConnectionPool);
        pConnectionPool = NULL;
    }
    OCCI_CATCH(false);
    return true;
}

bool OcciOperator::InitDatabase(const wstring &ConnectString,
        const wstring &SysUser, const wstring &Password)
{
    ErrorMessage = L"this function is not completely";
	return false;
}

bool OcciOperator::ClipPC( const std::wstring &projName, double x1, double x2, double y1, double y2,double z1,double z2 )
{
	OCCI_TRY;
	std::wstring base = GetBaseTable(projName);
	std::wstring restst = GetReststTable(projName);
	DeleteTable(restst);

	//////////////////////////////////////////////////////////////////////////
	//查询行数，即PC对象数
	std::wstring sqlStr = L"select count(*) from "+base;
	Statement *pStatement = pCommonConnection->createStatement();
	pStatement->setSQL(WcsToMbs(sqlStr));
	ResultSet *pResultSet = pStatement->executeQuery();
	pResultSet->next();
	int n = int(pResultSet->getNumber (1));

	//////////////////////////////////////////////////////////////////////////
	//执行clip函数，切割数据到restst
	sqlStr = L"declare\n\
	inp sdo_pc;\n\
	begin\n\
	select pc  into inp from(select pc, rownum num from "+base+L")where num = :1;\n\
	insert into "+restst+L"\n\
	select * from table(sdo_pc_pkg.clip_pc\n\
	(inp,\n\
	sdo_geometry(2003, null, null,\n\
	mdsys.sdo_elem_info_array(1, 1003, 3),\n\
	mdsys.sdo_ordinate_array(:2, :3, :4, :5)),\n\
	null, null, null, null)\n\
	);\n\
	end;\n";

	//////////////////////////////////////////////////////////////////////////
	//遍历PC对象执行clip操作
	std::vector<HANDLE> hThreads;
	for(int i = 0; i < n; i++)
	{
		LPVOID para[] = {pConnectionPool, &sqlStr, &i, &x1, &x2, &y1, &y2};
		HANDLE hc = CreateThread(NULL, 0, ClipProc, para, 0, NULL);//在窗口模式中需要使用线程防止假死
		hThreads.push_back(hc);
	}

	WaitForMultipleObjects(hThreads.size(), &hThreads.front(), TRUE, -1);

	OCCI_CATCH(false);
	return true;
}

bool OcciOperator::DeleteTable(std::wstring tableName)
{
	OCCI_TRY;
	//std::string sqlStr = "DELETE FROM "+tableName;
	std::wstring sqlStr = L"TRUNCATE TABLE "+tableName;
	Statement *pStatement = pCommonConnection->createStatement();
	pStatement->setSQL(WcsToMbs(sqlStr));
	pStatement->execute();
	pCommonConnection->commit();
	OCCI_CATCH(false);
	return true;
}

std::wstring OcciOperator::GetBaseTable( const std::wstring &projName )
{
	std::wstring wtableName = L"";
	OCCI_TRY;
	Statement *pStatement = pCommonConnection->createStatement();
	pStatement->setSQL("SELECT BASETAB FROM PROJECTION WHERE NAME = :1");
	pStatement->setString(1, WcsToMbs(projName));
	ResultSet *pResultSet = pStatement->executeQuery();
	if (!pResultSet->next())
		throw L"项目不存在";

	std::string tableName = pResultSet->getString(1);
	wtableName = MbsToWcs(tableName);
	OCCI_CATCH(wtableName);
	return wtableName;
}

std::wstring OcciOperator::GetReststTable( const std::wstring &projName )
{
	std::wstring wtableName = L"";
	OCCI_TRY;
	Statement *pStatement = pCommonConnection->createStatement();
	pStatement->setSQL("SELECT RESTST FROM PROJECTION WHERE NAME = :1");
	pStatement->setString(1, WcsToMbs(projName));
	ResultSet *pResultSet = pStatement->executeQuery();
	if (!pResultSet->next())
		throw L"项目不存在";

	std::string tableName = pResultSet->getString(1);
	wtableName = MbsToWcs(tableName);

	OCCI_CATCH(wtableName);
	return wtableName;
}

std::wstring OcciOperator::MbsToWcs( const std::string &str )
{
	wchar_t *wstr = new wchar_t[str.length()+1];
	mbstowcs(wstr, str.c_str(), str.length()+1);
	std::wstring res = wstr;
	delete[] wstr;
	return res;
}

std::string OcciOperator::WcsToMbs( const std::wstring &wstr )
{
	char *str = new char[wstr.length()*2+1];
	wcstombs(str, wstr.c_str(), wstr.length()*2+1);
	std::string res = str;
	delete[] str;
	return res;
}


