// DllViewer.cpp : 定义控制台应用程序的入口点。
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
int _tmain(int argc, _TCHAR* argv[])
{
	std::map<std::wstring,std::vector<std::string>> exeDlls;
	std::map<std::string,int> allDlls;
	std::set<std::string> allExes;
	if (argc >1)
	{
		exeDlls = fun( argv[1] );
	}
	for ( std::map<std::wstring,std::vector<std::string>>::iterator it = exeDlls.begin(),itend = exeDlls.end();
		it != itend; ++it)
	{
		for (int i = 0,c = it->second.size();i<c;++i)
		{
			allDlls.insert(std::pair<std::string,int>(it->second[i],0));
		}
		allExes.insert( WChar2Ansi(it->first.c_str()) );
	}
	std::vector<bool> talbe;
	for (std::map<std::string,int>::iterator i = allDlls.begin(),ic = allDlls.end();i!=ic;++i )
	{
		for (std::map<std::wstring,std::vector<std::string>>::iterator j = exeDlls.begin(), jc = exeDlls.end();j!=jc;++j)
		{
			int jj = 0, jjc = j->second.size();
			for ( ;jj<jjc;jj++)
			{
				if (j->second[jj]==(i->first))
				{
					talbe.push_back(true);
					break;	
				}
			}
			if (jj == jjc)
			{
				talbe.push_back(false);
			}
		}
	}
	std::string outppp;
	std::map<std::string,int>::iterator itDll = allDlls.begin();
	std::set<std::string>::iterator itExe = allExes.begin();
	for ( int i = 0, ic = allDlls.size()+1; i<ic; ++i )
	{
		int line= 0;
		for ( int j = 0, jc = exeDlls.size()+1; j < jc; ++j )
		{
			if ( i == 0 &&j>0)
			{
				
				outppp += ",";
				outppp += (*itExe++);
			}
			else if  (j == 0&&i > 0)
			{
				outppp += (itDll->first);
				
			}
			else if (i>0&&j>0)
			{
				outppp+=",";
				if (talbe[(j-1)+(jc-1)*(i-1)] == true)
				{
					outppp+="1";
					++line;
				}
				else
				{
					outppp+="";
				}
			}
		}
		if ( i > 0 &&itDll!=allDlls.end())
		{
			itDll->second = line;
			itDll++;
		}
		outppp+="\r\n";
	}
	std::vector<std::string> sortdlls;
	std::vector<std::string> sorrExes;
	for (std::map<std::string,int>::iterator it = allDlls.begin(),itend = allDlls.end();
		it!=itend; ++it)
	{
		sortdlls.push_back(it->first);
	}
	//void insertSort(Type* arr,long len)/*InsertSort algorithm*/
	{
//		void bubbleSort(int arr[],int n)
	
		int i,j;
		std::string t;
		for(i=0;i<sortdlls.size()-1;i++)
		{
			for(j=0;j<sortdlls.size()-i-1;j++)
			{
				if(allDlls[sortdlls[j+1]]>allDlls[sortdlls[j]])
				{
					t=sortdlls[j+1];
					sortdlls[j+1]=sortdlls[j];
					sortdlls[j]=t;
				}
			}
		}
	}
	for (int i = 0,c = sortdlls.size(); i<c; ++i)
	{
		outppp+=sortdlls[i]+std::string(",");
	}
	//for (int i = 0, c = sortdlls.size(); i<c; ++i)
	//{
	//	for (std::map<std::wstring,std::vector<std::string>>::iterator j = exeDlls.begin() ;j!=exeDlls.end();++j)
	//	{
	//		int  ii = 0,cc = j->second.size();
	//		for (; ii<cc; ++ii)
	//		{
	//			if (j->second[ii] == sortdlls[i] )
	//			{
	//				break;
	//			}
	//		}
	//		if (ii == cc)
	//		{
	//			sorrExes.push_back(WChar2Ansi(j->first.c_str()));
	//			exeDlls.erase(j);
	//			break;
	//		}
	//	}
	//}
	{
	/*	for (;sorrExes.size()<allExes.size() ;)
		{
			int ii = 0;
			for (std::map<std::wstring,std::vector<std::string>>::iterator j = exeDlls.begin() ;j!=exeDlls.end();++j)
			{
				int  i = 0,c = j->second.size();
				for (; i<c; ++i)
				{
					if (j->second[i] == sortdlls[1] )
					{
						break;
					}
				}
				if (i == c)
				{
					sorrExes.push_back(WChar2Ansi(j->first.c_str()));
					exeDlls.erase(j);
					break;
				}
				sortdlls.erase(sortdlls.begin());
			}
		}
		*/
	}

	std::vector<ExeDlls> sortedExeDlls;
	for (auto iter = exeDlls.begin(), iend = exeDlls.end();
		iter!=iend; ++iter)
	{
		ExeDlls ed;
		ed.exe = iter->first;
		for (auto iter2 = allDlls.begin(), iend2 = allDlls.end();
			iter2!=iend2; ++iter2)
		{
			ed.dlls[iter2->first] = false;
		}
		for (auto iter3 = iter->second.begin(), iend3 = iter->second.end();
			iter3!=iend3; ++iter3)
		{
			ed.dlls[*iter3] = true;
		}
		sortedExeDlls.push_back(ed);
	}

	std::stable_sort(sortedExeDlls.begin(), sortedExeDlls.end(), &CmpExeDlls);

	const char * filename = "C:\\Documents and Settings\\Administrator\\桌面\\新建 文本文档.csv";
	std::ofstream fs(filename,std::ios::binary|std::ios::out);
// 	std::ofstream fs("依赖的dll.csv",std::ios::binary);
	//fs.write(outppp.c_str(),outppp.size());
	// exe行
	for (auto iter = sortedExeDlls.begin(), iend = sortedExeDlls.end();
		iter!=iend; ++iter)
	{
		fs << "," << WChar2Ansi(iter->exe.c_str());
	}
	fs << "\r\n";
	// 各DLL行
	for (auto iter1 = sortedExeDlls.begin()->dlls.begin(), iend1 = sortedExeDlls.begin()->dlls.end();
		iter1!=iend1; ++iter1)
	{
		fs << iter1->first << ",";
		for (auto iter2 = sortedExeDlls.begin(), iend2 = sortedExeDlls.end();
			iter2!=iend2; ++iter2)
		{
			fs << (iter2->dlls[iter1->first]?"1":"") << ",";
		}
		fs << "\r\n";
	}
	fs.flush();
	fs.close();
	ShellExecuteA(NULL, "open", filename, NULL, NULL, SW_SHOW);
	//system("pause");
	return 0;
}

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
		wprintf( L"路径错误\n\"%s\" 没有找到\n", pszPath );
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
