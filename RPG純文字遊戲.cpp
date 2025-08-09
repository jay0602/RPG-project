#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <algorithm>
#include <map>
#include <sstream>

using namespace std;

// 顏色改變工具
void SetColor(int color = 7) {
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// 技能類型
enum SkillType {
    ATTACK,
    HEAL,
    BUFF,
    DEBUFF,
    BUFF_ATTACK,
    DEBUFF_ATTACK,
    UTILITY
};

// 傷害類型
enum DamageType {
    PHYSICAL,
    MAGICAL
};

// 狀態效果
enum StatusEffect {
    NONE,
    BURN,
    POISON,
    STUN,
    BLEED,
    WEAK,
    FROST,
    GUARD,
    RAGE,
    CALM,
    HUNGER,
    MEDITATION,
    EVADE_CHANCE
};

// 裝備稀有度
enum Rarity {
    COMMON,     // 白色 +0 (50%)
    UNCOMMON,   // 綠色 +1 (32%)
    RARE,       // 藍色 +3 (14%)
    EPIC,       // 紫色 +5 (3%)
    LEGENDARY   // 橘色 +9 (1%)
};

// 裝備類型
enum EquipmentType {
    NONE_EQUIP,
    HELMET,
    ARMOR,
    STAFF
};

// 裝備效果類型
enum EquipmentEffectType {
    HP_BONUS,
    MP_BONUS,
    HP_REGEN_BONUS,
    MP_REGEN_BONUS,
    EXP_BONUS,
    DEFENSE_BONUS,
    M_DEFENSE_BONUS,
    POISON_DAMAGE_BONUS,
    BURN_DAMAGE_BONUS,
    M_ATTACK_BONUS,
    DAMAGE_REFLECT,
    STATUS_DURATION_BONUS,
    CLEANSE_PER_TURN,
    M_ATTACK_WITH_STATUS
};

// 簡單隨機數產生器
int getRandomNumber(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    return rand() % (max - min + 1) + min;
}

// 效果結構體
struct Effect {
    EquipmentEffectType type;
    int value;

    // C++98 建構子
    Effect(EquipmentEffectType t, int v) : type(t), value(v) {}
};

// 裝備結構體
struct Equipment {
    string name;
    EquipmentType type;
    Rarity rarity;
    int rarityValue;
    vector<Effect> effects;
    int bonus; // 新增的額外隨機加成值
    
    // C++98 建構子
    Equipment(string n, EquipmentType et, Rarity r, int rv, const vector<Effect>& ef, int b = 0) : name(n), type(et), rarity(r), rarityValue(rv), effects(ef), bonus(b) {}
    Equipment() : name("無"), type(NONE_EQUIP), rarity(COMMON), rarityValue(0), bonus(0) {} // 預設空裝備
};

// 裝備模板結構體，用於更豐富的隨機生成
struct EquipmentTemplate {
    string name;
    EquipmentType type;
    EquipmentEffectType effectType;
    int baseStat;
    int bonusMin;
    int bonusMax;
};

// 狀態結構體
struct Status {
    StatusEffect type;
    int duration;
    int value;
    int turnCount;

    // C++98 建構子
    Status(StatusEffect t, int d, int v) : type(t), duration(d), value(v), turnCount(0) {}
};

// 技能結構體
struct Skill {
    string name;
    SkillType type;
    int mpCost;
    int baseDamage;
    int value;
    int hitRate;
    DamageType damageType;
    StatusEffect statusToInflict; 
    int statusDuration;
    int statusValue;

    // C++98 建構子，移除 'v' 參數
    Skill(string n, SkillType t, int mc, int bd, int v, int hr, DamageType dt, StatusEffect si, int sd, int sv) 
        : name(n), type(t), mpCost(mc), baseDamage(bd), value(v), hitRate(hr), damageType(dt), statusToInflict(si), statusDuration(sd), statusValue(sv) {}
};

// 敵人結構體
struct Enemy {
    string name;
    int hp;
    int maxHp;
    int expValue;
    int attack;
    int magicAttack;
    int hpRegen;
    vector<Status> activeStatuses;
    vector<Skill> skills;

    // Boss 專用變數
    int temperatureValue;
    int lawbreakerAbility; // 0:None, 1:NoStatus, 2:NoHPRegen, 3:NoMPRegen, 4:NoAttack
    int exileGauge;
    int damageTakenThisTurn;


    // C++98 建構子，初始化成員變數
    Enemy() : name(""), hp(0), maxHp(0), expValue(0), attack(0), magicAttack(0), hpRegen(0), 
              temperatureValue(50), lawbreakerAbility(0), exileGauge(0), damageTakenThisTurn(0) {}

    void addStatus(StatusEffect type, int duration, int value) {
        if (type == BLEED) {
            activeStatuses.push_back(Status(type, duration, value));
            return;
        }

        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == type) {
                // MODIFICATION: 讓憤怒狀態可以疊加
                if (type == RAGE) {
                    activeStatuses[i].duration = max(activeStatuses[i].duration, duration);
                    activeStatuses[i].value += value;
                } else {
                    activeStatuses[i].duration += duration;
                }
                return;
            }
        }
        activeStatuses.push_back(Status(type, duration, value));
    }

    void regenerate() {
        if (hp > 0) {
            hp += hpRegen;
            if (hp > maxHp) hp = maxHp;
        }
    }
};

// 玩家結構體
struct Player {
    int hp, maxHp;
    int mp, maxMp;
    int magicAttack;
    int defense;
    int magicDefense;
    int hpRegen;
    int mpRegen;

    int level;
    int exp;
    int expToNextLevel;

    // MODIFICATION: 新增成員變數來儲存升級獲得的額外屬性
    int bonusMaxHp;
    int bonusMaxMp;
    int bonusMagicAttack;
    int bonusDefense;
    int bonusMagicDefense;

    // 裝備欄位
    Equipment helmet;
    Equipment armor;
    Equipment staff;

    vector<Status> activeStatuses;
    vector<Skill> skills;
    const int MAX_SKILLS = 5;

    // C++98 建構子
    // MODIFICATION: 初始化新的額外屬性為 0
    Player() : hp(100), maxHp(100), mp(50), maxMp(50), magicAttack(10), defense(5), magicDefense(5),
               hpRegen(2), mpRegen(1), level(1), exp(0), expToNextLevel(30 + 20 * level),
               bonusMaxHp(0), bonusMaxMp(0), bonusMagicAttack(0), bonusDefense(0), bonusMagicDefense(0) {}

    // MODIFICATION: 更新函式以正確計算總屬性
    void calculateTotalStats() {
        // 1. 根據等級計算基礎屬性
        int baseMaxHp = 100 + (level - 1) * 10;
        int baseMaxMp = 50 + (level - 1) * 5;
        int baseMagicAttack = 10 + (level - 1);
        int baseDefense = 5 + (level - 1) * 2;
        int baseMagicDefense = 5 + (level - 1) * 2;
        int baseHpRegen = 2;
        int baseMpRegen = 1;

        // 2. 將升級獲得的額外屬性加到基礎屬性上
        maxHp = baseMaxHp + bonusMaxHp;
        maxMp = baseMaxMp + bonusMaxMp;
        magicAttack = baseMagicAttack + bonusMagicAttack;
        defense = baseDefense + bonusDefense;
        magicDefense = baseMagicDefense + bonusMagicDefense;
        hpRegen = baseHpRegen;
        mpRegen = baseMpRegen;

        // 3. 套用裝備的加成
        vector<Equipment*> equippedItems;
        equippedItems.push_back(&helmet);
        equippedItems.push_back(&armor);
        equippedItems.push_back(&staff);

        for (size_t i = 0; i < equippedItems.size(); ++i) {
            Equipment* item = equippedItems[i];
            if (item->type == NONE_EQUIP) continue;
            for (size_t j = 0; j < item->effects.size(); ++j) {
                Effect effect = item->effects[j];
                switch (effect.type) {
                    case HP_BONUS: maxHp += effect.value; break;
                    case MP_BONUS: maxMp += effect.value; break;
                    case HP_REGEN_BONUS: hpRegen += effect.value; break;
                    case MP_REGEN_BONUS: mpRegen += effect.value; break;
                    case DEFENSE_BONUS: defense += effect.value; break;
                    case M_DEFENSE_BONUS: magicDefense += effect.value; break;
                    case M_ATTACK_BONUS: magicAttack += effect.value; break;
                    default: break;
                }
            }
        }
    }

