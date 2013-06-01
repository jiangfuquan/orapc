
#ifndef _OCCIOPERATOR_H_
#define _OCCIOPERATOR_H_

#include <vector>
#include <string>
using namespace std;

#include <Windows.h>

#include "occi.h"
using namespace oracle::occi;

class OcciOperator
{
private:
    OcciOperator();
    ~OcciOperator();
    static OcciOperator *pInstance;
public:
    static OcciOperator* GetInstance();
public:
    const std::wstring GetErrorMessage();
    bool Connect(const wstring &ConnectString, const wstring &UserName, 
            const wstring &Password);
    bool IsConnected();
    bool DisConnect();
public:
    bool InitDatabase(const wstring &ConnectString, const wstring &SysUser,
            const wstring &Password);
	bool ClipPC(const std::wstring &projName, double x1, double x2, double y1, double y2,double z1,double z2);
	bool DeleteTable(std::wstring tableName);
	std::wstring GetBaseTable( const std::wstring &projName );
	std::wstring GetReststTable( const std::wstring &projName );

public:
	static std::wstring MbsToWcs(const std::string &str);
	static std::string WcsToMbs(const std::wstring &wstr);

private:
    std::wstring ErrorMessage;
    Environment *pEnvironment;
    StatelessConnectionPool *pConnectionPool;
	Connection *pCommonConnection;
private:
    class Garbo
    {
    public:
        ~Garbo();
    };
    static Garbo garbo;
};

#endif

