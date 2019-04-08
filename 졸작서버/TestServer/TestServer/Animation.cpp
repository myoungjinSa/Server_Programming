#include "Global.h"
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
	

	AnimationInfo* animInfo = new AnimationInfo[12];
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
void CAnimation::DecideAnimation(int& animationNum,float& fTimeElapsed)
{
	
}

void CAnimation::UpdateAnimation(float& fTimeElapsed)
{
	
}