#include "Global.h"
#include "TestServer.h"
#include "Animation.h"


CAnimation::CAnimation()
	:m_fPosition{0.0f}
{

}


CAnimation::~CAnimation()
{

}
void CAnimation::LoadAnimationInfo()
{
	std::ifstream in("../Animation/AnimationFile.txt");

	int count = 0;
	in >> count;
	

	AnimationInfo* animInfo = new AnimationInfo[count];
	for (int i = 0; i < count; ++i) 
	{
		
		in >> animInfo[i].m_animNum;
		in >> animInfo[i].m_pstrName;
		// 애니메이션이 안되는 문제해결해야댐
		if (!strcmp(animInfo[i].m_pstrName, "ATK3") || !strcmp(animInfo[i].m_pstrName, "Digging")
			|| !strcmp(animInfo[i].m_pstrName, "Jump") || !strcmp(animInfo[i].m_pstrName, "RaiseHand")
			|| !strcmp(animInfo[i].m_pstrName, "Die2")
			)
		{
			animInfo[i].m_nType = ANIMATION_TYPE_ONCE;
		}
		in >> animInfo[i].m_fLength;

		animaionMap[std::string(animInfo[i].m_pstrName)] = &animInfo[i];
	}


}
bool CAnimation::DecideAnimation(SOCKETINFO& clients,const float& fSpeed,const float& fTimeElapsed,const UCHAR& key)
{
	bool b_update = false;
//	clients.
	if(fSpeed == 0.0f)
	{
		clients.player.animationNum = PLAYER_ANIMATION_NUM::IDLE;
		b_update = true;

	}
	else
	{
		if(key == VK_UP )
		{
			clients.player.animationNum = PLAYER_ANIMATION_NUM::RUN_FAST;
			b_update = true;
		}
		else if(key == VK_DOWN )
		{
			clients.player.animationNum = PLAYER_ANIMATION_NUM::RUNBACKWARD;
			b_update = true;
		}
	
	}
	return b_update;
}

void CAnimation::UpdateAnimation(SOCKETINFO& clients,float fTimeElapsed)
{

	
}