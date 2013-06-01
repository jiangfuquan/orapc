
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
	//OCCI_TRY;
	bool ret = false;
	try{
		char un[1024];
		char pw[1024];
		char cs[1024];
		wcstombs(cs, ConnectString.c_str(), 1024);
		wcstombs(un, UserName.c_str(), 1024);
		wcstombs(pw, Password.c_str(), 1024);
		pConnectionPool = pEnvironment->createStatelessConnectionPool(un, pw, cs);
		//OCCI_CATCH(false);
	}
	catch(SQLException &excp)
	{
		wchar_t str[10240];
		mbstowcs(str, excp.what(), 10240);
		ErrorMessage = wstring(str);
		return ret;
	}
	catch(std::wstring &str)
	{
		ErrorMessage = str;
		return ret;
	}
	catch(wchar_t *str)
	{
		ErrorMessage = str;
		return ret;
	}
	catch(...)
	{
		ErrorMessage = L"Î´Öª´íÎó";
		return ret;
	}


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
        