    void addStatus(StatusEffect type, int duration, int value) {
        if (type == BLEED) {
            activeStatuses.push_back(Status(type, duration, value));
            return;
        }
        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == type) {
                activeStatuses[i].duration += duration;
                return;
            }
        }
        activeStatuses.push_back(Status(type, duration, value));
    }

    void showStatus() {
        SetColor(14);
        cout << "\n----------------------------------------\n";
        cout << "            角色能力值              \n";
        cout << "----------------------------------------\n";
        cout << "等級: " << level << endl;
        cout << "經驗值: " << exp << "/" << expToNextLevel << endl;
        cout << "HP: " << hp << "/" << maxHp << endl;
        cout << "MP: " << mp << "/" << maxMp << endl;
        cout << "魔法攻擊力: " << magicAttack << endl;
        cout << "物理防禦力: " << defense << endl;
        cout << "魔法防禦力: " << magicDefense << endl;
        cout << "HP回復: " << hpRegen << "/回合" << endl;
        cout << "MP回復: " << mpRegen << "/回合" << endl;
        cout << "----------------------------------------\n";
        cout << "            當前狀態              \n";
        cout << "----------------------------------------\n";
        if (activeStatuses.empty()) {
            cout << "目前沒有任何狀態。\n";
        } else {
            for (size_t i = 0; i < activeStatuses.size(); ++i) {
                Status status = activeStatuses[i];
                string statusName;
                switch (status.type) {
                    case BURN: statusName = "灼傷"; break;
                    case POISON: statusName = "中毒"; break;
                    case STUN: statusName = "暈眩"; break;
                    case BLEED: statusName = "出血"; break;
                    case WEAK: statusName = "虛弱"; break;
                    case FROST: statusName = "寒冷"; break;
                    case GUARD: statusName = "守護"; break;
                    case RAGE: statusName = "憤怒"; break;
                    case CALM: statusName = "冷靜"; break;
                    case HUNGER: statusName = "飢餓"; break;
                    case MEDITATION: statusName = "冥想"; break;
                    case EVADE_CHANCE: statusName = "迴避"; break;
                    default: statusName = "未知狀態"; break;
                }
                cout << "- " << statusName << " (剩餘回合: " << status.duration << ")";
                if (status.type == POISON) cout << " 傷害: " << status.value + status.turnCount;
                if (status.type == BLEED) cout << " 額外傷害: " << status.value;
                if (status.type == WEAK) cout << " 易傷: " << status.value << "%";
                if (status.type == FROST) cout << " 傷害減免: " << status.value << "%";
                if (status.type == GUARD) cout << " 傷害減免: " << status.value << "%";
                if (status.type == RAGE) cout << " 傷害增加: " << status.value << "%";
                if (status.type == EVADE_CHANCE) cout << " 閃避率: " << status.value << "%";
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        displayEquipment();
        cout << "----------------------------------------\n";
        cout << "            擁有的技能              \n";
        cout << "----------------------------------------\n";
        if (skills.empty()) {
            cout << "沒有已學會的技能。\n";
        } else {
            for (size_t i = 0; i < skills.size(); ++i) {
                Skill currentSkill = skills[i];
                cout << i + 1 << ". 「" << currentSkill.name << "」 (MP消耗: " << currentSkill.mpCost << ")";
                if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
                    cout << ", 傷害: " << currentSkill.baseDamage << ", 命中: " << currentSkill.hitRate << "%";
                } else if (currentSkill.type == HEAL) {
                    cout << ", 治癒量: " << currentSkill.value;
                }
                if (currentSkill.statusToInflict != NONE) {
                    string statusName;
                    switch (currentSkill.statusToInflict) {
                        case BURN: statusName = "灼傷"; break;
                        case POISON: statusName = "中毒"; break;
                        case STUN: statusName = "暈眩"; break;
                        case BLEED: statusName = "出血"; break;
                        default: statusName = "未知狀態"; break;
                    }
                    cout << " (施加狀態: " << statusName << " " << currentSkill.statusDuration << "回合)";
                }
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        SetColor();
        system("pause");
    }

    // 顯示玩家的裝備
    void displayEquipment() {
        cout << "             裝備品              \n";
        cout << "----------------------------------------\n";
        cout << "頭盔: " << helmet.name << endl;
        cout << "盔甲: " << armor.name << endl;
        cout << "法杖: " << staff.name << endl;
        cout << "----------------------------------------\n";
    }

    // 新增：獲取裝備特定效果的總值
    int getEffectValue(const Equipment& item, EquipmentEffectType type) {
        int totalValue = 0;
        for (size_t i = 0; i < item.effects.size(); ++i) {
            if (item.effects[i].type == type) {
                totalValue += item.effects[i].value;
            }
        }
        return totalValue;
    }

    void equipItem(const Equipment& item) {
        SetColor(11);
        cout << "\n你獲得了新裝備！\n";
        
        Equipment oldItem;
        if (item.type == HELMET) oldItem = helmet;
        else if (item.type == ARMOR) oldItem = armor;
        else if (item.type == STAFF) oldItem = staff;
        
        cout << "\n----------------------------------------\n";
        cout << "            新裝備屬性              \n";
        cout << "----------------------------------------\n";
        
        // 修正後的顏色顯示邏輯
        cout << "名稱: 「" << item.name << "」 (";
        switch (item.rarity) {
            case COMMON: SetColor(15); cout << "白色"; break;
            case UNCOMMON: SetColor(10); cout << "綠色"; break;
            case RARE: SetColor(9); cout << "藍色"; break;
            case EPIC: SetColor(13); cout << "紫色"; break;
            case LEGENDARY: SetColor(6); cout << "橘色"; break;
        }
        SetColor(11); // 重設顏色
        cout << ")\n";
        
        cout << "類型: ";
        switch (item.type) {
            case HELMET: cout << "頭盔"; break;
            case ARMOR: cout << "盔甲"; break;
            case STAFF: cout << "法杖"; break;
        }
        cout << endl;
        cout << "屬性加成:\n";
        for (size_t i = 0; i < item.effects.size(); ++i) {
            Effect effect = item.effects[i];
            cout << " - ";
            switch (effect.type) {
                case HP_BONUS: cout << "最大HP: "; break;
                case MP_BONUS: cout << "最大MP: "; break;
                case HP_REGEN_BONUS: cout << "HP回復: "; break;
                case MP_REGEN_BONUS: cout << "MP回復: "; break;
                case EXP_BONUS: cout << "經驗值獲得: "; break;
                case DEFENSE_BONUS: cout << "物理防禦: "; break;
                case M_DEFENSE_BONUS: cout << "魔法防禦: "; break;
                case POISON_DAMAGE_BONUS: cout << "中毒傷害: "; break;
                case BURN_DAMAGE_BONUS: cout << "灼傷傷害: "; break;
                case M_ATTACK_BONUS: cout << "魔法攻擊: "; break;
                case DAMAGE_REFLECT: cout << "傷害反彈: "; break;
                case STATUS_DURATION_BONUS: cout << "狀態持續時間: "; break;
                case CLEANSE_PER_TURN: cout << "每回合淨化自身"; break;
                case M_ATTACK_WITH_STATUS: cout << "自身有狀態時魔法攻擊增加: "; break;
            }
            if (effect.type != CLEANSE_PER_TURN) {
                 cout << (effect.value > 0 ? "+" : "") << effect.value;
                 if (effect.type == M_ATTACK_BONUS || effect.type == DEFENSE_BONUS || effect.type == M_DEFENSE_BONUS || effect.type == HP_BONUS || effect.type == MP_BONUS) {
                     // 顯示基礎值和額外加成
                     stringstream ss;
                     ss << (item.rarityValue + item.bonus);
                     cout << " (基礎值:" << ss.str() << ")";
                 }
                 cout << endl;
            } else {
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        
        // 比較功能：如果舊裝備不為空
        if (oldItem.type != NONE_EQUIP) {
            cout << "  **與目前裝備「" << oldItem.name << "」比較**\n";
            cout << "----------------------------------------\n";
            // 找出所有新舊裝備含有的效果類型
            map<EquipmentEffectType, string> effectNames;
            effectNames[HP_BONUS] = "最大HP";
            effectNames[MP_BONUS] = "最大MP";
            effectNames[HP_REGEN_BONUS] = "HP回復";
            effectNames[MP_REGEN_BONUS] = "MP回復";
            effectNames[EXP_BONUS] = "經驗值獲得";
            effectNames[DEFENSE_BONUS] = "物理防禦";
            effectNames[M_DEFENSE_BONUS] = "魔法防禦";
            effectNames[POISON_DAMAGE_BONUS] = "中毒傷害";
            effectNames[BURN_DAMAGE_BONUS] = "灼傷傷害";
            effectNames[M_ATTACK_BONUS] = "魔法攻擊";
            effectNames[DAMAGE_REFLECT] = "傷害反彈";
            effectNames[STATUS_DURATION_BONUS] = "狀態持續時間";
            effectNames[CLEANSE_PER_TURN] = "每回合淨化自身";
            effectNames[M_ATTACK_WITH_STATUS] = "自身有狀態時魔法攻擊增加";

            vector<EquipmentEffectType> allEffectTypes;
            for(map<EquipmentEffectType, string>::iterator it = effectNames.begin(); it != effectNames.end(); ++it) {
                allEffectTypes.push_back(it->first);
            }
            
            bool hasComparison = false;
            for (size_t i = 0; i < allEffectTypes.size(); ++i) {
                int newTotal = getEffectValue(item, allEffectTypes[i]);
                int oldTotal = getEffectValue(oldItem, allEffectTypes[i]);

                if (newTotal != oldTotal) {
                    hasComparison = true;
                    int diff = newTotal - oldTotal;
                    stringstream diffSs;
                    diffSs << diff;
                    string diffStr = diffSs.str();
                    
                    if (diff > 0) {
                        SetColor(10); // 綠色
                        diffStr = "(+" + diffStr + ")";
                    } else {
                        SetColor(12); // 紅色
                        diffStr = "(" + diffStr + ")";
                    }
                    cout << effectNames[allEffectTypes[i]] << ": " << (newTotal > 0 ? "+" : "") << newTotal << " " << diffStr << endl;
                }
            }

            if (!hasComparison) {
                SetColor(15);
                cout << "無屬性差異。\n";
            }
            SetColor(11);
            cout << "----------------------------------------\n";
        }

        cout << "是否要裝備它？ (y/n)\n";
        SetColor();

        char choice;
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            switch (item.type) {
                case HELMET:
                    if (helmet.type != NONE_EQUIP) {
                        cout << "你卸下了「" << helmet.name << "」。\n";
                    }
                    helmet = item;
                    cout << "「" << helmet.name << "」已裝備。\n";
                    break;
                case ARMOR:
                    if (armor.type != NONE_EQUIP) {
                        cout << "你卸下了「" << armor.name << "」。\n";
                    }
                    armor = item;
                    cout << "「" << armor.name << "」已裝備。\n";
                    break;
                case STAFF:
                    if (staff.type != NONE_EQUIP) {
                        cout << "你卸下了「" << staff.name << "」。\n";
                    }
                    staff = item;
                    cout << "「" << staff.name << "」已裝備。\n";
                    break;
            }
            calculateTotalStats(); // 重新計算屬性
        } else {
            cout << "你選擇不裝備它，裝備被丟棄了。\n";
        }
        system("pause");
    }

    // MODIFICATION: 重構升級邏輯
	void levelUp() {
 	    SetColor(2);
 	    level++;
 	    exp -= expToNextLevel;
   	 	expToNextLevel = 30 + 20 * level; 

   	 	cout << "\n****************************************\n";
   		cout << "* LEVEL UP! 你升到了等級 " << level << "! *\n";
  	 	cout << "****************************************\n";
  	 	SetColor(7);

  	  	int choice = 0;
    	do {
        	cout << "請選擇要提升的屬性：\n";
        	cout << "0. 查看目前能力值\n";
        	cout << "1. 提升 Max HP (+10)\n";
        	cout << "2. 提升 Max MP (+5)\n";
        	cout << "3. 提升 魔法攻擊 (+2)\n";
        	cout << "4. 提升 物理防禦 (+3)\n";
        	cout << "5. 提升 魔法防禦 (+3)\n";
        	cout << "請輸入你的選擇 (0-5): ";
        	cin >> choice;

            // 允許在不失去選擇提示的情況下查看狀態
            if (choice == 0) {
                showStatus();
                choice = -1; // 重新迴圈
                continue;
            }

            // 將選擇的加成應用於額外屬性變數
        	switch (choice) {
            	case 1: 
                	bonusMaxHp += 10; 
                	cout << "最大HP的升級點數增加了！\n"; 
               		break;
            	case 2: 
                	bonusMaxMp += 5; 
                	cout << "最大MP的升級點數增加了！\n"; 
                	break;
            	case 3: 
                    bonusMagicAttack += 2; 
                    cout << "魔法攻擊的升級點數增加了！\n"; 
                    break;
            	case 4: 
                    bonusDefense += 3; 
                    cout << "物理防禦的升級點數增加了！\n"; 
                    break;
            	case 5: 
                    bonusMagicDefense += 3; 
                    cout << "魔法防禦的升級點數增加了！\n"; 
                    break;
            	default: 
                    cout << "無效的選擇，請重新輸入。\n"; 
                    choice = -1; // 重新迴圈
                    break;
    	    }
   		} while (choice == -1);

        // 現在，重新計算所有屬性以包含新的加成
        calculateTotalStats(); 
    
        // 升級時完全治癒玩家
  	    hp = maxHp;
  	    mp = maxMp;
    
    	SetColor();
	}

    void gainExp(int amount) {
        int totalExpBonus = 0;
        vector<Equipment*> equippedItems;
        equippedItems.push_back(&helmet);
        equippedItems.push_back(&armor);
        equippedItems.push_back(&staff);
        for (size_t i = 0; i < equippedItems.size(); ++i) {
            Equipment* item = equippedItems[i];
            for (size_t j = 0; j < item->effects.size(); ++j) {
                Effect effect = item->effects[j];
                if (effect.type == EXP_BONUS) {
                    totalExpBonus += effect.value;
                }
            }
        }
        exp += amount + totalExpBonus;
        cout << "你獲得了 " << amount << " 點經驗值";
        if (totalExpBonus > 0) {
            cout << " (額外獲得 " << totalExpBonus << " 點)";
        }
        cout << "。\n";
        while (exp >= expToNextLevel) {
            levelUp();
        }
    }

    void regenerate(bool canRegenHp, bool canRegenMp) {
        if (hp > 0) {
            if(canRegenHp) {
                hp += hpRegen;
                if (hp > maxHp) hp = maxHp;
            }
            if(canRegenMp) {
                mp += mpRegen;
                if (mp > maxMp) mp = maxMp;
            }
        }
    }

    void takeDamage(int amount, DamageType type, Enemy& enemy) {
        int damageTaken = 0;
        float finalDamage = (float)amount;

        bool hasEvaded = false;
        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == EVADE_CHANCE && getRandomNumber(1, 100) <= activeStatuses[i].value) {
                hasEvaded = true;
                break;
            }
        }
        if (hasEvaded) {
            SetColor(15);
            cout << "你成功閃避了攻擊！\n";
            SetColor();
            return;
        }

        for (size_t i = 0; i < armor.effects.size(); ++i) {
            if (armor.effects[i].type == DAMAGE_REFLECT) {
                int reflectDamage = armor.effects[i].value;
                enemy.hp -= reflectDamage;
                cout << "你的盔甲反彈了 " << reflectDamage << " 點傷害給敵人「" << enemy.name << "」！\n";
            }
        }
        
        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == WEAK) {
                finalDamage *= (1.0f + (float)activeStatuses[i].value / 100.0f);
            } else if (activeStatuses[i].type == GUARD) {
                finalDamage *= (1.0f - (float)activeStatuses[i].value / 100.0f);
            }
        }

        string damageTypeStr = "";
        if (type == PHYSICAL) {
            damageTaken = (int)finalDamage - defense;
            damageTypeStr = "物理";
        } else if (type == MAGICAL) {
            damageTaken = (int)finalDamage - magicDefense;
            damageTypeStr = "魔法";
        }
        
        if (damageTaken < 1) damageTaken = 1;
        hp -= damageTaken;
        if (hp < 0) hp = 0;
        SetColor(4);
        cout << "你受到了 " << damageTaken << " 點 " << damageTypeStr << " 傷害！\n";
        SetColor();

        // 將傷害記錄到 Boss 的計數器中
        enemy.damageTakenThisTurn += damageTaken;

        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == BLEED) {
                int bleedDamage = activeStatuses[i].value;
                hp -= bleedDamage;
                if (hp < 0) hp = 0;
                SetColor(4);
                cout << "你因出血額外受到了 " << bleedDamage << " 點傷害！\n";
                SetColor();
            }
        }
    }

    void printStatus() {
        SetColor(14);
        cout << "[玩家 Lv: " << level << "] HP: " << hp << "/" << maxHp
             << ", MP: " << mp << "/" << maxMp
             << ", EXP: " << exp << "/" << expToNextLevel;
        if (!activeStatuses.empty()) {
            cout << " (狀態:";
            for (size_t i = 0; i < activeStatuses.size(); ++i) {
                 Status status = activeStatuses[i];
                 string statusName;
                switch (status.type) {
                    case BURN: statusName = "灼傷"; break;
                    case POISON: statusName = "中毒"; break;
                    case STUN: statusName = "暈眩"; break;
                    case BLEED: statusName = "出血"; break;
                    case WEAK: statusName = "虛弱"; break;
                    case FROST: statusName = "寒冷"; break;
                    case GUARD: statusName = "守護"; break;
                    case RAGE: statusName = "憤怒"; break;
                    case CALM: statusName = "冷靜"; break;
                    case HUNGER: statusName = "飢餓"; break;
                    case MEDITATION: statusName = "冥想"; break;
                    case EVADE_CHANCE: statusName = "迴避"; break;
                    default: statusName = "未知狀態"; break;
                }
                cout << " " << statusName << "[" << status.duration << "]";
            }
            cout << ")";
        }
        cout << endl;
        SetColor();
    }

    void learnSkill(const Skill& newSkill) {
        SetColor(6);
        cout << "----------------------------------------\n";
        cout << "恭喜！你學會了新技能：「" << newSkill.name << "」！\n";
        cout << "----------------------------------------\n";
        
        if (skills.size() < MAX_SKILLS) {
            skills.push_back(newSkill);
            cout << "技能「" << newSkill.name << "」已加入你的技能清單。\n";
        } else {
            cout << "你的技能清單已滿。請選擇一個技能替換 (輸入 0 放棄): \n";
            for (size_t i = 0; i < skills.size(); ++i) {
                cout << i + 1 << ". 替換「" << skills[i].name << "」\n";
            }
            int choice;
            cout << "請輸入你的選擇: ";
            cin >> choice;

            if (choice > 0 && choice <= MAX_SKILLS) {
                cout << "你放棄了「" << skills[choice - 1].name << "」\n";
                skills[choice - 1] = newSkill;
                cout << "技能「" << newSkill.name << "」成功替換了它。\n";
            } else {
                cout << "你放棄了學習新技能。\n";
            }
        }
        SetColor();
        system("pause");
    }
};

