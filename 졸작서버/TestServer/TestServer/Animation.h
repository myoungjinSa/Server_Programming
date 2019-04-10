#pragma once


#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_PINGPONG		2


struct AnimationInfo
{
	~AnimationInfo() { std::cout << "소멸" << std::endl; }
	int												m_animNum;

	char											m_pstrName[64];
	float											m_fLength ;
	int 											m_nType = ANIMATION_TYPE_LOOP ; //Once, Loop, PingPong
};

class CAnimation
{
	
	
	float 											m_fPosition ;
    
	std::string animationName;

public:
	explicit CAnimation();

	virtual ~CAnimation();
	void LoadAnimationInfo();
	void UpdateAnimation(SOCKETINFO& clients,float fTimeElapsed);
	bool DecideAnimation(SOCKETINFO& clients ,const float& fSpeed,const float& fTimeElapsed,const UCHAR& key);

public:
	enum PLAYER_ANIMATION_NUM { IDLE, JUMP,RUN_FAST,RUNBACKWARD,ATK3,Digging,Die2,RaiseHand,Ice,Victory,Aert,Slide};
	
	std::map<std::string, AnimationInfo*> animaionMap;		//애니메이션 
};