#include "Storm.h"
#include <Windows.h>

namespace {
  typedef void* (__stdcall *SMemAlloc_type)(u32 size, const char* filename, u32 line, int idk); // 401
  typedef u32(__stdcall *SMemFree_type)(void* ptr, const char* filename, u32 line, int idk); // 403
  typedef u32(__stdcall *SFileCloseFile_type)(SHANDLE hFile); // 253
  typedef u32(__stdcall *SFileGetFileSize_type)(SHANDLE hFile, u32* lpFileSizeHigh); // 265
  typedef u32(__stdcall *SFileOpenFileEx_type)(SHANDLE hArchive, const char* filename, u32 scope, SHANDLE* hFile); // 268
  typedef u32(__stdcall *SFileReadFile_type)(SHANDLE hFile, void* buffer, u32 toRead, u32* read, int idk); // 269
  typedef bool(__stdcall *SErrGetErrorStr_type)(u32 dwErrCode, char* buffer, u32 bufferchars); // 462
  typedef u32(__stdcall *SErrGetLastError_type)(); // 463
  typedef void(__stdcall *SErrSetLastError_type)(u32 dwErrCode); // 465
  SMemAlloc_type SMemAlloc_ptr = NULL;
  SMemFree_type SMemFree_ptr = NULL;
  SFileCloseFile_type SFileCloseFile_ptr = NULL;
  SFileGetFileSize_type SFileGetFileSize_ptr = NULL;
  SFileOpenFileEx_type SFileOpenFileEx_ptr = NULL;
  SFileReadFile_type SFileReadFile_ptr = NULL;
  SErrGetErrorStr_type SErrGetErrorStr_ptr = NULL;
  SErrGetLastError_type SErrGetLastError_ptr = NULL;
  SErrSetLastError_type SErrSetLastError_ptr = NULL;

  void loadStorm() {
    HMODULE storm = GetModuleHandle("storm.dll");
    SMemAlloc_ptr = (SMemAlloc_type)GetProcAddress(storm, MAKEINTRESOURCE(401));
    SMemFree_ptr = (SMemFree_type)GetProcAddress(storm, MAKEINTRESOURCE(403));
    SFileCloseFile_ptr = (SFileCloseFile_type)GetProcAddress(storm, MAKEINTRESOURCE(253));
    SFileGetFileSize_ptr = (SFileGetFileSize_type)GetProcAddress(storm, MAKEINTRESOURCE(265));
    SFileOpenFileEx_ptr = (SFileOpenFileEx_type)GetProcAddress(storm, MAKEINTRESOURCE(268));
    SFileReadFile_ptr = (SFileReadFile_type)GetProcAddress(storm, MAKEINTRESOURCE(269));
    SErrGetErrorStr_ptr = (SErrGetErrorStr_type)GetProcAddress(storm, MAKEINTRESOURCE(462));
    SErrGetLastError_ptr = (SErrGetLastError_type)GetProcAddress(storm, MAKEINTRESOURCE(463));
    SErrSetLastError_ptr = (SErrSetLastError_type)GetProcAddress(storm, MAKEINTRESOURCE(465));
  }
};


void* SMemAlloc(u32 size, const char* filename, u32 line, int idk) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SMemAlloc_ptr(size, filename, line, idk);
}
u32 SMemFree(void* ptr, const char* filename, u32 line, int idk) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SMemFree_ptr(ptr, filename, line, idk);
}
u32 SFileCloseFile(SHANDLE hFile) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SFileCloseFile_ptr(hFile);
}
u32 SFileGetFileSize(SHANDLE hFile, u32* lpFileSizeHigh) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SFileGetFileSize_ptr(hFile, lpFileSizeHigh);
}
u32 SFileOpenFileEx(SHANDLE hArchive, const char* filename, u32 scope, SHANDLE* hFile) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SFileOpenFileEx_ptr(hArchive, filename, scope, hFile);
}
u32 SFileReadFile(SHANDLE hFile, void* buffer, u32 toRead, u32* read, int idk) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SFileReadFile_ptr(hFile, buffer, toRead, read, idk);
}

bool SErrGetErrorStr(u32 dwErrCode, char* buffer, u32 bufferchars) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SErrGetErrorStr_ptr(dwErrCode, buffer, bufferchars);
}
u32 SErrGetLastError() {
  if (SMemAlloc_ptr == NULL) loadStorm();
  return SErrGetLastError_ptr();
}
void SErrSetLastError(u32 dwErrCode) {
  if (SMemAlloc_ptr == NULL) loadStorm();
  SErrSetLastError_ptr(dwErrCode);
}

const u32 Func_FastFileRead = 0x004D2D10;
void* FastFileRead(const char* filename, void* buffer, bool returnIfNotFound, u32 scope, u32* filesize) {
  char dbg_filename[] = "Storm.cpp"; // __FILE__
  static u32 dbg_filename_int = (u32)(dbg_filename);
  static u32 returnIfNotFound_int;
  returnIfNotFound_int = returnIfNotFound;
  static void* result;

  __asm {
    PUSHAD
    
    PUSH __LINE__
    PUSH dbg_filename_int
    PUSH returnIfNotFound_int
    PUSH buffer
    PUSH filename
    MOV ECX, scope
    MOV EAX, filesize
    CALL Func_FastFileRead

    MOV result, EAX

    POPAD
  }
  return result;
}