// 玩家和敵人的獨立技能清單
vector<Skill> PLAYER_SKILLS;
vector<Skill> ENEMY_SKILLS;

// 新增裝備模板列表
vector<EquipmentTemplate> ALL_EQUIPMENT_TEMPLATES;
vector<EquipmentEffectType> ALL_EFFECT_TYPES;

// MODIFICATION: 新增一個全域變數來代表特殊的「綠帽」
Equipment greenHat("綠帽", HELMET, COMMON, 0, vector<Effect>());

// 根據稀有度獲得數值
int getRarityValue(Rarity rarity) {
    switch (rarity) {
        case COMMON: return 0;
        case UNCOMMON: return 1;
        case RARE: return 3;
        case EPIC: return 5;
        case LEGENDARY: return 9;
        default: return 0;
    }
}

// 根據機率隨機獲得稀有度
Rarity getRandomRarity() {
    int chance = getRandomNumber(1, 100);
    if (chance <= 1) return LEGENDARY;      // 1%
    if (chance <= 4) return EPIC;           // 3%
    if (chance <= 18) return RARE;          // 14%
    if (chance <= 50) return UNCOMMON;      // 32%
    return COMMON;                          // 50%
}

// 根據類型和稀有度隨機生成裝備
Equipment generateRandomEquipment(EquipmentType type) {
    Rarity rarity = getRandomRarity();
    int rarityValue = getRarityValue(rarity);

    // 根據裝備類型篩選模板
    vector<EquipmentTemplate> filteredTemplates;
    for (size_t i = 0; i < ALL_EQUIPMENT_TEMPLATES.size(); ++i) {
        if (ALL_EQUIPMENT_TEMPLATES[i].type == type) {
            filteredTemplates.push_back(ALL_EQUIPMENT_TEMPLATES[i]);
        }
    }

    if (filteredTemplates.empty()) {
        return Equipment(); // 返回空裝備
    }

    // 隨機選擇一個模板
    EquipmentTemplate chosenTemplate = filteredTemplates[getRandomNumber(0, filteredTemplates.size() - 1)];

    // 產生額外的隨機加成值
    int bonus = getRandomNumber(chosenTemplate.bonusMin, chosenTemplate.bonusMax);
    int totalValue = chosenTemplate.baseStat + rarityValue + bonus;

    vector<Effect> effects;
    // 主要屬性
    effects.push_back(Effect(chosenTemplate.effectType, totalValue));
    
    // 傳說裝備有特殊屬性
    if (rarity == LEGENDARY) {
        int choice = getRandomNumber(1, 3);
        if (type == HELMET && choice == 1) effects.push_back(Effect(CLEANSE_PER_TURN, 1));
        else if (type == ARMOR && choice == 1) effects.push_back(Effect(DAMAGE_REFLECT, rarityValue + bonus));
        else if (type == STAFF && choice == 1) effects.push_back(Effect(STATUS_DURATION_BONUS, 2));
    }
    
    // 如果稀有度為RARE以上，額外給予一個隨機效果
    if (rarity >= RARE) {
        vector<EquipmentEffectType> secondaryEffectPool;
        for (size_t i = 0; i < ALL_EFFECT_TYPES.size(); ++i) {
            if (ALL_EFFECT_TYPES[i] != chosenTemplate.effectType) {
                // 不讓第二個效果和第一個效果重複
                secondaryEffectPool.push_back(ALL_EFFECT_TYPES[i]);
            }
        }

        if (!secondaryEffectPool.empty()) {
            EquipmentEffectType secondaryEffectType = secondaryEffectPool[getRandomNumber(0, secondaryEffectPool.size() - 1)];
            int secondaryValue = getRandomNumber(1, rarityValue); // 第二個效果的值較小
            effects.push_back(Effect(secondaryEffectType, secondaryValue));
        }
    }

    // 命名規則
    map<Rarity, string> rarityPrefixMap;
    rarityPrefixMap[COMMON] = "普通的";
    rarityPrefixMap[UNCOMMON] = "罕見的";
    rarityPrefixMap[RARE] = "稀有的";
    rarityPrefixMap[EPIC] = "史詩的";
    rarityPrefixMap[LEGENDARY] = "傳說的";
    
    string finalName = rarityPrefixMap[rarity] + " " + chosenTemplate.name;
    
    return Equipment(finalName, type, rarity, rarityValue, effects, bonus);
}

// 建立隨機敵人
Enemy createEnemy(int level) {
    Enemy e;
    int type = getRandomNumber(0, 6); 
    e.attack = 5 + level;
    e.magicAttack = 5 + level;
    if (type == 0) {
        e.name = "史萊姆";
        e.hp = 30 + level * 4;
        e.maxHp = e.hp;
        e.expValue = 10 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[1]);
    } else if (type == 1) {
        e.name = "哥布林";
        e.hp = 40 + level * 5;
        e.maxHp = e.hp;
        e.expValue = 15 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        // MODIFICATION: 讓哥布林學會新的「聲援咆嘯」技能
        e.skills.push_back(ENEMY_SKILLS[9]); 
    } else if (type == 2) {
        e.name = "骷髏戰士";
        e.hp = 30 + level * 2 + getRandomNumber(5, 15);
        e.maxHp = e.hp;
        e.expValue = 13 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[2]);
    } else if (type == 3) {
        e.name = "毒蜘蛛";
        e.hp = 25 + level * 2 + getRandomNumber(3, 10);
        e.maxHp = e.hp;
        e.expValue = 25 + level;
        e.skills.push_back(ENEMY_SKILLS[3]);
        e.skills.push_back(ENEMY_SKILLS[4]);
    } else if (type == 4) {
        e.name = "殭屍";
        e.hp = 50 + level * 4 + getRandomNumber(5, 30);
        e.maxHp = e.hp;
        e.expValue = 20 + level + getRandomNumber(5, 10);
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[5]);
    } else if (type == 5) {
        e.name = "巫師";
        e.hp = 35 + level * 3;
        e.maxHp = e.hp;
        e.expValue = 22 + level;
        e.skills.push_back(ENEMY_SKILLS[1]);
        e.skills.push_back(ENEMY_SKILLS[6]);
    } else { // type == 6
        e.name = "牛頭人";
        e.hp = 80 + level * 10;
        e.maxHp = e.hp;
        e.expValue = 40 + level * 2;
        e.skills.push_back(ENEMY_SKILLS[7]);
        // MODIFICATION: 讓牛頭人學會「丟綠色帽子」
        e.skills.push_back(ENEMY_SKILLS[8]);
    }
    return e;
}

