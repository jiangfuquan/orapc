
#ifndef _OCCIOPERATOR_H_
#define _OCCIOPERATOR_H_

#include <vector>
#include <string>
using namespace std;

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
    bool DisConnnect();
public:
    bool InitDatabase(const wstring &ConnectString, const wstring &SysUser,
            const wstring &Password);
private:
    std::wstring ErrorMessage;
    Environment *pEnvironment;
    StatelessConnectionPool *pConnectionPool;
private:
    class Garbo
    {
    public:
        ~Garbo();
    }
    static Garbo garbo;
};

#endif

