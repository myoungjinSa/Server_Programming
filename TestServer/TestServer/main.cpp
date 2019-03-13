#include "global.h"
#include "ServerFramework.h"

int main()
{
	CServerFramework Server;


	while(true)
	{
		Server.AcceptClient();
	}

}