// 戰鬥流程
void startBattle(Player& player, Enemy& enemy) {
    SetColor(13);
    cout << "野生的 " << enemy.name << " 出現了!" << endl;
    SetColor();
    
    if (enemy.name == "牛頭人") {
        SetColor(12);
        cout << enemy.name << " 發出了憤怒的咆哮！它進入了憤怒狀態！\n";
        enemy.addStatus(RAGE, 10, 20); // 憤怒10回合，攻擊力+20%
        SetColor();
    }

    player.calculateTotalStats();

    while (player.hp > 0 && enemy.hp > 0) {
        cout << "\n-- 回合開始 --\n";
        enemy.damageTakenThisTurn = 0; // 每回合重置傷害計數器
        
        // Boss 被動技能階段
        if (enemy.name == "Exile") {
            SetColor(13);
            cout << "Exile 的存在正在侵蝕你的潛能！\n";
            player.bonusMaxHp -= 5;
            player.bonusMaxMp -= 5;
            enemy.exileGauge += 10;
            player.calculateTotalStats();
            cout << "你的最大HP和最大MP永久減少了 5 點！ (放逐計量條: " << enemy.exileGauge << ")\n";
            SetColor();
        }
        if (enemy.name == "斷律") {
            enemy.lawbreakerAbility = getRandomNumber(1, 4);
            SetColor(13);
            cout << "斷律重編了本回合的規則！\n";
            switch(enemy.lawbreakerAbility) {
                case 1: cout << "規則：你無法獲得任何狀態！\n"; break;
                case 2: cout << "規則：你無法恢復HP！\n"; break;
                case 3: cout << "規則：你無法恢復MP！\n"; break;
                case 4: cout << "規則：你無法使用攻擊技能！\n"; break;
            }
            SetColor();
        }
        if (enemy.name == "恆溫者") {
            SetColor(13);
            if (enemy.temperatureValue >= 80) {
                cout << "高溫觸發了恆溫者的爐心！它進入了憤怒狀態！你無法使用攻擊技能！\n";
                enemy.addStatus(RAGE, 2, 20);
            } else if (enemy.temperatureValue <= 20) {
                cout << "低溫凍結了你的意志！你進入了寒冷狀態！你無法使用治療或強化技能！\n";
                player.addStatus(FROST, 2, 15);
            }
            cout << "目前溫度值: " << enemy.temperatureValue << endl;
            SetColor();
        }


        if (player.hp > player.maxHp) player.hp = player.maxHp;
		if (player.mp > player.maxMp) player.mp = player.maxMp;
        player.printStatus();
        cout << enemy.name << " HP: " << enemy.hp;

        if (!enemy.activeStatuses.empty()) {
            cout << " (狀態:";
            for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                Status status = enemy.activeStatuses[i];
                string statusName;
                switch (status.type) {
                    case BURN: statusName = "灼傷"; break;
                    case POISON: statusName = "中毒"; break;
                    case STUN: statusName = "暈眩"; break;
                    case BLEED: statusName = "出血"; break;
                    case WEAK: statusName = "虛弱"; break;
                    case FROST: statusName = "寒冷"; break;
                    case RAGE: statusName = "憤怒"; break;
                    default: statusName = "未知狀態"; break;
                }
                cout << " " << statusName << "[" << status.duration << "]";
            }
            cout << ")";
        }
        cout << endl;

        // NEW: 魔力控制系統 (Mana Control System)
        float manaPercentage = (player.maxMp > 0) ? ((float)player.mp / player.maxMp * 100.0f) : 0;
        float manaDamageModifier = 1.0f;
        int manaHitRateModifier = 0;

        if (manaPercentage >= 85.0f) {
            manaDamageModifier = 1.2f;
            SetColor(10);
            cout << "你的魔力滿溢，攻擊力提升了！ (傷害 +20%)\n";
            SetColor();
        } else if (manaPercentage < 50.0f && manaPercentage >= 30.0f) {
            manaDamageModifier = 0.9f;
            SetColor(12);
            cout << "你的魔力有些不穩，攻擊變得虛弱。 (傷害 -10%)\n";
            SetColor();
        } else if (manaPercentage < 30.0f && manaPercentage >= 10.0f) {
            manaDamageModifier = 0.7f;
            manaHitRateModifier = -10;
            SetColor(12);
            cout << "魔力嚴重不足，你的攻擊變得軟弱無力。 (傷害 -30%, 命中率 -10%)\n";
            SetColor();
        } else if (manaPercentage < 10.0f && manaPercentage >= 5.0f) {
            manaDamageModifier = 0.5f;
            manaHitRateModifier = -30;
            SetColor(12);
            cout << "魔力瀕臨枯竭，你幾乎無法施法！ (傷害 -50%, 命中率 -30%)\n";
            SetColor();
        } else if (manaPercentage < 5.0f) {
            SetColor(4);
            cout << "魔力完全枯竭，你受到了強烈的反噬，暈眩了一回合！\n";
            SetColor();
            player.addStatus(STUN, 2, 0); // 暈眩效果會在本回合結束時-1，所以設為2才能暈眩下一整個回合
        }
        
        vector<int> playerExpiredStatuses;
        for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
            if (enemy.name == "克羅諾斯") {
                 player.activeStatuses[i].duration -= 2; // 時間加速
            } else {
                 player.activeStatuses[i].duration--;
            }
            player.activeStatuses[i].turnCount++;

            if (player.activeStatuses[i].type == BURN) {
                int burnDamageBonus = 0;
                for (size_t k = 0; k < player.staff.effects.size(); ++k) {
                    if (player.staff.effects[k].type == BURN_DAMAGE_BONUS) {
                        burnDamageBonus += player.staff.effects[k].value;
                    }
                }
                int burnDamage = player.activeStatuses[i].value + burnDamageBonus;
                player.hp -= burnDamage;
                cout << "你因灼傷受到了 " << burnDamage << " 點傷害！\n";
            } else if (player.activeStatuses[i].type == POISON) {
                int poisonDamageBonus = 0;
                for (size_t k = 0; k < player.staff.effects.size(); ++k) {
                    if (player.staff.effects[k].type == POISON_DAMAGE_BONUS) {
                        poisonDamageBonus += player.staff.effects[k].value;
                    }
                }
                int poisonDamage = player.activeStatuses[i].value + player.activeStatuses[i].turnCount -1 + poisonDamageBonus;
                player.hp -= poisonDamage;
                cout << "你因中毒受到了 " << poisonDamage << " 點傷害！\n";
            } else if (player.activeStatuses[i].type == HUNGER) {
                player.mp -= player.activeStatuses[i].value;
                cout << "你因飢餓消耗了 " << player.activeStatuses[i].value << " 點魔力！\n";
            } else if (player.activeStatuses[i].type == MEDITATION) {
                player.hp += 5;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                player.mp += 5;
                if (player.mp > player.maxMp) player.mp = player.maxMp;
                SetColor(11);
                cout << "你因冥想回復了 5 點 HP 和 MP。\n";
                SetColor();
            }
            if (player.activeStatuses[i].duration <= 0) {
                // 狀態即將過期，顯示提示
                SetColor(8);
                cout << "你的狀態「" << player.activeStatuses[i].type << "」已結束。\n";
                SetColor();
                playerExpiredStatuses.push_back(i);
            }
        }
        for (int i = playerExpiredStatuses.size() - 1; i >= 0; --i) {
            player.activeStatuses.erase(player.activeStatuses.begin() + playerExpiredStatuses[i]);
        }

        for (size_t i = 0; i < player.helmet.effects.size(); ++i) {
            if (player.helmet.effects[i].type == CLEANSE_PER_TURN) {
                SetColor(10);
                cout << "你的頭盔淨化了你身上的所有負面狀態！\n";
                SetColor();
                player.activeStatuses.clear();
                break;
            }
        }

        vector<int> enemyExpiredStatuses;
        for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
            enemy.activeStatuses[i].duration--;
            enemy.activeStatuses[i].turnCount++;
            
            if (enemy.activeStatuses[i].type == BURN) {
                enemy.hp -= enemy.activeStatuses[i].value;
                cout << enemy.name << " 因灼傷受到了 " << enemy.activeStatuses[i].value << " 點傷害！\n";
            } else if (enemy.activeStatuses[i].type == POISON) {
                int poisonDamage = enemy.activeStatuses[i].value + enemy.activeStatuses[i].turnCount -1;
                enemy.hp -= poisonDamage;
                cout << enemy.name << " 因中毒受到了 " << poisonDamage << " 點傷害！\n";
            }
            if (enemy.activeStatuses[i].duration <= 0) {
                // 狀態即將過期，顯示提示
                SetColor(8);
                cout << "敵人「" << enemy.name << "」的狀態「" << enemy.activeStatuses[i].type << "」已結束。\n";
                SetColor();
                enemyExpiredStatuses.push_back(i);
            }
        }
        for (int i = enemyExpiredStatuses.size() - 1; i >= 0; --i) {
            enemy.activeStatuses.erase(enemy.activeStatuses.begin() + enemyExpiredStatuses[i]);
        }
        
        if (player.hp <= 0) {
            cout << "你因狀態效果被擊敗了！\n";
            return;
        }
        if (enemy.hp <= 0) {
            cout << enemy.name << " 因狀態效果被擊敗了！\n";
            player.gainExp(enemy.expValue);
            return;
        }

        bool isStunned = false;
        for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
            if (player.activeStatuses[i].type == STUN) {
                isStunned = true;
                break;
            }
        }

        if (!isStunned) {
            cout << "\n選擇你的行動:\n";
            for (size_t i = 0; i < player.skills.size(); ++i) {
                Skill currentSkill = player.skills[i];
                cout << i + 1 << ". 「" << currentSkill.name << "」 (MP消耗: " << currentSkill.mpCost << ")";
                if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
                    cout << ", 傷害: " << currentSkill.baseDamage << ", 命中: " << currentSkill.hitRate << "%";
                } else if (currentSkill.type == HEAL) {
                    cout << ", 治癒量: " << currentSkill.value;
                }
                if (currentSkill.statusToInflict != NONE) {
                    string statusName;
                    switch (currentSkill.statusToInflict) {
                        case BURN: statusName = "灼傷"; break;
                        case POISON: statusName = "中毒"; break;
                        case STUN: statusName = "暈眩"; break;
                        case BLEED: statusName = "出血"; break;
                        default: statusName = "未知狀態"; break;
                    }
                    cout << " (施加: " << statusName << ")";
                }
                cout << endl;
            }

            cout << player.skills.size() + 1 << ". 休息 (恢復MP)\n";
            cout << player.skills.size() + 2 << ". 查看能力值\n";

            int choice = 0;
            bool validChoice = false;

            while (!validChoice) {
                cout << "請輸入你的選擇 (1-" << player.skills.size() + 2 << "): ";
                cin >> choice;

                if (choice > 0 && choice <= player.skills.size()) {
                    const Skill& chosenSkill = player.skills[choice - 1];
                    
                    // Boss 規則檢查
                    if (enemy.name == "斷律" && enemy.lawbreakerAbility == 4 && (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == BUFF_ATTACK)) {
                        cout << "規則不允許你使用攻擊技能！\n"; continue;
                    }
                    if (enemy.name == "恆溫者" && enemy.temperatureValue >= 80 && (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == BUFF_ATTACK)) {
                        cout << "高溫讓你無法施展攻擊技能！\n"; continue;
                    }
                    if (enemy.name == "恆溫者" && enemy.temperatureValue <= 20 && (chosenSkill.type == HEAL || chosenSkill.type == BUFF)) {
                        cout << "低溫讓你無法施展治療或強化技能！\n"; continue;
                    }

                    if (player.mp >= chosenSkill.mpCost) {
                        player.mp -= chosenSkill.mpCost;
                        validChoice = true;
                        
                        // 恆溫者溫度計算
                        if (enemy.name == "恆溫者") {
                            if (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == BUFF_ATTACK) {
                                enemy.temperatureValue += 10;
                            } else {
                                enemy.temperatureValue -= 10;
                            }
                        }

                        switch (chosenSkill.type) {
                            case ATTACK:
                            case DEBUFF_ATTACK:
                            case BUFF_ATTACK: {
                                int finalHitRate = chosenSkill.hitRate + manaHitRateModifier; // 套用魔力控制的命中修正
                                for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
                                    if (player.activeStatuses[i].type == CALM) {
                                        finalHitRate += 10;
                                    }
                                }
                                
                                if (getRandomNumber(1, 100) <= finalHitRate) {
                                    int damage = player.magicAttack + chosenSkill.baseDamage;
                                    damage = (int)((float)damage * manaDamageModifier); // 套用魔力控制的傷害修正

                                    bool hasStatus = false;
                                    for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
                                        if (player.activeStatuses[i].type != NONE) {
                                            hasStatus = true;
                                            break;
                                        }
                                    }
                                    if (hasStatus) {
                                        for (size_t i = 0; i < player.staff.effects.size(); ++i) {
                                            if (player.staff.effects[i].type == M_ATTACK_WITH_STATUS) {
                                                damage = (int)((float)damage * (1.0f + (float)player.staff.effects[i].value / 100.0f));
                                                cout << "你的法杖因你身上的狀態而發出了光芒，攻擊力增加了！\n";
                                                break;
                                            }
                                        }
                                    }

                                    for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
                                        if (player.activeStatuses[i].type == RAGE) {
                                            damage = (int)((float)damage * (1.0f + (float)player.activeStatuses[i].value / 100.0f));
                                        }
                                    }
                                    for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
                                        if (player.activeStatuses[i].type == FROST) {
                                            damage = (int)((float)damage * (1.0f - (float)player.activeStatuses[i].value / 100.0f));
                                        }
                                    }

                                    if (chosenSkill.name == "復仇打擊") {
                                        damage += (player.maxHp - player.hp);
                                    }
                                    
                                    enemy.hp -= damage;
                                    enemy.damageTakenThisTurn += damage;
                                    SetColor(4);
                                    cout << "你用「" << chosenSkill.name << "」造成了 " << damage << " 點傷害!\n";
                                    SetColor();

                                    for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                                        if (enemy.activeStatuses[i].type == BLEED) {
                                            int bleedDamage = enemy.activeStatuses[i].value;
                                            enemy.hp -= bleedDamage;
                                            if (enemy.hp < 0) enemy.hp = 0;
                                            SetColor(4);
                                            cout << "敵人「" << enemy.name << "」因出血額外受到了 " << bleedDamage << " 點傷害！\n";
                                            SetColor();
                                        }
                                    }

                                    if (chosenSkill.statusToInflict != NONE) {
                                        bool canApplyStatus = true;
                                        if (enemy.name == "斷律" && enemy.lawbreakerAbility == 1) {
                                            canApplyStatus = false;
                                            cout << "規則不允許你施加狀態！\n";
                                        }
                                        if (enemy.name == "斷律" && chosenSkill.name == "裂界網") {
                                            canApplyStatus = false;
                                            cout << "裂界網無效化了你的狀態效果！\n";
                                        }
                                        
                                        if (canApplyStatus) {
                                            int finalDuration = chosenSkill.statusDuration;
                                            for (size_t i = 0; i < player.staff.effects.size(); ++i) {
                                                if (player.staff.effects[i].type == STATUS_DURATION_BONUS) {
                                                    finalDuration += player.staff.effects[i].value;
                                                    cout << "(法杖提供了額外 +" << player.staff.effects[i].value << " 回合持續時間)\n";
                                                    break;
                                                }
                                            }
                                            if (chosenSkill.type == DEBUFF_ATTACK) {
                                                enemy.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                                cout << "敵人「" << enemy.name << "」獲得了狀態：「" << chosenSkill.statusToInflict << "」\n";
                                            } else if (chosenSkill.type == BUFF_ATTACK) {
                                                player.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                                cout << "你獲得了狀態：「" << chosenSkill.statusToInflict << "」\n";
                                            }
                                        }
                                    }
                                } else {
                                    SetColor(8);
                                    cout << "你用「" << chosenSkill.name << "」但是失敗了!\n";
                                    SetColor();
                                }
                                break;
                            }
                            case HEAL: {
                                int healAmount = chosenSkill.value;
                                if (enemy.name == "斷律" && enemy.lawbreakerAbility == 2) {
                                    cout << "規則不允許你恢復HP！\n";
                                } else {
                                    player.hp += healAmount;
                                    if (player.hp > player.maxHp) player.hp = player.maxHp;
                                    SetColor(2);
                                    cout << "你使用「" << chosenSkill.name << "」，恢復了 " << healAmount << " 點 HP。\n";
                                    SetColor();
                                }
                                break;
                            }
                            case BUFF: {
                                if (chosenSkill.statusToInflict != NONE) {
                                    int finalDuration = chosenSkill.statusDuration;
                                     for (size_t i = 0; i < player.staff.effects.size(); ++i) {
                                        if (player.staff.effects[i].type == STATUS_DURATION_BONUS) {
                                            finalDuration += player.staff.effects[i].value;
                                            SetColor(11);
                                            cout << "你的法杖提供了額外 +" << player.staff.effects[i].value << " 回合持續時間！\n";
                                            SetColor();
                                            break;
                                        }
                                    }
                                    player.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                    cout << "你獲得了狀態：「" << chosenSkill.name << "」\n";
                                }
                                break;
                            }
                            case DEBUFF: {
                                if (chosenSkill.statusToInflict != NONE) {
                                    int finalDuration = chosenSkill.statusDuration;
                                     for (size_t i = 0; i < player.staff.effects.size(); ++i) {
                                        if (player.staff.effects[i].type == STATUS_DURATION_BONUS) {
                                            finalDuration += player.staff.effects[i].value;
                                            SetColor(11);
                                            cout << "你的法杖提供了額外 +" << player.staff.effects[i].value << " 回合持續時間！\n";
                                            SetColor();
                                            break;
                                        }
                                    }
                                    enemy.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                    cout << "敵人「" << enemy.name << "」獲得了狀態：「" << chosenSkill.name << "」\n";
                                }
                                break;
                            }
                            case UTILITY: {
                                if (chosenSkill.name == "淨化") {
                                    player.activeStatuses.clear();
                                    SetColor(10);
                                    cout << "你使用「" << chosenSkill.name << "」，清除了自身所有狀態！\n";
                                    SetColor();
                                }
                                break;
                            }
                        }
                    } else {
                        cout << "魔力不足。請重新選擇。\n";
                    }
                } else if (choice == player.skills.size() + 1) {
                    validChoice = true;
                    if (enemy.name == "恆溫者") enemy.temperatureValue -= 10;
                    
                    bool canRegenMp = true;
                    if (enemy.name == "斷律" && enemy.lawbreakerAbility == 3) {
                         cout << "規則不允許你恢復MP！\n";
                         canRegenMp = false;
                    }
                    if(canRegenMp){
                        player.mp += player.mpRegen + 5;
                        if (player.mp > player.maxMp) player.mp = player.maxMp;
                        SetColor(10);
                        cout << "你選擇休息一回合，感覺魔力恢復了一些。\n";
                        SetColor();
                    }
                } else if (choice == player.skills.size() + 2) {
                    player.showStatus();
                } else {
                    cout << "無效的選擇，請重新輸入。\n";
                }
            }
        } else {
            cout << "你因暈眩無法行動。\n";
        }

        if (enemy.hp <= 0) {
            SetColor(14);
            cout << enemy.name << " 被打敗了!\n";
            SetColor();
            player.gainExp(enemy.expValue);
            return;
        }

        bool enemyIsStunned = false;
        for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
            if (enemy.activeStatuses[i].type == STUN) {
                enemyIsStunned = true;
                break;
            }
        }

        if (!enemyIsStunned) {
            int skillIndex = getRandomNumber(0, enemy.skills.size() - 1);
            const Skill& chosenSkill = enemy.skills[skillIndex];

            cout << enemy.name << " 準備使用「" << chosenSkill.name << "」!\n";

            // Boss 特殊技能處理
            if (enemy.name == "克羅諾斯") {
                if (chosenSkill.name == "衰退") {
                    player.hp -= chosenSkill.baseDamage;
                    cout << "衰退之力無視了你的防禦，造成了 " << chosenSkill.baseDamage << " 點傷害！\n";
                } else if (chosenSkill.name == "倒轉之輪") {
                    if (enemy.hp < player.hp) {
                        cout << "克羅諾斯倒轉了時間之輪，交換了雙方的生命力！\n";
                        int tempHp = player.hp;
                        player.hp = enemy.hp;
                        enemy.hp = tempHp;
                        if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                    } else {
                         cout << "克羅諾斯試圖倒轉時間，但失敗了。\n";
                    }
                } else if (chosenSkill.name == "第十三時") {
                     enemy.hp += 50;
                     if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                     cout << "克羅諾斯進入時間的間隙，回復了 50 點生命！\n";
                } else { // 秒裂, 時間齒輪
                    player.takeDamage(chosenSkill.baseDamage + enemy.magicAttack, chosenSkill.damageType, enemy);
                    player.addStatus(chosenSkill.statusToInflict, chosenSkill.statusDuration, chosenSkill.statusValue);
                     if(chosenSkill.name == "時間齒輪") enemy.addStatus(RAGE, 10, 20);
                }
            } else if (enemy.name == "恆溫者") {
                player.takeDamage(chosenSkill.baseDamage + enemy.magicAttack, chosenSkill.damageType, enemy);
                if (chosenSkill.name == "爐心爆破") {
                    enemy.temperatureValue += 20;
                    player.addStatus(BURN, 5, 10);
                } else if (chosenSkill.name == "冰結迴旋") {
                    enemy.temperatureValue -= 20;
                    player.addStatus(FROST, 3, 15);
                    player.addStatus(BLEED, 3, 5);
                } else if (chosenSkill.name == "極光世界") {
                    enemy.temperatureValue = 50;
                    player.activeStatuses.clear();
                    enemy.activeStatuses.clear();
                    cout << "極光洗淨了戰場上的一切狀態！\n";
                } else if (chosenSkill.name == "雙極崩界") {
                    player.mp -= 12;
                    if(player.mp < 0) player.mp = 0;
                    cout << "你的魔力被剝奪了！\n";
                }
            } else if (enemy.name == "斷律") {
                 if (chosenSkill.name == "動作鎖鏈") {
                    player.takeDamage(chosenSkill.baseDamage, PHYSICAL, enemy);
                    enemy.addStatus(GUARD, 2, 30);
                 } else if (chosenSkill.name == "絕路封鎖") {
                    player.addStatus(HUNGER, 5, 5);
                 } else if (chosenSkill.name == "裂界網") {
                    player.takeDamage(chosenSkill.baseDamage, PHYSICAL, enemy);
                    enemy.activeStatuses.clear();
                 } else if (chosenSkill.name == "規則重編") {
                    cout << "規則被重編，你受到了與你造成傷害等量的反噬！\n";
                    player.takeDamage(enemy.damageTakenThisTurn, PHYSICAL, enemy);
                 }
            } else if (enemy.name == "Exile") {
                if (chosenSkill.name == "虛無領域") {
                    enemy.addStatus(GUARD, 5, 30);
                    enemy.exileGauge += 30;
                } else if (chosenSkill.name == "放逐協議") {
                    cout << "Exile 的協議生效，它回復了本回合受到的所有傷害！\n";
                    enemy.hp += enemy.damageTakenThisTurn;
                    if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                    enemy.exileGauge += enemy.damageTakenThisTurn;
                } else if (chosenSkill.name == "現實扭曲") {
                    cout << "現實被扭曲，你的所有狀態都被轉移給了 Exile！\n";
                    enemy.activeStatuses.insert(enemy.activeStatuses.end(), player.activeStatuses.begin(), player.activeStatuses.end());
                    player.activeStatuses.clear();
                    enemy.exileGauge += 30;
                } else if (chosenSkill.name == "遺忘脈衝") {
                    player.takeDamage(chosenSkill.baseDamage, MAGICAL, enemy);
                    player.mp -= 20;
                    if (player.mp < 0) player.mp = 0;
                    enemy.exileGauge += chosenSkill.baseDamage;
                } else if (chosenSkill.name == "最終審判") {
                    cout << "最終審判降臨！你受到了 " << enemy.exileGauge << " 點無法防禦的傷害！\n";
                    player.hp -= enemy.exileGauge;
                    enemy.exileGauge = 0;
                }
                cout << "(放逐計量條: " << enemy.exileGauge << ")\n";
            }
            // 普通敵人攻擊邏輯
            else {
                if (getRandomNumber(1, 100) <= chosenSkill.hitRate) {
                    int damage = chosenSkill.baseDamage;
                    if (chosenSkill.damageType == PHYSICAL) {
                        damage += enemy.attack;
                    } else if (chosenSkill.damageType == MAGICAL) {
                        damage += enemy.magicAttack;
                    }
                    
                    float enemyDamageModifier = 1.0f;
                    for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                        if (enemy.activeStatuses[i].type == FROST) {
                            enemyDamageModifier -= (float)enemy.activeStatuses[i].value / 100.0f;
                        }
                    }
                    if (enemyDamageModifier < 0) enemyDamageModifier = 0;
                    damage = (int)((float)damage * enemyDamageModifier);

                    for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                        if (enemy.activeStatuses[i].type == RAGE) {
                            damage = (int)((float)damage * (1.0f + (float)enemy.activeStatuses[i].value / 100.0f));
                        }
                    }

                    if (chosenSkill.statusToInflict != NONE) {
                        if (chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == DEBUFF) {
                            player.addStatus(chosenSkill.statusToInflict, chosenSkill.statusDuration, chosenSkill.statusValue);
                            cout << "你獲得了狀態：「" << chosenSkill.statusToInflict << "」\n";
                        }
                        else if (chosenSkill.type == BUFF) {
                            enemy.addStatus(chosenSkill.statusToInflict, chosenSkill.statusDuration, chosenSkill.statusValue);
                            cout << "敵人「" << enemy.name << "」對自己施加了狀態：「" << chosenSkill.name << "」\n";
                        }
                    }
                    
                    if (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK) {
                        player.takeDamage(damage, chosenSkill.damageType, enemy);
                    } else if (chosenSkill.type == DEBUFF || chosenSkill.type == BUFF) {
                        cout << enemy.name << " 使用了「" << chosenSkill.name << "」，沒有直接造成傷害。\n";
                        
                    }
                } else {
                    SetColor(8);
                    cout << enemy.name << " 的「" << chosenSkill.name << "」失敗了!\n";
                    SetColor();
                }
            }
        } else {
            cout << "敵人「" << enemy.name << "」因暈眩無法行動。\n";
        }
        
        if (player.hp <= 0) {
            return;
        }
        
        bool canRegenHp = true, canRegenMp = true;
        if (enemy.name == "斷律") {
            if (enemy.lawbreakerAbility == 2) canRegenHp = false;
            if (enemy.lawbreakerAbility == 3) canRegenMp = false;
        }
        player.regenerate(canRegenHp, canRegenMp);
        enemy.regenerate();
        cout << endl;
    }
}

