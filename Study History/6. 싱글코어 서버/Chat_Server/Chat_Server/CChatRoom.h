#pragma once
#include <Windows.h>
#include <string>
#include <list>
class CChatRoom {
public:
	DWORD _dwRoomID;
	std::wstring _wsTitle;
	std::list<DWORD> _userList;

};

