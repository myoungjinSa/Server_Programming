
#include "stdafx.h"
#include "MainDB.h"

int main()
{

	auto db = std::make_unique<MainDB>();

	if (db->Initialize())
		db->Connect_DB();
	else
		std::cout << "��� ���� ����" << std::endl;
}