// 初始化所有技能的函式
void initializeSkills() {
    PLAYER_SKILLS.push_back(Skill("普通攻擊", ATTACK, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("魔法彈", ATTACK, 5, 10, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("魔法切割", ATTACK, 5, 15, 0, 85, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("魔法彈+", ATTACK, 8, 20, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("魔法大砲", ATTACK, 15, 50, 0, 70, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("水砲", ATTACK, 10, 25, 0, 95, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("火球術", DEBUFF_ATTACK, 10, 20, 0, 90, MAGICAL, BURN, 3, 10));
    PLAYER_SKILLS.push_back(Skill("鬼火", DEBUFF_ATTACK, 5, 5, 0, 85, MAGICAL, BURN, 5, 10));
    PLAYER_SKILLS.push_back(Skill("煉獄", DEBUFF_ATTACK, 30, 65, 0, 50, MAGICAL, BURN, 10, 10));
    PLAYER_SKILLS.push_back(Skill("纏火衝鋒", BUFF_ATTACK, 10, 40, 0, 90, MAGICAL, BURN, 3, 10));
    PLAYER_SKILLS.push_back(Skill("冰箭術", DEBUFF_ATTACK, 8, 15, 0, 95, MAGICAL, FROST, 2, 15));
    PLAYER_SKILLS.push_back(Skill("閃電術", DEBUFF_ATTACK, 15, 30, 0, 80, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("打雷", DEBUFF_ATTACK, 25, 70, 0, 70, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("電擊", DEBUFF_ATTACK, 5, 5, 0, 90, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("復仇打擊", ATTACK, 15, 5, 0, 80, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("治療術", HEAL, 15, 0, 30, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("快速治療", HEAL, 5, 0, 12, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("強力治療術", HEAL, 30, 0, 55, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("強化防禦", BUFF, 10, 0, 0, 100, MAGICAL, GUARD, 2, 30));
    PLAYER_SKILLS.push_back(Skill("力量祝福", BUFF, 8, 0, 0, 100, MAGICAL, RAGE, 4, 20));
    PLAYER_SKILLS.push_back(Skill("增幅攻擊", BUFF_ATTACK, 10, 10, 0, 100, MAGICAL, RAGE, 2, 20));
    PLAYER_SKILLS.push_back(Skill("精準打擊", BUFF, 5, 0, 0, 100, MAGICAL, CALM, 4, 10));
    PLAYER_SKILLS.push_back(Skill("幻影步法", BUFF, 6, 0, 0, 100, MAGICAL, EVADE_CHANCE, 1, 50));
    PLAYER_SKILLS.push_back(Skill("淨化", UTILITY, 15, 0, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("毒箭", DEBUFF_ATTACK, 5, 5, 0, 100, MAGICAL, POISON, 3, 3));
    PLAYER_SKILLS.push_back(Skill("酸雨", DEBUFF_ATTACK, 10, 10, 0, 90, MAGICAL, POISON, 5, 3));
    PLAYER_SKILLS.push_back(Skill("劇毒鎖鏈", DEBUFF_ATTACK, 25, 20, 0, 95, MAGICAL, POISON, 6, 5));
    PLAYER_SKILLS.push_back(Skill("暴風雪", ATTACK, 25, 50, 0, 70, MAGICAL, FROST, 3, 15));
    PLAYER_SKILLS.push_back(Skill("神聖之光", ATTACK, 20, 40, 0, 85, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("衝擊波", ATTACK, 10, 30, 0, 80, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("虛弱詛咒", DEBUFF, 10, 0, 0, 100, MAGICAL, WEAK, 3, 20));
    PLAYER_SKILLS.push_back(Skill("出血攻擊", DEBUFF_ATTACK, 15, 15, 0, 90, MAGICAL, BLEED, 3, 5));
    PLAYER_SKILLS.push_back(Skill("冥想", BUFF, 15, 0, 0, 100, MAGICAL, MEDITATION, 5, 0));

    // 普通敵人技能
    ENEMY_SKILLS.push_back(Skill("爪擊", ATTACK, 0, 10, 0, 95, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("魔法彈", ATTACK, 0, 8, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("重擊", ATTACK, 0, 20, 0, 70, PHYSICAL, STUN, 1, 0));
    ENEMY_SKILLS.push_back(Skill("毒液噴射", DEBUFF_ATTACK, 0, 5, 0, 85, PHYSICAL, POISON, 3, 5));
    ENEMY_SKILLS.push_back(Skill("蛛網纏繞", DEBUFF, 0, 0, 0, 80, PHYSICAL, STUN, 2, 0));
    ENEMY_SKILLS.push_back(Skill("虛弱詛咒", DEBUFF, 0, 0, 0, 90, MAGICAL, WEAK, 3, 30));
    ENEMY_SKILLS.push_back(Skill("閃電術", ATTACK, 0, 30, 0, 80, MAGICAL, STUN, 1, 0));
    ENEMY_SKILLS.push_back(Skill("鬼火", DEBUFF_ATTACK, 0, 8, 0, 80, MAGICAL, BURN, 4, 8));
    ENEMY_SKILLS.push_back(Skill("帽子攻擊", ATTACK, 0, 30, 0, 90, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("聲援咆嘯", BUFF, 0, 0, 0, 100, MAGICAL, RAGE, 3, 15));
    // Boss 技能
    ENEMY_SKILLS.push_back(Skill("秒裂", DEBUFF_ATTACK, 0, 25, 0, 100, MAGICAL, WEAK, 10, 20));
    ENEMY_SKILLS.push_back(Skill("第十三時", HEAL, 0, 0, 50, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("時間齒輪", BUFF_ATTACK, 0, 40, 0, 100, PHYSICAL, RAGE, 10, 20));
    ENEMY_SKILLS.push_back(Skill("衰退", ATTACK, 0, 30, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("倒轉之輪", UTILITY, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("爐心爆破", DEBUFF_ATTACK, 0, 50, 0, 80, MAGICAL, BURN, 5, 10));
    ENEMY_SKILLS.push_back(Skill("冰結迴旋", DEBUFF_ATTACK, 0, 40, 0, 90, PHYSICAL, FROST, 3, 15));
    ENEMY_SKILLS.push_back(Skill("極光世界", ATTACK, 0, 45, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("雙極崩界", ATTACK, 0, 50, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("動作鎖鏈", ATTACK, 0, 60, 0, 60, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("絕路封鎖", DEBUFF, 0, 0, 0, 100, MAGICAL, HUNGER, 5, 5));
    ENEMY_SKILLS.push_back(Skill("裂界網", ATTACK, 0, 20, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("規則重編", ATTACK, 0, 0, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("虛無領域", BUFF, 0, 0, 0, 100, MAGICAL, GUARD, 5, 30));
    ENEMY_SKILLS.push_back(Skill("放逐協議", HEAL, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("現實扭曲", UTILITY, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("遺忘脈衝", ATTACK, 0, 50, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("最終審判", ATTACK, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));

    // 新增更多裝備模板
    ALL_EQUIPMENT_TEMPLATES.push_back({"木製頭盔", HELMET, M_DEFENSE_BONUS, 5, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"鐵盔", HELMET, DEFENSE_BONUS, 8, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"精靈頭冠", HELMET, MP_BONUS, 10, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"神聖光環", HELMET, HP_REGEN_BONUS, 3, 1, 3});
    ALL_EQUIPMENT_TEMPLATES.push_back({"皮甲", ARMOR, DEFENSE_BONUS, 10, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"鎖子甲", ARMOR, DEFENSE_BONUS, 15, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"龍鱗鎧甲", ARMOR, M_DEFENSE_BONUS, 12, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"法師長袍", ARMOR, MP_BONUS, 15, 3, 6});
    ALL_EQUIPMENT_TEMPLATES.push_back({"橡木法杖", STAFF, M_ATTACK_BONUS, 8, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"水晶法杖", STAFF, M_ATTACK_BONUS, 12, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"火焰法杖", STAFF, BURN_DAMAGE_BONUS, 5, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"寒冰法杖", STAFF, STATUS_DURATION_BONUS, 1, 1, 2});
    
    // 初始化所有效果類型列表
    ALL_EFFECT_TYPES.push_back(HP_BONUS);
    ALL_EFFECT_TYPES.push_back(MP_BONUS);
    ALL_EFFECT_TYPES.push_back(HP_REGEN_BONUS);
    ALL_EFFECT_TYPES.push_back(MP_REGEN_BONUS);
    ALL_EFFECT_TYPES.push_back(EXP_BONUS);
    ALL_EFFECT_TYPES.push_back(DEFENSE_BONUS);
    ALL_EFFECT_TYPES.push_back(M_DEFENSE_BONUS);
    ALL_EFFECT_TYPES.push_back(POISON_DAMAGE_BONUS);
    ALL_EFFECT_TYPES.push_back(BURN_DAMAGE_BONUS);
    ALL_EFFECT_TYPES.push_back(M_ATTACK_BONUS);
    ALL_EFFECT_TYPES.push_back(DAMAGE_REFLECT);
    ALL_EFFECT_TYPES.push_back(STATUS_DURATION_BONUS);
    ALL_EFFECT_TYPES.push_back(CLEANSE_PER_TURN);
    ALL_EFFECT_TYPES.push_back(M_ATTACK_WITH_STATUS);
}

// NEW: 處理樓層結束時的隨機事件
void handleFloorEvents(Player& player) {
    int eventRoll = getRandomNumber(1, 100);
    char choice;
    int roll;

    // 每個事件有 2% 的機率
    if (eventRoll <= 2) { // 事件 1: 不明的藥水
        SetColor(11);
        cout << "\n你發現了一瓶冒著氣泡的不明藥水，要喝下它嗎？ (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            roll = getRandomNumber(1, 100);
            if (roll <= 60) {
                player.hp += 20;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                SetColor(10);
                cout << "你感到精神一振，恢復了 20 點生命！\n";
            } else {
                player.hp -= 10;
                if (player.hp < 0) player.hp = 0;
                player.addStatus(POISON, 2, 5);
                SetColor(12);
                cout << "藥水嚐起來很噁心！你失去了 10 點生命並中毒了！\n";
            }
        } else {
            cout << "你小心翼翼地把藥水放回了原處。\n";
        }

    } else if (eventRoll <= 4) { // 事件 2: 陷阱
        SetColor(12);
        cout << "\n你踩到了一個隱藏的陷阱！\n";
        roll = getRandomNumber(1, 100);
        if (roll <= 30) {
            cout << "地板噴出火焰，你受到了 3 回合的灼傷！\n";
            player.addStatus(BURN, 3, 5);
        } else if (roll <= 60) {
            cout << "尖刺從牆壁射出，你受到了 20 點傷害！\n";
            player.hp -= 20;
            if (player.hp < 0) player.hp = 0;
        } else if (roll <= 90) {
            cout << "你吸入了毒氣，受到了 4 回合的中毒！\n";
            player.addStatus(POISON, 4, 5);
        } else {
            SetColor(10);
            cout << "在千鈞一髮之際，你驚險地躲過了陷阱！你因此獲得了 20 點經驗值。\n";
            player.gainExp(20);
        }

    } else if (eventRoll <= 6) { // 事件 3: 散落的書籍
        int expGained = getRandomNumber(10, 100);
        SetColor(10);
        cout << "\n你找到了一些散落的書籍，閱讀後對魔法的理解更深了。\n";
        player.gainExp(expGained);

    } else if (eventRoll <= 8) { // 事件 4: 食物
        SetColor(11);
        cout << "\n你找到了一些看起來還能吃的食物，要吃掉它嗎？ (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            roll = getRandomNumber(1, 100);
            if (roll <= 90) {
                player.hp += 30;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                player.mp += 15;
                if (player.mp > player.maxMp) player.mp = player.maxMp;
                SetColor(10);
                cout << "你恢復了 30 點生命和 15 點魔力。\n";
            } else {
                player.addStatus(POISON, 5, 5);
                SetColor(12);
                cout << "食物已經腐壞了！你中毒了！\n";
            }
        } else {
            cout << "你決定不冒這個險。\n";
        }

    } else if (eventRoll <= 10) { // 事件 5: 祝福 (HP)
        SetColor(10);
        cout << "\n一道溫暖的光芒籠罩著你，你感覺自己變得更強壯了。\n";
        cout << "最大生命值增加了 10 點！\n";
        player.bonusMaxHp += 10;
        player.calculateTotalStats();

    } else if (eventRoll <= 12) { // 事件 6: 祝福 (MP)
        SetColor(10);
        cout << "\n一股清涼的能量流過你的身體，你的思緒變得更清晰了。\n";
        cout << "最大魔力增加了 5 點！\n";
        player.bonusMaxMp += 5;
        player.calculateTotalStats();

    } else if (eventRoll <= 14) { // 事件 7: 詛咒
        SetColor(12);
        cout << "\n你感覺到一股邪惡的力量纏上了你，讓你感到虛弱。\n";
        cout << "你受到了 3 回合的虛弱狀態！\n";
        player.addStatus(WEAK, 3, 20);

    } else if (eventRoll <= 16) { // 事件 8: 神秘立場
        SetColor(11);
        cout << "\n你走進了一個神秘的立場，感覺到能量在拉扯你的身體。\n";
        cout << "1. 強化肉體，削弱精神 (+5 防禦, -5 魔防)\n";
        cout << "2. 強化精神，削弱肉體 (+5 魔防, -5 防禦)\n";
        cout << "3. 離開立場\n";
        cout << "請輸入你的選擇 (1-3): ";
        SetColor();
        int fieldChoice;
        cin >> fieldChoice;
        if (fieldChoice == 1) {
            cout << "你選擇了強化肉體。\n";
            player.bonusDefense += 5;
            player.bonusMagicDefense -= 5;
            player.calculateTotalStats();
        } else if (fieldChoice == 2) {
            cout << "你選擇了強化精神。\n";
            player.bonusMagicDefense += 5;
            player.bonusDefense -= 5;
            player.calculateTotalStats();
        } else {
            cout << "你選擇離開了這個奇怪的地方。\n";
        }

    } else if (eventRoll <= 18) { // 事件 9: 奇怪的祭壇
        SetColor(11);
        cout << "\n你發現了一個奇怪的祭壇，上面似乎可以獻上祭品。要消耗 10 點生命來祈禱嗎？ (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            player.hp -= 10;
            if (player.hp < 0) player.hp = 0;
            cout << "你將血液滴在祭壇上...\n";
            roll = getRandomNumber(1, 100);
            if (roll <= 30) {
                SetColor(10);
                cout << "祭壇發出光芒，你獲得了 50 點經驗值！\n";
                player.gainExp(50);
            } else if (roll <= 50) {
                cout << "祭壇沒有任何反應。\n";
            } else if (roll <= 90) {
                SetColor(10);
                cout << "你感覺到魔力回復的速度變快了！ (MP回復 +1)\n";
                player.mpRegen += 1;
            } else {
                SetColor(10);
                cout << "你感覺到傷口癒合的速度變快了！ (HP回復 +2)\n";
                player.hpRegen += 2;
            }
        } else {
            cout << "你對這個未知的祭壇保持敬畏，選擇了離開。\n";
        }

    } else if (eventRoll <= 20) { // 事件 10: 魔法陣
        int magicGained = getRandomNumber(5, 10);
        SetColor(10);
        cout << "\n你在地上發現一個殘破的魔法陣，修復它之後，你從中學到了一些知識。\n";
        cout << "魔法攻擊力增加了 " << magicGained << " 點！\n";
        player.bonusMagicAttack += magicGained;
        player.calculateTotalStats();

    } else if (eventRoll <= 22) { // 事件 11: 迷路
        SetColor(12);
        cout << "\n你在黑暗中迷失了方向，繞了很久才找到出路，感覺肚子很餓。\n";
        cout << "你獲得了 5 回合的飢餓狀態！\n";
        player.addStatus(HUNGER, 5, 2);

    } else if (eventRoll <= 24) { // 事件 12: 魔菇
        SetColor(11);
        cout << "\n牆上長著一朵奇怪的魔菇，看起來很誘人。要吃掉它嗎？ (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            roll = getRandomNumber(1, 100);
            if (roll <= 20) {
                SetColor(12);
                cout << "魔菇有毒！你中毒了 5 回合！\n";
                player.addStatus(POISON, 5, 5);
            } else if (roll <= 50) {
                SetColor(13);
                cout << "吃下魔菇後你感到一股無名火，進入了 3 回合的憤怒狀態！\n";
                player.addStatus(RAGE, 3, 20);
            } else if (roll <= 70) {
                SetColor(10);
                cout << "魔菇意外地美味，你恢復了 20 點生命。\n";
                player.hp += 20;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
            } else if (roll <= 90) {
                SetColor(12);
                cout << "這魔菇沒什麼味道，但你的肚子開始叫了。你進入了 3 回合的飢餓狀態。\n";
                player.addStatus(HUNGER, 3, 2);
            } else {
                SetColor(10);
                cout << "你從魔菇的孢子中獲得了 5 點經驗值。\n";
                player.gainExp(5);
            }
        } else {
            cout << "你決定不亂吃東西。\n";
        }

    } else if (eventRoll <= 26) { // 事件 13: 遺骸
        SetColor(9);
        cout << "\n你看到路邊有一具冒險者的遺骸，讓你對生命有了新的體悟。\n";
        cout << "你獲得了 4 回合的冷靜狀態。\n";
        player.addStatus(CALM, 4, 10);

    } else if (eventRoll <= 28) { // 事件 14: 寶箱
        SetColor(6);
        cout << "\n你發現了一個寶箱！\n";
        roll = getRandomNumber(1, 100);
        if (roll <= 60) {
            cout << "寶箱裡有一件裝備！\n";
            int equipTypeChoice = getRandomNumber(1, 3);
            EquipmentType type;
            if (equipTypeChoice == 1) type = HELMET;
            else if (equipTypeChoice == 2) type = ARMOR;
            else type = STAFF;
            Equipment newEquip = generateRandomEquipment(type);
            player.equipItem(newEquip);
        } else if (roll <= 90) {
            cout << "寶箱裡有一些食物，你恢復了 15 點生命。\n";
            player.hp += 15;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
        } else {
            SetColor(12);
            cout << "寶箱突然張開大嘴咬了你一口！原來是寶箱怪！\n";
            cout << "你受到了 30 點傷害，但擊敗它後獲得了 20 點經驗值。\n";
            player.hp -= 30;
            if (player.hp < 0) player.hp = 0;
            player.gainExp(20);
        }
    
    } else if (eventRoll <= 30) { // NEW: 事件 15: 對著神像祈禱
        SetColor(10);
        cout << "\n你發現一尊古老的神像，你虔誠地祈禱...\n";
        int statChoice = getRandomNumber(1, 5);
        switch (statChoice) {
            case 1:
                cout << "你感覺身體變得更堅韌了！ (最大生命成長點數 +2)\n";
                player.bonusMaxHp += 2;
                break;
            case 2:
                cout << "你感覺魔力更加充盈了！ (最大魔力成長點數 +2)\n";
                player.bonusMaxMp += 2;
                break;
            case 3:
                cout << "你對魔法的領悟加深了！ (魔法攻擊成長點數 +2)\n";
                player.bonusMagicAttack += 2;
                break;
            case 4:
                cout << "你的防禦技巧更精進了！ (物理防禦成長點數 +2)\n";
                player.bonusDefense += 2;
                break;
            case 5:
                cout << "你的心靈受到了庇護！ (魔法防禦成長點數 +2)\n";
                player.bonusMagicDefense += 2;
                break;
        }
        player.calculateTotalStats();

    } else if (eventRoll <= 50) { // 裝備掉落 (20% 機率, from 31 to 50)
        SetColor(14);
        cout << "\n敵人的殘骸中似乎有閃閃發光的東西...\n";
        SetColor();
        int equipTypeChoice = getRandomNumber(1, 3);
        EquipmentType type;
        if (equipTypeChoice == 1) type = HELMET;
        else if (equipTypeChoice == 2) type = ARMOR;
        else type = STAFF;
        
        Equipment newEquip = generateRandomEquipment(type);
        player.equipItem(newEquip);
    } else {
        // 什麼都沒發生
        cout << "\n你繼續前進，沒有發生什麼特別的事。\n";
    }
    SetColor();
    system("pause");
}

int main() {
    srand(time(0));
    
    initializeSkills();

    Player player;
    // 使用建構子初始化
    player.skills.push_back(PLAYER_SKILLS[0]);

    vector<Skill> availableStartSkills;
    for (size_t i = 1; i < PLAYER_SKILLS.size(); ++i) {
        availableStartSkills.push_back(PLAYER_SKILLS[i]);
    }
    
    random_shuffle(availableStartSkills.begin(), availableStartSkills.end());

    vector<Skill> choiceSkills;
    for (int i = 0; i < 3 && i < availableStartSkills.size(); ++i) {
        choiceSkills.push_back(availableStartSkills[i]);
    }

    SetColor(11);
    cout << "\n----------------------------------------\n";
    cout << "           選擇你的初始技能              \n";
    cout << "----------------------------------------\n";
    cout << "你已經學會了「" << player.skills[0].name << "」。\n";
    cout << "請從以下三個技能中，選擇一個作為你的初始技能：\n";
    for (size_t i = 0; i < choiceSkills.size(); ++i) {
        Skill currentSkill = choiceSkills[i];
        cout << i + 1 << ". 「" << currentSkill.name << "」 (MP消耗: " << currentSkill.mpCost << ")";
        if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
            cout << ", 傷害: " << currentSkill.baseDamage << ", 命中: " << currentSkill.hitRate << "%";
        } else if (currentSkill.type == HEAL) {
            cout << ", 治癒量: " << currentSkill.value;
        }
        cout << endl;
    }
    cout << "----------------------------------------\n";
    SetColor();
    
    int choice = 0;
    bool validChoice = false;
    while (!validChoice) {
        cout << "請輸入你的選擇 (1-3): ";
        cin >> choice;
        if (choice > 0 && choice <= choiceSkills.size()) {
            player.skills.push_back(choiceSkills[choice - 1]);
            validChoice = true;
            SetColor(6);
            cout << "你選擇了「" << player.skills.back().name << "」作為你的初始技能！\n";
            SetColor();
            system("pause");
        } else {
            cout << "無效的選擇，請重新輸入。\n";
        }
    }


    for (int floor = 1; floor <= 50; ++floor) {
        SetColor(11);
        system("cls");
        cout << "\n=== Floor " << floor << " ===\n";
        SetColor();
        
        Enemy enemy;
        if (floor == 50) {
            enemy.name = "Exile";
            enemy.hp = 600;
            enemy.maxHp = 600;
            enemy.expValue = 1000;
            enemy.skills.push_back(ENEMY_SKILLS[23]); // 虛無領域
            enemy.skills.push_back(ENEMY_SKILLS[24]); // 放逐協議
            enemy.skills.push_back(ENEMY_SKILLS[25]); // 現實扭曲
            enemy.skills.push_back(ENEMY_SKILLS[26]); // 遺忘脈衝
            enemy.skills.push_back(ENEMY_SKILLS[27]); // 最終審判
        } else if (floor == 40) {
            enemy.name = "斷律";
            enemy.hp = 500;
            enemy.maxHp = 500;
            enemy.expValue = 550;
            enemy.skills.push_back(ENEMY_SKILLS[19]); // 動作鎖鏈
            enemy.skills.push_back(ENEMY_SKILLS[20]); // 絕路封鎖
            enemy.skills.push_back(ENEMY_SKILLS[21]); // 裂界網
            enemy.skills.push_back(ENEMY_SKILLS[22]); // 規則重編
        } else if (floor == 30) {
            enemy.name = "恆溫者";
            enemy.hp = 400;
            enemy.maxHp = 400;
            enemy.expValue = 250;
            enemy.skills.push_back(ENEMY_SKILLS[15]); // 爐心爆破
            enemy.skills.push_back(ENEMY_SKILLS[16]); // 冰結迴旋
            enemy.skills.push_back(ENEMY_SKILLS[17]); // 極光世界
            enemy.skills.push_back(ENEMY_SKILLS[18]); // 雙極崩界
        } else if (floor == 20) {
            enemy.name = "克羅諾斯";
            enemy.hp = 300;
            enemy.maxHp = 300;
            enemy.expValue = 150;
            enemy.hpRegen = 10;
            enemy.skills.push_back(ENEMY_SKILLS[10]); // 秒裂
            enemy.skills.push_back(ENEMY_SKILLS[11]); // 第十三時
            enemy.skills.push_back(ENEMY_SKILLS[12]); // 時間齒輪
            enemy.skills.push_back(ENEMY_SKILLS[13]); // 衰退
            enemy.skills.push_back(ENEMY_SKILLS[14]); // 倒轉之輪
        } else {
            enemy = createEnemy(floor);
        }

        startBattle(player, enemy);

        if (player.hp <= 0) {
            cout << "\n遊戲結束。\n";
            break;
        }

        if (player.hp > 0 && floor < 50) {
            // 呼叫隨機事件函式
            handleFloorEvents(player);

            // 獨立的技能學習機會
            if (getRandomNumber(1, 100) > floor) {
                SetColor(6);
                cout << "\n在戰鬥過後，你對魔力的掌控更加熟練了，似乎可以學會新的技能！\n";
                
                vector<Skill> availableSkills;
                // 找出玩家尚未學會的技能
                for (size_t i = 1; i < PLAYER_SKILLS.size(); ++i) {
                    bool alreadyLearned = false;
                    for (size_t j = 0; j < player.skills.size(); ++j) {
                        if (PLAYER_SKILLS[i].name == player.skills[j].name) {
                            alreadyLearned = true;
                            break;
                        }
                    }
                    if (!alreadyLearned) {
                        availableSkills.push_back(PLAYER_SKILLS[i]);
                    }
                }

                if (availableSkills.empty()) {
                    cout << "但你似乎已經學會了所有技能。\n";
                    system("pause");
                } else {
                    random_shuffle(availableSkills.begin(), availableSkills.end());
                    vector<Skill> choiceSkills;
                    // 提供最多 3 個選項
                    for (int i = 0; i < 3 && i < availableSkills.size(); ++i) {
                        choiceSkills.push_back(availableSkills[i]);
                    }

                    cout << "請從以下技能中選擇一個學習 (輸入 0 放棄)：\n";
                    for (size_t i = 0; i < choiceSkills.size(); ++i) {
                        Skill currentSkill = choiceSkills[i];
                        cout << i + 1 << ". 「" << currentSkill.name << "」 (MP消耗: " << currentSkill.mpCost << ")\n";
                    }
                    int skillChoice;
                    cin >> skillChoice;
                    if (skillChoice > 0 && skillChoice <= choiceSkills.size()) {
                        player.learnSkill(choiceSkills[skillChoice - 1]);
                    } else {
                        cout << "你決定鞏固現有的知識，暫時不學習新技能。\n";
                        system("pause");
                    }
                }
            }
        }
    }

    if (player.hp > 0) {
        SetColor(10);
        cout << "\n恭喜你！你成功地通關了地牢！\n";
        SetColor();
    }

    return 0;
}
