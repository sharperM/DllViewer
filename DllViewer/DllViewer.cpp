// DllViewer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "PeDump.h"
#include <Shlwapi.h >
#include <set>
#include <sstream>
#include <algorithm>

struct ExeDlls {
	std::wstring exe;
	std::map<std::string, bool> dlls;
};

bool CmpExeDlls(const ExeDlls& v1, const ExeDlls& v2)
{
	for (auto iter1 = v1.dlls.begin(), iter2 = v2.dlls.begin(), iend1 = v1.dlls.end();
		iter1!=iend1; ++iter1, ++iter2)
	{
		if (!iter1->second && iter2->second)
		{
			return true;
		}
		else if (iter1->second && !iter2->second)
		{
			return false;
		}
	}
	return false;
}


std::map<std::wstring,std::vector<std::string>> fun( const TCHAR* pszPath );
std::string WChar2Ansi(LPCWSTR pwszSrc);

//������ ΪҪ���ҵ�Ŀ¼·��
int _tmain(int argc, _TCHAR* argv[])
{
	std::map<std::wstring,std::vector<std::string>> exeDlls;
	std::map<std::string,int> allDlls;

	if (argc >1)
	{
		exeDlls = fun( argv[1] );
	}
	else
	{
		printf("����̫����");
	}
	for ( std::map<std::wstring,std::vector<std::string>>::iterator it = exeDlls.begin(),itend = exeDlls.end();
		it != itend; ++it)
	{
		for (int i = 0,c = it->second.size();i<c;++i)
		{
			allDlls.insert(std::pair<std::string,int>(it->second[i],0));
		}
	}
	
	std::vector<bool> isDllUsed; //��¼exe�õ�����Щdll��û�õ���Щdll
	for (std::map<std::string,int>::iterator i = allDlls.begin(),ic = allDlls.end();i!=ic;++i )
	{
		for (std::map<std::wstring,std::vector<std::string>>::iterator j = exeDlls.begin(), jc = exeDlls.end();j!=jc;++j)
		{
			int jj = 0, jjc = j->second.size();
			for ( ;jj<jjc;jj++)
			{
				if (j->second[jj]==(i->first))
				{
					isDllUsed.push_back(true);
					break;	
				}
			}
			if (jj == jjc)
			{
				isDllUsed.push_back(false);
			}
		}
	}
	//���� csv
	std::string strCsvContent=",";

	int nIndexDll = 0, nIndexExe = 0;
	
	for (std::map<std::wstring,std::vector<std::string>>::iterator iterED = exeDlls.begin(), iterEDend = exeDlls.end();iterED!=iterEDend;++iterED)
	{//��һ��
		strCsvContent += WChar2Ansi(iterED->first.c_str());
		strCsvContent += ",";
	}
	strCsvContent += "\r\n";
	for ( std::map<std::string,int>::iterator iterDll =  allDlls.begin(), iterDllend =  allDlls.end();iterDll!=iterDllend;iterDll++ )
	{
		int dllBeUseCount= 0;
		nIndexExe = 0;
		for (std::map<std::wstring,std::vector<std::string>>::iterator iterED = exeDlls.begin(), iterEDend = exeDlls.end();iterED!=iterEDend;++iterED)
		{
			if  ( iterED == exeDlls.begin() )
			{
				//��һ��
				strCsvContent += iterDll->first;
				strCsvContent += ",";
			}
			if (isDllUsed[(nIndexExe)+(exeDlls.size())*(nIndexDll)] == true)
			{
					strCsvContent+="1";
					++dllBeUseCount;
			}
			strCsvContent += ",";
			nIndexExe++;
		}
		nIndexDll++;

		iterDll->second = dllBeUseCount;
		strCsvContent+="\r\n";
	}

	const char * filename = ".\\exeDependDllList.csv";
	std::ofstream fs(filename,std::ios::binary|std::ios::out);
	fs<<strCsvContent;

	fs.flush();
	fs.close();
	ShellExecuteA(NULL, "open", filename, NULL, NULL, SW_SHOW);
	return 0;
}


//return <�������֣�ʹ�õ�dll>
std::map<std::wstring,std::vector<std::string>> fun( const TCHAR* pszPath )
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	std::wstring sz;
	sz = pszPath;
	sz = sz+L"\\*.*";
	std::map<std::wstring,std::vector<std::string>>  allExeDlls;
	hFind = FindFirstFile( sz.c_str(), &findFileData );
	if ( hFind == INVALID_HANDLE_VALUE )
	{
		wprintf( L"·������\n\"%s\" û���ҵ�\n", pszPath );
	}
	else
	{
		std::wstring tmepName;
		std::wstring path = pszPath;
		while( FindNextFile( hFind, &findFileData ) )
		{
			if ( findFileData.cFileName[0]!= '.' )
			{
				if (findFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					std::wstring nextDir = pszPath;
					nextDir = nextDir+L"\\"+findFileData.cFileName;
					return fun(nextDir.c_str());
				}
				else 
				{
					tmepName =  wcslwr(findFileData.cFileName);
					if (std::wstring::npos != tmepName.find(L".exe"))
					{
						TCHAR tempPath[MAX_PATH];
						std::wstring ttt = path+L"\\"+tmepName;
						memcpy(tempPath,ttt.c_str(),ttt.size()*2+2);
						allExeDlls[tmepName] = DumpFile(tempPath);
					}
				}
			}
		}
	}
	return allExeDlls;
}


std::string WChar2Ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen<= 0) return std::string("");
	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");
	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;
	std::string strTemp(pszDst);
	delete [] pszDst;
	return strTemp;
}
