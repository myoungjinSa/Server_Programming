
#include "stdafx.h"
#include "MainDB.h"

int main()
{

	auto db = std::make_unique<MainDB>();

	if (db->Initialize())
		db->Connect_DB();
	else
		std::cout << "디비 생성 실패" << std::endl;
}