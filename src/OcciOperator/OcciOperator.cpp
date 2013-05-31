
#include "OcciOperator.h"

#define OCCI_TRY try{
#define OCCI_CATCH(ret) }\
catch(SQLException &excp)\
{\
    ErrorMessage = excp.what();\
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
    ErrorMessage = L"Î´Öª´íÎó";\
    return ret;\
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
    pEnvironment = Environment::createEnvironment(Environment::OBJECT);
}

OcciOperator::~OcciOperator()
{
    if(pOcciConnectionPool != NULL)
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
    pConnectionPool = pEnvironment->createStatelessConnectionPool(UserName, Password, ConnectString);
    OCCI_CATCH(false);
    return true;
}

bool OcciOperator::IsConnected()
{
    if(pConnectionPool == NULL)
        return false;
    else
        return ture;
}

bool OcciOperator::Disconnect()
{
    OCCI_TRY;
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
}
        
