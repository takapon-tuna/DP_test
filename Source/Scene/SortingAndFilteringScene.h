#pragma once

#include <vector>
#include "Scene.h"

// �\�[�g���t�B���^�����O�����V�[��
class SortingAndFilteringScene : public Scene
{
public:
	SortingAndFilteringScene();
	~SortingAndFilteringScene() override = default;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	// �t�B���^�����O
	void Filtering();
private:
	// ����
	static const int ATTR_NONE	= 0;
	static const int ATTR_WATER	= 1 << 0;
	static const int ATTR_FIRE	= 1 << 1;
	static const int ATTR_WIND	= 1 << 2;
	static const int ATTR_ALL	= ATTR_WATER | ATTR_FIRE | ATTR_WIND;

	// ���A���e�B
	enum
	{
		RARITY_N,
		RARITY_R,
		RARITY_SR,
		RARITY_SSR,
	};

	// �\�[�g�^�C�v
	enum
	{
		SORT_NONE,
		SORT_NAME,
		SORT_RARITY,
		SORT_PRICE,
		SORT_POWER,
	};

	// ����f�[�^
	struct Weapon
	{
		const char*		name;
		int				price;
		int				power;
		int				rarity;
		int				attr;
	};

	std::vector<Weapon>		weapons;
	std::vector<Weapon*>	displayWeapons;
	int						sortType = SORT_NONE;
	int						selectRarity = -1;
	int						filterAttr = 0;
};
