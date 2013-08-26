#include "stdafx.h"
#include "PeDump.h"
#include <map>
#include <vector>

#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER 240
#define GetImgDirEntryRVA( pNTHdr, IDE ) \
	(pNTHdr->OptionalHeader.DataDirectory[IDE].VirtualAddress)
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

PBYTE g_pMappedFileBase = 0;

//================================================================================
//
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
//
template <class T> PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, T* pNTHeader)	// 'T' == PIMAGE_NT_HEADERS 
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
	unsigned i;

	for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
	{
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		DWORD size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;

		// Is the RVA within this section?
		if ( (rva >= section->VirtualAddress) && 
			(rva < (section->VirtualAddress + size)))
			return section;
	}

	return 0;
}


template <class T> LPVOID GetPtrFromRVA( DWORD rva, T* pNTHeader, PBYTE imageBase ) // 'T' = PIMAGE_NT_HEADERS 
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;

	pSectionHdr = GetEnclosingSectionHeader( rva, pNTHeader );
	if ( !pSectionHdr )
		return 0;
	delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}

template <class T> std::vector<std::string>  DumpImportsSection(PBYTE pImageBase, T * pNTHeader)	// 'T' = PIMAGE_NT_HEADERS 
{
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	DWORD importsStartRVA;
	std::vector<std::string>  result;
	// Look up where the imports section is (normally in the .idata section)
	// but not necessarily so.  Therefore, grab the RVA from the data dir.
	importsStartRVA = GetImgDirEntryRVA(pNTHeader,IMAGE_DIRECTORY_ENTRY_IMPORT);
	if ( !importsStartRVA )
		return result;

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRVA(importsStartRVA,pNTHeader,pImageBase);
	if ( !pImportDesc )
		return result;

	bool bIs64Bit = ( pNTHeader->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL64_HEADER );

	printf("Imports Table:\n");
	std::vector<std::string> dlls;
	while ( 1 )
	{
		// See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
		if ( (pImportDesc->TimeDateStamp==0 ) && (pImportDesc->Name==0) )
			break;
		std::string st;
		printf("  %s\n", GetPtrFromRVA(pImportDesc->Name, pNTHeader, pImageBase) );
		st = (char*)GetPtrFromRVA(pImportDesc->Name, pNTHeader, pImageBase);
		strlwr(const_cast<char*>(st.c_str()));
		dlls.push_back(st);
		pImportDesc++;   // advance to next IMAGE_IMPORT_DESCRIPTOR
	}
	printf("\n");
	return dlls;
}


std::vector<std::string> DumpExeFile( PIMAGE_DOS_HEADER dosHeader )
{
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_NT_HEADERS64 pNTHeader64;
	PBYTE pImageBase = (PBYTE)dosHeader;
	std::vector<std::string>  result;
	// Make pointers to 32 and 64 bit versions of the header.
	pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
		dosHeader->e_lfanew );

	pNTHeader64 = (PIMAGE_NT_HEADERS64)pNTHeader;

	// First, verify that the e_lfanew field gave us a reasonable
	// pointer, then verify the PE signature.
	if ( IsBadReadPtr( pNTHeader, sizeof(pNTHeader->Signature) ) )
	{
		printf("Not a Portable Executable (PE) EXE\n");
		return result;
	}

	if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
	{
		printf("Not a Portable Executable (PE) EXE\n");
		return result;
	}
	bool bIs64Bit = ( pNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC );

	return bIs64Bit
		? DumpImportsSection(pImageBase, pNTHeader64 )
		: DumpImportsSection(pImageBase, pNTHeader);

}

std::vector<std::string> DumpFile(LPWSTR filename)
{
	HANDLE hFile;
	HANDLE hFileMapping;
	PIMAGE_DOS_HEADER dosHeader;
	std::vector<std::string> result;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		printf("Couldn't open file with CreateFile()\n");
		return result;
	}

	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if ( hFileMapping == 0 )
	{
		CloseHandle(hFile);
		printf("Couldn't open file mapping with CreateFileMapping()\n");
		return result;
	}

	g_pMappedFileBase = (PBYTE)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0);
	if ( g_pMappedFileBase == 0 )
	{
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		printf("Couldn't map view of file with MapViewOfFile()\n");
		return result;
	}

	wprintf(L"Dump of file %s\n\n", filename);

	dosHeader = (PIMAGE_DOS_HEADER)g_pMappedFileBase;
	PIMAGE_FILE_HEADER pImgFileHdr = (PIMAGE_FILE_HEADER)g_pMappedFileBase;

	if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
			return DumpExeFile( dosHeader );
	}
	else
		printf("unrecognized file format\n");

	UnmapViewOfFile(g_pMappedFileBase);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);
	return result;
}