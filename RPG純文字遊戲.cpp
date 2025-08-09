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

// �C����ܤu��
void SetColor(int color = 7) {
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// �ޯ�����
enum SkillType {
    ATTACK,
    HEAL,
    BUFF,
    DEBUFF,
    BUFF_ATTACK,
    DEBUFF_ATTACK,
    UTILITY
};

// �ˮ`����
enum DamageType {
    PHYSICAL,
    MAGICAL
};

// ���A�ĪG
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

// �˳Ƶ}����
enum Rarity {
    COMMON,     // �զ� +0 (50%)
    UNCOMMON,   // ��� +1 (32%)
    RARE,       // �Ŧ� +3 (14%)
    EPIC,       // ���� +5 (3%)
    LEGENDARY   // ��� +9 (1%)
};

// �˳�����
enum EquipmentType {
    NONE_EQUIP,
    HELMET,
    ARMOR,
    STAFF
};

// �˳ƮĪG����
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

// ²���H���Ʋ��;�
int getRandomNumber(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    return rand() % (max - min + 1) + min;
}

// �ĪG���c��
struct Effect {
    EquipmentEffectType type;
    int value;

    // C++98 �غc�l
    Effect(EquipmentEffectType t, int v) : type(t), value(v) {}
};

// �˳Ƶ��c��
struct Equipment {
    string name;
    EquipmentType type;
    Rarity rarity;
    int rarityValue;
    vector<Effect> effects;
    int bonus; // �s�W���B�~�H���[����
    
    // C++98 �غc�l
    Equipment(string n, EquipmentType et, Rarity r, int rv, const vector<Effect>& ef, int b = 0) : name(n), type(et), rarity(r), rarityValue(rv), effects(ef), bonus(b) {}
    Equipment() : name("�L"), type(NONE_EQUIP), rarity(COMMON), rarityValue(0), bonus(0) {} // �w�]�Ÿ˳�
};

// �˳ƼҪO���c��A�Ω���״I���H���ͦ�
struct EquipmentTemplate {
    string name;
    EquipmentType type;
    EquipmentEffectType effectType;
    int baseStat;
    int bonusMin;
    int bonusMax;
};

// ���A���c��
struct Status {
    StatusEffect type;
    int duration;
    int value;
    int turnCount;

    // C++98 �غc�l
    Status(StatusEffect t, int d, int v) : type(t), duration(d), value(v), turnCount(0) {}
};

// �ޯ൲�c��
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

    // C++98 �غc�l�A���� 'v' �Ѽ�
    Skill(string n, SkillType t, int mc, int bd, int v, int hr, DamageType dt, StatusEffect si, int sd, int sv) 
        : name(n), type(t), mpCost(mc), baseDamage(bd), value(v), hitRate(hr), damageType(dt), statusToInflict(si), statusDuration(sd), statusValue(sv) {}
};

// �ĤH���c��
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

    // Boss �M���ܼ�
    int temperatureValue;
    int lawbreakerAbility; // 0:None, 1:NoStatus, 2:NoHPRegen, 3:NoMPRegen, 4:NoAttack
    int exileGauge;
    int damageTakenThisTurn;


    // C++98 �غc�l�A��l�Ʀ����ܼ�
    Enemy() : name(""), hp(0), maxHp(0), expValue(0), attack(0), magicAttack(0), hpRegen(0), 
              temperatureValue(50), lawbreakerAbility(0), exileGauge(0), damageTakenThisTurn(0) {}

    void addStatus(StatusEffect type, int duration, int value) {
        if (type == BLEED) {
            activeStatuses.push_back(Status(type, duration, value));
            return;
        }

        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == type) {
                // MODIFICATION: �����㪬�A�i�H�|�[
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

// ���a���c��
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

    // MODIFICATION: �s�W�����ܼƨ��x�s�ɯ���o���B�~�ݩ�
    int bonusMaxHp;
    int bonusMaxMp;
    int bonusMagicAttack;
    int bonusDefense;
    int bonusMagicDefense;

    // �˳����
    Equipment helmet;
    Equipment armor;
    Equipment staff;

    vector<Status> activeStatuses;
    vector<Skill> skills;
    const int MAX_SKILLS = 5;

    // C++98 �غc�l
    // MODIFICATION: ��l�Ʒs���B�~�ݩʬ� 0
    Player() : hp(100), maxHp(100), mp(50), maxMp(50), magicAttack(10), defense(5), magicDefense(5),
               hpRegen(2), mpRegen(1), level(1), exp(0), expToNextLevel(30 + 20 * level),
               bonusMaxHp(0), bonusMaxMp(0), bonusMagicAttack(0), bonusDefense(0), bonusMagicDefense(0) {}

    // MODIFICATION: ��s�禡�H���T�p���`�ݩ�
    void calculateTotalStats() {
        // 1. �ھڵ��ŭp���¦�ݩ�
        int baseMaxHp = 100 + (level - 1) * 10;
        int baseMaxMp = 50 + (level - 1) * 5;
        int baseMagicAttack = 10 + (level - 1);
        int baseDefense = 5 + (level - 1) * 2;
        int baseMagicDefense = 5 + (level - 1) * 2;
        int baseHpRegen = 2;
        int baseMpRegen = 1;

        // 2. �N�ɯ���o���B�~�ݩʥ[���¦�ݩʤW
        maxHp = baseMaxHp + bonusMaxHp;
        maxMp = baseMaxMp + bonusMaxMp;
        magicAttack = baseMagicAttack + bonusMagicAttack;
        defense = baseDefense + bonusDefense;
        magicDefense = baseMagicDefense + bonusMagicDefense;
        hpRegen = baseHpRegen;
        mpRegen = baseMpRegen;

        // 3. �M�θ˳ƪ��[��
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
        cout << "            �����O��              \n";
        cout << "----------------------------------------\n";
        cout << "����: " << level << endl;
        cout << "�g���: " << exp << "/" << expToNextLevel << endl;
        cout << "HP: " << hp << "/" << maxHp << endl;
        cout << "MP: " << mp << "/" << maxMp << endl;
        cout << "�]�k�����O: " << magicAttack << endl;
        cout << "���z���m�O: " << defense << endl;
        cout << "�]�k���m�O: " << magicDefense << endl;
        cout << "HP�^�_: " << hpRegen << "/�^�X" << endl;
        cout << "MP�^�_: " << mpRegen << "/�^�X" << endl;
        cout << "----------------------------------------\n";
        cout << "            ��e���A              \n";
        cout << "----------------------------------------\n";
        if (activeStatuses.empty()) {
            cout << "�ثe�S�����󪬺A�C\n";
        } else {
            for (size_t i = 0; i < activeStatuses.size(); ++i) {
                Status status = activeStatuses[i];
                string statusName;
                switch (status.type) {
                    case BURN: statusName = "�`��"; break;
                    case POISON: statusName = "���r"; break;
                    case STUN: statusName = "�w�t"; break;
                    case BLEED: statusName = "�X��"; break;
                    case WEAK: statusName = "��z"; break;
                    case FROST: statusName = "�H�N"; break;
                    case GUARD: statusName = "�u�@"; break;
                    case RAGE: statusName = "����"; break;
                    case CALM: statusName = "�N�R"; break;
                    case HUNGER: statusName = "���j"; break;
                    case MEDITATION: statusName = "�߷Q"; break;
                    case EVADE_CHANCE: statusName = "�j��"; break;
                    default: statusName = "�������A"; break;
                }
                cout << "- " << statusName << " (�Ѿl�^�X: " << status.duration << ")";
                if (status.type == POISON) cout << " �ˮ`: " << status.value + status.turnCount;
                if (status.type == BLEED) cout << " �B�~�ˮ`: " << status.value;
                if (status.type == WEAK) cout << " ����: " << status.value << "%";
                if (status.type == FROST) cout << " �ˮ`��K: " << status.value << "%";
                if (status.type == GUARD) cout << " �ˮ`��K: " << status.value << "%";
                if (status.type == RAGE) cout << " �ˮ`�W�[: " << status.value << "%";
                if (status.type == EVADE_CHANCE) cout << " �{�ײv: " << status.value << "%";
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        displayEquipment();
        cout << "----------------------------------------\n";
        cout << "            �֦����ޯ�              \n";
        cout << "----------------------------------------\n";
        if (skills.empty()) {
            cout << "�S���w�Ƿ|���ޯ�C\n";
        } else {
            for (size_t i = 0; i < skills.size(); ++i) {
                Skill currentSkill = skills[i];
                cout << i + 1 << ". �u" << currentSkill.name << "�v (MP����: " << currentSkill.mpCost << ")";
                if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
                    cout << ", �ˮ`: " << currentSkill.baseDamage << ", �R��: " << currentSkill.hitRate << "%";
                } else if (currentSkill.type == HEAL) {
                    cout << ", �v¡�q: " << currentSkill.value;
                }
                if (currentSkill.statusToInflict != NONE) {
                    string statusName;
                    switch (currentSkill.statusToInflict) {
                        case BURN: statusName = "�`��"; break;
                        case POISON: statusName = "���r"; break;
                        case STUN: statusName = "�w�t"; break;
                        case BLEED: statusName = "�X��"; break;
                        default: statusName = "�������A"; break;
                    }
                    cout << " (�I�[���A: " << statusName << " " << currentSkill.statusDuration << "�^�X)";
                }
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        SetColor();
        system("pause");
    }

    // ��ܪ��a���˳�
    void displayEquipment() {
        cout << "             �˳ƫ~              \n";
        cout << "----------------------------------------\n";
        cout << "�Y��: " << helmet.name << endl;
        cout << "����: " << armor.name << endl;
        cout << "�k��: " << staff.name << endl;
        cout << "----------------------------------------\n";
    }

    // �s�W�G����˳ƯS�w�ĪG���`��
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
        cout << "\n�A��o�F�s�˳ơI\n";
        
        Equipment oldItem;
        if (item.type == HELMET) oldItem = helmet;
        else if (item.type == ARMOR) oldItem = armor;
        else if (item.type == STAFF) oldItem = staff;
        
        cout << "\n----------------------------------------\n";
        cout << "            �s�˳��ݩ�              \n";
        cout << "----------------------------------------\n";
        
        // �ץ��᪺�C������޿�
        cout << "�W��: �u" << item.name << "�v (";
        switch (item.rarity) {
            case COMMON: SetColor(15); cout << "�զ�"; break;
            case UNCOMMON: SetColor(10); cout << "���"; break;
            case RARE: SetColor(9); cout << "�Ŧ�"; break;
            case EPIC: SetColor(13); cout << "����"; break;
            case LEGENDARY: SetColor(6); cout << "���"; break;
        }
        SetColor(11); // ���]�C��
        cout << ")\n";
        
        cout << "����: ";
        switch (item.type) {
            case HELMET: cout << "�Y��"; break;
            case ARMOR: cout << "����"; break;
            case STAFF: cout << "�k��"; break;
        }
        cout << endl;
        cout << "�ݩʥ[��:\n";
        for (size_t i = 0; i < item.effects.size(); ++i) {
            Effect effect = item.effects[i];
            cout << " - ";
            switch (effect.type) {
                case HP_BONUS: cout << "�̤jHP: "; break;
                case MP_BONUS: cout << "�̤jMP: "; break;
                case HP_REGEN_BONUS: cout << "HP�^�_: "; break;
                case MP_REGEN_BONUS: cout << "MP�^�_: "; break;
                case EXP_BONUS: cout << "�g�����o: "; break;
                case DEFENSE_BONUS: cout << "���z���m: "; break;
                case M_DEFENSE_BONUS: cout << "�]�k���m: "; break;
                case POISON_DAMAGE_BONUS: cout << "���r�ˮ`: "; break;
                case BURN_DAMAGE_BONUS: cout << "�`�˶ˮ`: "; break;
                case M_ATTACK_BONUS: cout << "�]�k����: "; break;
                case DAMAGE_REFLECT: cout << "�ˮ`�ϼu: "; break;
                case STATUS_DURATION_BONUS: cout << "���A����ɶ�: "; break;
                case CLEANSE_PER_TURN: cout << "�C�^�X�b�Ʀۨ�"; break;
                case M_ATTACK_WITH_STATUS: cout << "�ۨ������A���]�k�����W�[: "; break;
            }
            if (effect.type != CLEANSE_PER_TURN) {
                 cout << (effect.value > 0 ? "+" : "") << effect.value;
                 if (effect.type == M_ATTACK_BONUS || effect.type == DEFENSE_BONUS || effect.type == M_DEFENSE_BONUS || effect.type == HP_BONUS || effect.type == MP_BONUS) {
                     // ��ܰ�¦�ȩM�B�~�[��
                     stringstream ss;
                     ss << (item.rarityValue + item.bonus);
                     cout << " (��¦��:" << ss.str() << ")";
                 }
                 cout << endl;
            } else {
                cout << endl;
            }
        }
        cout << "----------------------------------------\n";
        
        // ����\��G�p�G�¸˳Ƥ�����
        if (oldItem.type != NONE_EQUIP) {
            cout << "  **�P�ثe�˳ơu" << oldItem.name << "�v���**\n";
            cout << "----------------------------------------\n";
            // ��X�Ҧ��s�¸˳Ƨt�����ĪG����
            map<EquipmentEffectType, string> effectNames;
            effectNames[HP_BONUS] = "�̤jHP";
            effectNames[MP_BONUS] = "�̤jMP";
            effectNames[HP_REGEN_BONUS] = "HP�^�_";
            effectNames[MP_REGEN_BONUS] = "MP�^�_";
            effectNames[EXP_BONUS] = "�g�����o";
            effectNames[DEFENSE_BONUS] = "���z���m";
            effectNames[M_DEFENSE_BONUS] = "�]�k���m";
            effectNames[POISON_DAMAGE_BONUS] = "���r�ˮ`";
            effectNames[BURN_DAMAGE_BONUS] = "�`�˶ˮ`";
            effectNames[M_ATTACK_BONUS] = "�]�k����";
            effectNames[DAMAGE_REFLECT] = "�ˮ`�ϼu";
            effectNames[STATUS_DURATION_BONUS] = "���A����ɶ�";
            effectNames[CLEANSE_PER_TURN] = "�C�^�X�b�Ʀۨ�";
            effectNames[M_ATTACK_WITH_STATUS] = "�ۨ������A���]�k�����W�[";

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
                        SetColor(10); // ���
                        diffStr = "(+" + diffStr + ")";
                    } else {
                        SetColor(12); // ����
                        diffStr = "(" + diffStr + ")";
                    }
                    cout << effectNames[allEffectTypes[i]] << ": " << (newTotal > 0 ? "+" : "") << newTotal << " " << diffStr << endl;
                }
            }

            if (!hasComparison) {
                SetColor(15);
                cout << "�L�ݩʮt���C\n";
            }
            SetColor(11);
            cout << "----------------------------------------\n";
        }

        cout << "�O�_�n�˳ƥ��H (y/n)\n";
        SetColor();

        char choice;
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            switch (item.type) {
                case HELMET:
                    if (helmet.type != NONE_EQUIP) {
                        cout << "�A���U�F�u" << helmet.name << "�v�C\n";
                    }
                    helmet = item;
                    cout << "�u" << helmet.name << "�v�w�˳ơC\n";
                    break;
                case ARMOR:
                    if (armor.type != NONE_EQUIP) {
                        cout << "�A���U�F�u" << armor.name << "�v�C\n";
                    }
                    armor = item;
                    cout << "�u" << armor.name << "�v�w�˳ơC\n";
                    break;
                case STAFF:
                    if (staff.type != NONE_EQUIP) {
                        cout << "�A���U�F�u" << staff.name << "�v�C\n";
                    }
                    staff = item;
                    cout << "�u" << staff.name << "�v�w�˳ơC\n";
                    break;
            }
            calculateTotalStats(); // ���s�p���ݩ�
        } else {
            cout << "�A��ܤ��˳ƥ��A�˳ƳQ���F�C\n";
        }
        system("pause");
    }

    // MODIFICATION: ���c�ɯ��޿�
	void levelUp() {
 	    SetColor(2);
 	    level++;
 	    exp -= expToNextLevel;
   	 	expToNextLevel = 30 + 20 * level; 

   	 	cout << "\n****************************************\n";
   		cout << "* LEVEL UP! �A�ɨ�F���� " << level << "! *\n";
  	 	cout << "****************************************\n";
  	 	SetColor(7);

  	  	int choice = 0;
    	do {
        	cout << "�п�ܭn���ɪ��ݩʡG\n";
        	cout << "0. �d�ݥثe��O��\n";
        	cout << "1. ���� Max HP (+10)\n";
        	cout << "2. ���� Max MP (+5)\n";
        	cout << "3. ���� �]�k���� (+2)\n";
        	cout << "4. ���� ���z���m (+3)\n";
        	cout << "5. ���� �]�k���m (+3)\n";
        	cout << "�п�J�A����� (0-5): ";
        	cin >> choice;

            // ���\�b�����h��ܴ��ܪ����p�U�d�ݪ��A
            if (choice == 0) {
                showStatus();
                choice = -1; // ���s�j��
                continue;
            }

            // �N��ܪ��[�����Ω��B�~�ݩ��ܼ�
        	switch (choice) {
            	case 1: 
                	bonusMaxHp += 10; 
                	cout << "�̤jHP���ɯ��I�ƼW�[�F�I\n"; 
               		break;
            	case 2: 
                	bonusMaxMp += 5; 
                	cout << "�̤jMP���ɯ��I�ƼW�[�F�I\n"; 
                	break;
            	case 3: 
                    bonusMagicAttack += 2; 
                    cout << "�]�k�������ɯ��I�ƼW�[�F�I\n"; 
                    break;
            	case 4: 
                    bonusDefense += 3; 
                    cout << "���z���m���ɯ��I�ƼW�[�F�I\n"; 
                    break;
            	case 5: 
                    bonusMagicDefense += 3; 
                    cout << "�]�k���m���ɯ��I�ƼW�[�F�I\n"; 
                    break;
            	default: 
                    cout << "�L�Ī���ܡA�Э��s��J�C\n"; 
                    choice = -1; // ���s�j��
                    break;
    	    }
   		} while (choice == -1);

        // �{�b�A���s�p��Ҧ��ݩʥH�]�t�s���[��
        calculateTotalStats(); 
    
        // �ɯŮɧ����v¡���a
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
        cout << "�A��o�F " << amount << " �I�g���";
        if (totalExpBonus > 0) {
            cout << " (�B�~��o " << totalExpBonus << " �I)";
        }
        cout << "�C\n";
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
            cout << "�A���\�{�פF�����I\n";
            SetColor();
            return;
        }

        for (size_t i = 0; i < armor.effects.size(); ++i) {
            if (armor.effects[i].type == DAMAGE_REFLECT) {
                int reflectDamage = armor.effects[i].value;
                enemy.hp -= reflectDamage;
                cout << "�A�����Ҥϼu�F " << reflectDamage << " �I�ˮ`���ĤH�u" << enemy.name << "�v�I\n";
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
            damageTypeStr = "���z";
        } else if (type == MAGICAL) {
            damageTaken = (int)finalDamage - magicDefense;
            damageTypeStr = "�]�k";
        }
        
        if (damageTaken < 1) damageTaken = 1;
        hp -= damageTaken;
        if (hp < 0) hp = 0;
        SetColor(4);
        cout << "�A����F " << damageTaken << " �I " << damageTypeStr << " �ˮ`�I\n";
        SetColor();

        // �N�ˮ`�O���� Boss ���p�ƾ���
        enemy.damageTakenThisTurn += damageTaken;

        for (size_t i = 0; i < activeStatuses.size(); ++i) {
            if (activeStatuses[i].type == BLEED) {
                int bleedDamage = activeStatuses[i].value;
                hp -= bleedDamage;
                if (hp < 0) hp = 0;
                SetColor(4);
                cout << "�A�]�X���B�~����F " << bleedDamage << " �I�ˮ`�I\n";
                SetColor();
            }
        }
    }

    void printStatus() {
        SetColor(14);
        cout << "[���a Lv: " << level << "] HP: " << hp << "/" << maxHp
             << ", MP: " << mp << "/" << maxMp
             << ", EXP: " << exp << "/" << expToNextLevel;
        if (!activeStatuses.empty()) {
            cout << " (���A:";
            for (size_t i = 0; i < activeStatuses.size(); ++i) {
                 Status status = activeStatuses[i];
                 string statusName;
                switch (status.type) {
                    case BURN: statusName = "�`��"; break;
                    case POISON: statusName = "���r"; break;
                    case STUN: statusName = "�w�t"; break;
                    case BLEED: statusName = "�X��"; break;
                    case WEAK: statusName = "��z"; break;
                    case FROST: statusName = "�H�N"; break;
                    case GUARD: statusName = "�u�@"; break;
                    case RAGE: statusName = "����"; break;
                    case CALM: statusName = "�N�R"; break;
                    case HUNGER: statusName = "���j"; break;
                    case MEDITATION: statusName = "�߷Q"; break;
                    case EVADE_CHANCE: statusName = "�j��"; break;
                    default: statusName = "�������A"; break;
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
        cout << "���ߡI�A�Ƿ|�F�s�ޯ�G�u" << newSkill.name << "�v�I\n";
        cout << "----------------------------------------\n";
        
        if (skills.size() < MAX_SKILLS) {
            skills.push_back(newSkill);
            cout << "�ޯ�u" << newSkill.name << "�v�w�[�J�A���ޯ�M��C\n";
        } else {
            cout << "�A���ޯ�M��w���C�п�ܤ@�ӧޯ���� (��J 0 ���): \n";
            for (size_t i = 0; i < skills.size(); ++i) {
                cout << i + 1 << ". �����u" << skills[i].name << "�v\n";
            }
            int choice;
            cout << "�п�J�A�����: ";
            cin >> choice;

            if (choice > 0 && choice <= MAX_SKILLS) {
                cout << "�A���F�u" << skills[choice - 1].name << "�v\n";
                skills[choice - 1] = newSkill;
                cout << "�ޯ�u" << newSkill.name << "�v���\�����F���C\n";
            } else {
                cout << "�A���F�ǲ߷s�ޯ�C\n";
            }
        }
        SetColor();
        system("pause");
    }
};

// ���a�M�ĤH���W�ߧޯ�M��
vector<Skill> PLAYER_SKILLS;
vector<Skill> ENEMY_SKILLS;

// �s�W�˳ƼҪO�C��
vector<EquipmentTemplate> ALL_EQUIPMENT_TEMPLATES;
vector<EquipmentEffectType> ALL_EFFECT_TYPES;

// MODIFICATION: �s�W�@�ӥ����ܼƨӥN��S���u��U�v
Equipment greenHat("��U", HELMET, COMMON, 0, vector<Effect>());

// �ھڵ}������o�ƭ�
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

// �ھھ��v�H����o�}����
Rarity getRandomRarity() {
    int chance = getRandomNumber(1, 100);
    if (chance <= 1) return LEGENDARY;      // 1%
    if (chance <= 4) return EPIC;           // 3%
    if (chance <= 18) return RARE;          // 14%
    if (chance <= 50) return UNCOMMON;      // 32%
    return COMMON;                          // 50%
}

// �ھ������M�}�����H���ͦ��˳�
Equipment generateRandomEquipment(EquipmentType type) {
    Rarity rarity = getRandomRarity();
    int rarityValue = getRarityValue(rarity);

    // �ھڸ˳������z��ҪO
    vector<EquipmentTemplate> filteredTemplates;
    for (size_t i = 0; i < ALL_EQUIPMENT_TEMPLATES.size(); ++i) {
        if (ALL_EQUIPMENT_TEMPLATES[i].type == type) {
            filteredTemplates.push_back(ALL_EQUIPMENT_TEMPLATES[i]);
        }
    }

    if (filteredTemplates.empty()) {
        return Equipment(); // ��^�Ÿ˳�
    }

    // �H����ܤ@�ӼҪO
    EquipmentTemplate chosenTemplate = filteredTemplates[getRandomNumber(0, filteredTemplates.size() - 1)];

    // �����B�~���H���[����
    int bonus = getRandomNumber(chosenTemplate.bonusMin, chosenTemplate.bonusMax);
    int totalValue = chosenTemplate.baseStat + rarityValue + bonus;

    vector<Effect> effects;
    // �D�n�ݩ�
    effects.push_back(Effect(chosenTemplate.effectType, totalValue));
    
    // �ǻ��˳Ʀ��S���ݩ�
    if (rarity == LEGENDARY) {
        int choice = getRandomNumber(1, 3);
        if (type == HELMET && choice == 1) effects.push_back(Effect(CLEANSE_PER_TURN, 1));
        else if (type == ARMOR && choice == 1) effects.push_back(Effect(DAMAGE_REFLECT, rarityValue + bonus));
        else if (type == STAFF && choice == 1) effects.push_back(Effect(STATUS_DURATION_BONUS, 2));
    }
    
    // �p�G�}���׬�RARE�H�W�A�B�~�����@���H���ĪG
    if (rarity >= RARE) {
        vector<EquipmentEffectType> secondaryEffectPool;
        for (size_t i = 0; i < ALL_EFFECT_TYPES.size(); ++i) {
            if (ALL_EFFECT_TYPES[i] != chosenTemplate.effectType) {
                // �����ĤG�ӮĪG�M�Ĥ@�ӮĪG����
                secondaryEffectPool.push_back(ALL_EFFECT_TYPES[i]);
            }
        }

        if (!secondaryEffectPool.empty()) {
            EquipmentEffectType secondaryEffectType = secondaryEffectPool[getRandomNumber(0, secondaryEffectPool.size() - 1)];
            int secondaryValue = getRandomNumber(1, rarityValue); // �ĤG�ӮĪG���ȸ��p
            effects.push_back(Effect(secondaryEffectType, secondaryValue));
        }
    }

    // �R�W�W�h
    map<Rarity, string> rarityPrefixMap;
    rarityPrefixMap[COMMON] = "���q��";
    rarityPrefixMap[UNCOMMON] = "�u����";
    rarityPrefixMap[RARE] = "�}����";
    rarityPrefixMap[EPIC] = "�v�֪�";
    rarityPrefixMap[LEGENDARY] = "�ǻ���";
    
    string finalName = rarityPrefixMap[rarity] + " " + chosenTemplate.name;
    
    return Equipment(finalName, type, rarity, rarityValue, effects, bonus);
}

// �إ��H���ĤH
Enemy createEnemy(int level) {
    Enemy e;
    int type = getRandomNumber(0, 6); 
    e.attack = 5 + level;
    e.magicAttack = 5 + level;
    if (type == 0) {
        e.name = "�v�ܩi";
        e.hp = 30 + level * 4;
        e.maxHp = e.hp;
        e.expValue = 10 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[1]);
    } else if (type == 1) {
        e.name = "�����L";
        e.hp = 40 + level * 5;
        e.maxHp = e.hp;
        e.expValue = 15 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        // MODIFICATION: �������L�Ƿ|�s���u�n���H�S�v�ޯ�
        e.skills.push_back(ENEMY_SKILLS[9]); 
    } else if (type == 2) {
        e.name = "�u�\�Ԥh";
        e.hp = 30 + level * 2 + getRandomNumber(5, 15);
        e.maxHp = e.hp;
        e.expValue = 13 + level;
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[2]);
    } else if (type == 3) {
        e.name = "�r�j��";
        e.hp = 25 + level * 2 + getRandomNumber(3, 10);
        e.maxHp = e.hp;
        e.expValue = 25 + level;
        e.skills.push_back(ENEMY_SKILLS[3]);
        e.skills.push_back(ENEMY_SKILLS[4]);
    } else if (type == 4) {
        e.name = "�L��";
        e.hp = 50 + level * 4 + getRandomNumber(5, 30);
        e.maxHp = e.hp;
        e.expValue = 20 + level + getRandomNumber(5, 10);
        e.skills.push_back(ENEMY_SKILLS[0]);
        e.skills.push_back(ENEMY_SKILLS[5]);
    } else if (type == 5) {
        e.name = "�Ův";
        e.hp = 35 + level * 3;
        e.maxHp = e.hp;
        e.expValue = 22 + level;
        e.skills.push_back(ENEMY_SKILLS[1]);
        e.skills.push_back(ENEMY_SKILLS[6]);
    } else { // type == 6
        e.name = "���Y�H";
        e.hp = 80 + level * 10;
        e.maxHp = e.hp;
        e.expValue = 40 + level * 2;
        e.skills.push_back(ENEMY_SKILLS[7]);
        // MODIFICATION: �����Y�H�Ƿ|�u����U�l�v
        e.skills.push_back(ENEMY_SKILLS[8]);
    }
    return e;
}

// �԰��y�{
void startBattle(Player& player, Enemy& enemy) {
    SetColor(13);
    cout << "���ͪ� " << enemy.name << " �X�{�F!" << endl;
    SetColor();
    
    if (enemy.name == "���Y�H") {
        SetColor(12);
        cout << enemy.name << " �o�X�F���㪺�H���I���i�J�F���㪬�A�I\n";
        enemy.addStatus(RAGE, 10, 20); // ����10�^�X�A�����O+20%
        SetColor();
    }

    player.calculateTotalStats();

    while (player.hp > 0 && enemy.hp > 0) {
        cout << "\n-- �^�X�}�l --\n";
        enemy.damageTakenThisTurn = 0; // �C�^�X���m�ˮ`�p�ƾ�
        
        // Boss �Q�ʧޯඥ�q
        if (enemy.name == "Exile") {
            SetColor(13);
            cout << "Exile ���s�b���b�I�k�A�����I\n";
            player.bonusMaxHp -= 5;
            player.bonusMaxMp -= 5;
            enemy.exileGauge += 10;
            player.calculateTotalStats();
            cout << "�A���̤jHP�M�̤jMP�ä[��֤F 5 �I�I (��v�p�q��: " << enemy.exileGauge << ")\n";
            SetColor();
        }
        if (enemy.name == "�_��") {
            enemy.lawbreakerAbility = getRandomNumber(1, 4);
            SetColor(13);
            cout << "�_�߭��s�F���^�X���W�h�I\n";
            switch(enemy.lawbreakerAbility) {
                case 1: cout << "�W�h�G�A�L�k��o���󪬺A�I\n"; break;
                case 2: cout << "�W�h�G�A�L�k��_HP�I\n"; break;
                case 3: cout << "�W�h�G�A�L�k��_MP�I\n"; break;
                case 4: cout << "�W�h�G�A�L�k�ϥΧ����ޯ�I\n"; break;
            }
            SetColor();
        }
        if (enemy.name == "��Ū�") {
            SetColor(13);
            if (enemy.temperatureValue >= 80) {
                cout << "����Ĳ�o�F��Ū̪��l�ߡI���i�J�F���㪬�A�I�A�L�k�ϥΧ����ޯ�I\n";
                enemy.addStatus(RAGE, 2, 20);
            } else if (enemy.temperatureValue <= 20) {
                cout << "�C�ŭᵲ�F�A���N�ӡI�A�i�J�F�H�N���A�I�A�L�k�ϥΪv���αj�Ƨޯ�I\n";
                player.addStatus(FROST, 2, 15);
            }
            cout << "�ثe�ū׭�: " << enemy.temperatureValue << endl;
            SetColor();
        }


        if (player.hp > player.maxHp) player.hp = player.maxHp;
		if (player.mp > player.maxMp) player.mp = player.maxMp;
        player.printStatus();
        cout << enemy.name << " HP: " << enemy.hp;

        if (!enemy.activeStatuses.empty()) {
            cout << " (���A:";
            for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                Status status = enemy.activeStatuses[i];
                string statusName;
                switch (status.type) {
                    case BURN: statusName = "�`��"; break;
                    case POISON: statusName = "���r"; break;
                    case STUN: statusName = "�w�t"; break;
                    case BLEED: statusName = "�X��"; break;
                    case WEAK: statusName = "��z"; break;
                    case FROST: statusName = "�H�N"; break;
                    case RAGE: statusName = "����"; break;
                    default: statusName = "�������A"; break;
                }
                cout << " " << statusName << "[" << status.duration << "]";
            }
            cout << ")";
        }
        cout << endl;

        // NEW: �]�O����t�� (Mana Control System)
        float manaPercentage = (player.maxMp > 0) ? ((float)player.mp / player.maxMp * 100.0f) : 0;
        float manaDamageModifier = 1.0f;
        int manaHitRateModifier = 0;

        if (manaPercentage >= 85.0f) {
            manaDamageModifier = 1.2f;
            SetColor(10);
            cout << "�A���]�O�����A�����O���ɤF�I (�ˮ` +20%)\n";
            SetColor();
        } else if (manaPercentage < 50.0f && manaPercentage >= 30.0f) {
            manaDamageModifier = 0.9f;
            SetColor(12);
            cout << "�A���]�O���Ǥ�í�A�����ܱo��z�C (�ˮ` -10%)\n";
            SetColor();
        } else if (manaPercentage < 30.0f && manaPercentage >= 10.0f) {
            manaDamageModifier = 0.7f;
            manaHitRateModifier = -10;
            SetColor(12);
            cout << "�]�O�Y�������A�A�������ܱo�n�z�L�O�C (�ˮ` -30%, �R���v -10%)\n";
            SetColor();
        } else if (manaPercentage < 10.0f && manaPercentage >= 5.0f) {
            manaDamageModifier = 0.5f;
            manaHitRateModifier = -30;
            SetColor(12);
            cout << "�]�O�x�{�\�ܡA�A�X�G�L�k�I�k�I (�ˮ` -50%, �R���v -30%)\n";
            SetColor();
        } else if (manaPercentage < 5.0f) {
            SetColor(4);
            cout << "�]�O�����\�ܡA�A����F�j�P���Ͼ��A�w�t�F�@�^�X�I\n";
            SetColor();
            player.addStatus(STUN, 2, 0); // �w�t�ĪG�|�b���^�X������-1�A�ҥH�]��2�~��w�t�U�@��Ӧ^�X
        }
        
        vector<int> playerExpiredStatuses;
        for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
            if (enemy.name == "�Jù�մ�") {
                 player.activeStatuses[i].duration -= 2; // �ɶ��[�t
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
                cout << "�A�]�`�˨���F " << burnDamage << " �I�ˮ`�I\n";
            } else if (player.activeStatuses[i].type == POISON) {
                int poisonDamageBonus = 0;
                for (size_t k = 0; k < player.staff.effects.size(); ++k) {
                    if (player.staff.effects[k].type == POISON_DAMAGE_BONUS) {
                        poisonDamageBonus += player.staff.effects[k].value;
                    }
                }
                int poisonDamage = player.activeStatuses[i].value + player.activeStatuses[i].turnCount -1 + poisonDamageBonus;
                player.hp -= poisonDamage;
                cout << "�A�]���r����F " << poisonDamage << " �I�ˮ`�I\n";
            } else if (player.activeStatuses[i].type == HUNGER) {
                player.mp -= player.activeStatuses[i].value;
                cout << "�A�]���j���ӤF " << player.activeStatuses[i].value << " �I�]�O�I\n";
            } else if (player.activeStatuses[i].type == MEDITATION) {
                player.hp += 5;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                player.mp += 5;
                if (player.mp > player.maxMp) player.mp = player.maxMp;
                SetColor(11);
                cout << "�A�]�߷Q�^�_�F 5 �I HP �M MP�C\n";
                SetColor();
            }
            if (player.activeStatuses[i].duration <= 0) {
                // ���A�Y�N�L���A��ܴ���
                SetColor(8);
                cout << "�A�����A�u" << player.activeStatuses[i].type << "�v�w�����C\n";
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
                cout << "�A���Y���b�ƤF�A���W���Ҧ��t�����A�I\n";
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
                cout << enemy.name << " �]�`�˨���F " << enemy.activeStatuses[i].value << " �I�ˮ`�I\n";
            } else if (enemy.activeStatuses[i].type == POISON) {
                int poisonDamage = enemy.activeStatuses[i].value + enemy.activeStatuses[i].turnCount -1;
                enemy.hp -= poisonDamage;
                cout << enemy.name << " �]���r����F " << poisonDamage << " �I�ˮ`�I\n";
            }
            if (enemy.activeStatuses[i].duration <= 0) {
                // ���A�Y�N�L���A��ܴ���
                SetColor(8);
                cout << "�ĤH�u" << enemy.name << "�v�����A�u" << enemy.activeStatuses[i].type << "�v�w�����C\n";
                SetColor();
                enemyExpiredStatuses.push_back(i);
            }
        }
        for (int i = enemyExpiredStatuses.size() - 1; i >= 0; --i) {
            enemy.activeStatuses.erase(enemy.activeStatuses.begin() + enemyExpiredStatuses[i]);
        }
        
        if (player.hp <= 0) {
            cout << "�A�]���A�ĪG�Q���ѤF�I\n";
            return;
        }
        if (enemy.hp <= 0) {
            cout << enemy.name << " �]���A�ĪG�Q���ѤF�I\n";
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
            cout << "\n��ܧA�����:\n";
            for (size_t i = 0; i < player.skills.size(); ++i) {
                Skill currentSkill = player.skills[i];
                cout << i + 1 << ". �u" << currentSkill.name << "�v (MP����: " << currentSkill.mpCost << ")";
                if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
                    cout << ", �ˮ`: " << currentSkill.baseDamage << ", �R��: " << currentSkill.hitRate << "%";
                } else if (currentSkill.type == HEAL) {
                    cout << ", �v¡�q: " << currentSkill.value;
                }
                if (currentSkill.statusToInflict != NONE) {
                    string statusName;
                    switch (currentSkill.statusToInflict) {
                        case BURN: statusName = "�`��"; break;
                        case POISON: statusName = "���r"; break;
                        case STUN: statusName = "�w�t"; break;
                        case BLEED: statusName = "�X��"; break;
                        default: statusName = "�������A"; break;
                    }
                    cout << " (�I�[: " << statusName << ")";
                }
                cout << endl;
            }

            cout << player.skills.size() + 1 << ". �� (��_MP)\n";
            cout << player.skills.size() + 2 << ". �d�ݯ�O��\n";

            int choice = 0;
            bool validChoice = false;

            while (!validChoice) {
                cout << "�п�J�A����� (1-" << player.skills.size() + 2 << "): ";
                cin >> choice;

                if (choice > 0 && choice <= player.skills.size()) {
                    const Skill& chosenSkill = player.skills[choice - 1];
                    
                    // Boss �W�h�ˬd
                    if (enemy.name == "�_��" && enemy.lawbreakerAbility == 4 && (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == BUFF_ATTACK)) {
                        cout << "�W�h�����\�A�ϥΧ����ޯ�I\n"; continue;
                    }
                    if (enemy.name == "��Ū�" && enemy.temperatureValue >= 80 && (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK || chosenSkill.type == BUFF_ATTACK)) {
                        cout << "�������A�L�k�I�i�����ޯ�I\n"; continue;
                    }
                    if (enemy.name == "��Ū�" && enemy.temperatureValue <= 20 && (chosenSkill.type == HEAL || chosenSkill.type == BUFF)) {
                        cout << "�C�����A�L�k�I�i�v���αj�Ƨޯ�I\n"; continue;
                    }

                    if (player.mp >= chosenSkill.mpCost) {
                        player.mp -= chosenSkill.mpCost;
                        validChoice = true;
                        
                        // ��Ū̷ū׭p��
                        if (enemy.name == "��Ū�") {
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
                                int finalHitRate = chosenSkill.hitRate + manaHitRateModifier; // �M���]�O����R���ץ�
                                for (size_t i = 0; i < player.activeStatuses.size(); ++i) {
                                    if (player.activeStatuses[i].type == CALM) {
                                        finalHitRate += 10;
                                    }
                                }
                                
                                if (getRandomNumber(1, 100) <= finalHitRate) {
                                    int damage = player.magicAttack + chosenSkill.baseDamage;
                                    damage = (int)((float)damage * manaDamageModifier); // �M���]�O����ˮ`�ץ�

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
                                                cout << "�A���k���]�A���W�����A�ӵo�X�F���~�A�����O�W�[�F�I\n";
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

                                    if (chosenSkill.name == "�_������") {
                                        damage += (player.maxHp - player.hp);
                                    }
                                    
                                    enemy.hp -= damage;
                                    enemy.damageTakenThisTurn += damage;
                                    SetColor(4);
                                    cout << "�A�Ρu" << chosenSkill.name << "�v�y���F " << damage << " �I�ˮ`!\n";
                                    SetColor();

                                    for (size_t i = 0; i < enemy.activeStatuses.size(); ++i) {
                                        if (enemy.activeStatuses[i].type == BLEED) {
                                            int bleedDamage = enemy.activeStatuses[i].value;
                                            enemy.hp -= bleedDamage;
                                            if (enemy.hp < 0) enemy.hp = 0;
                                            SetColor(4);
                                            cout << "�ĤH�u" << enemy.name << "�v�]�X���B�~����F " << bleedDamage << " �I�ˮ`�I\n";
                                            SetColor();
                                        }
                                    }

                                    if (chosenSkill.statusToInflict != NONE) {
                                        bool canApplyStatus = true;
                                        if (enemy.name == "�_��" && enemy.lawbreakerAbility == 1) {
                                            canApplyStatus = false;
                                            cout << "�W�h�����\�A�I�[���A�I\n";
                                        }
                                        if (enemy.name == "�_��" && chosenSkill.name == "���ɺ�") {
                                            canApplyStatus = false;
                                            cout << "���ɺ��L�ĤƤF�A�����A�ĪG�I\n";
                                        }
                                        
                                        if (canApplyStatus) {
                                            int finalDuration = chosenSkill.statusDuration;
                                            for (size_t i = 0; i < player.staff.effects.size(); ++i) {
                                                if (player.staff.effects[i].type == STATUS_DURATION_BONUS) {
                                                    finalDuration += player.staff.effects[i].value;
                                                    cout << "(�k�����ѤF�B�~ +" << player.staff.effects[i].value << " �^�X����ɶ�)\n";
                                                    break;
                                                }
                                            }
                                            if (chosenSkill.type == DEBUFF_ATTACK) {
                                                enemy.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                                cout << "�ĤH�u" << enemy.name << "�v��o�F���A�G�u" << chosenSkill.statusToInflict << "�v\n";
                                            } else if (chosenSkill.type == BUFF_ATTACK) {
                                                player.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                                cout << "�A��o�F���A�G�u" << chosenSkill.statusToInflict << "�v\n";
                                            }
                                        }
                                    }
                                } else {
                                    SetColor(8);
                                    cout << "�A�Ρu" << chosenSkill.name << "�v���O���ѤF!\n";
                                    SetColor();
                                }
                                break;
                            }
                            case HEAL: {
                                int healAmount = chosenSkill.value;
                                if (enemy.name == "�_��" && enemy.lawbreakerAbility == 2) {
                                    cout << "�W�h�����\�A��_HP�I\n";
                                } else {
                                    player.hp += healAmount;
                                    if (player.hp > player.maxHp) player.hp = player.maxHp;
                                    SetColor(2);
                                    cout << "�A�ϥΡu" << chosenSkill.name << "�v�A��_�F " << healAmount << " �I HP�C\n";
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
                                            cout << "�A���k�����ѤF�B�~ +" << player.staff.effects[i].value << " �^�X����ɶ��I\n";
                                            SetColor();
                                            break;
                                        }
                                    }
                                    player.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                    cout << "�A��o�F���A�G�u" << chosenSkill.name << "�v\n";
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
                                            cout << "�A���k�����ѤF�B�~ +" << player.staff.effects[i].value << " �^�X����ɶ��I\n";
                                            SetColor();
                                            break;
                                        }
                                    }
                                    enemy.addStatus(chosenSkill.statusToInflict, finalDuration, chosenSkill.statusValue);
                                    cout << "�ĤH�u" << enemy.name << "�v��o�F���A�G�u" << chosenSkill.name << "�v\n";
                                }
                                break;
                            }
                            case UTILITY: {
                                if (chosenSkill.name == "�b��") {
                                    player.activeStatuses.clear();
                                    SetColor(10);
                                    cout << "�A�ϥΡu" << chosenSkill.name << "�v�A�M���F�ۨ��Ҧ����A�I\n";
                                    SetColor();
                                }
                                break;
                            }
                        }
                    } else {
                        cout << "�]�O�����C�Э��s��ܡC\n";
                    }
                } else if (choice == player.skills.size() + 1) {
                    validChoice = true;
                    if (enemy.name == "��Ū�") enemy.temperatureValue -= 10;
                    
                    bool canRegenMp = true;
                    if (enemy.name == "�_��" && enemy.lawbreakerAbility == 3) {
                         cout << "�W�h�����\�A��_MP�I\n";
                         canRegenMp = false;
                    }
                    if(canRegenMp){
                        player.mp += player.mpRegen + 5;
                        if (player.mp > player.maxMp) player.mp = player.maxMp;
                        SetColor(10);
                        cout << "�A��ܥ𮧤@�^�X�A�Pı�]�O��_�F�@�ǡC\n";
                        SetColor();
                    }
                } else if (choice == player.skills.size() + 2) {
                    player.showStatus();
                } else {
                    cout << "�L�Ī���ܡA�Э��s��J�C\n";
                }
            }
        } else {
            cout << "�A�]�w�t�L�k��ʡC\n";
        }

        if (enemy.hp <= 0) {
            SetColor(14);
            cout << enemy.name << " �Q���ѤF!\n";
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

            cout << enemy.name << " �ǳƨϥΡu" << chosenSkill.name << "�v!\n";

            // Boss �S��ޯ�B�z
            if (enemy.name == "�Jù�մ�") {
                if (chosenSkill.name == "�I�h") {
                    player.hp -= chosenSkill.baseDamage;
                    cout << "�I�h���O�L���F�A�����m�A�y���F " << chosenSkill.baseDamage << " �I�ˮ`�I\n";
                } else if (chosenSkill.name == "���ध��") {
                    if (enemy.hp < player.hp) {
                        cout << "�Jù�մ�����F�ɶ������A�洫�F���誺�ͩR�O�I\n";
                        int tempHp = player.hp;
                        player.hp = enemy.hp;
                        enemy.hp = tempHp;
                        if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                    } else {
                         cout << "�Jù�մ��չϭ���ɶ��A�����ѤF�C\n";
                    }
                } else if (chosenSkill.name == "�ĤQ�T��") {
                     enemy.hp += 50;
                     if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                     cout << "�Jù�մ��i�J�ɶ������ءA�^�_�F 50 �I�ͩR�I\n";
                } else { // ���, �ɶ�����
                    player.takeDamage(chosenSkill.baseDamage + enemy.magicAttack, chosenSkill.damageType, enemy);
                    player.addStatus(chosenSkill.statusToInflict, chosenSkill.statusDuration, chosenSkill.statusValue);
                     if(chosenSkill.name == "�ɶ�����") enemy.addStatus(RAGE, 10, 20);
                }
            } else if (enemy.name == "��Ū�") {
                player.takeDamage(chosenSkill.baseDamage + enemy.magicAttack, chosenSkill.damageType, enemy);
                if (chosenSkill.name == "�l���z�}") {
                    enemy.temperatureValue += 20;
                    player.addStatus(BURN, 5, 10);
                } else if (chosenSkill.name == "�B���j��") {
                    enemy.temperatureValue -= 20;
                    player.addStatus(FROST, 3, 15);
                    player.addStatus(BLEED, 3, 5);
                } else if (chosenSkill.name == "�����@��") {
                    enemy.temperatureValue = 50;
                    player.activeStatuses.clear();
                    enemy.activeStatuses.clear();
                    cout << "�����~�b�F�Գ��W���@�����A�I\n";
                } else if (chosenSkill.name == "�����Y��") {
                    player.mp -= 12;
                    if(player.mp < 0) player.mp = 0;
                    cout << "�A���]�O�Q��ܤF�I\n";
                }
            } else if (enemy.name == "�_��") {
                 if (chosenSkill.name == "�ʧ@����") {
                    player.takeDamage(chosenSkill.baseDamage, PHYSICAL, enemy);
                    enemy.addStatus(GUARD, 2, 30);
                 } else if (chosenSkill.name == "��������") {
                    player.addStatus(HUNGER, 5, 5);
                 } else if (chosenSkill.name == "���ɺ�") {
                    player.takeDamage(chosenSkill.baseDamage, PHYSICAL, enemy);
                    enemy.activeStatuses.clear();
                 } else if (chosenSkill.name == "�W�h���s") {
                    cout << "�W�h�Q���s�A�A����F�P�A�y���ˮ`���q���Ͼ��I\n";
                    player.takeDamage(enemy.damageTakenThisTurn, PHYSICAL, enemy);
                 }
            } else if (enemy.name == "Exile") {
                if (chosenSkill.name == "��L���") {
                    enemy.addStatus(GUARD, 5, 30);
                    enemy.exileGauge += 30;
                } else if (chosenSkill.name == "��v��ĳ") {
                    cout << "Exile ����ĳ�ͮġA���^�_�F���^�X���쪺�Ҧ��ˮ`�I\n";
                    enemy.hp += enemy.damageTakenThisTurn;
                    if (enemy.hp > enemy.maxHp) enemy.hp = enemy.maxHp;
                    enemy.exileGauge += enemy.damageTakenThisTurn;
                } else if (chosenSkill.name == "�{��ᦱ") {
                    cout << "�{��Q�ᦱ�A�A���Ҧ����A���Q�ಾ���F Exile�I\n";
                    enemy.activeStatuses.insert(enemy.activeStatuses.end(), player.activeStatuses.begin(), player.activeStatuses.end());
                    player.activeStatuses.clear();
                    enemy.exileGauge += 30;
                } else if (chosenSkill.name == "��ѯ߽�") {
                    player.takeDamage(chosenSkill.baseDamage, MAGICAL, enemy);
                    player.mp -= 20;
                    if (player.mp < 0) player.mp = 0;
                    enemy.exileGauge += chosenSkill.baseDamage;
                } else if (chosenSkill.name == "�̲׼f�P") {
                    cout << "�̲׼f�P���{�I�A����F " << enemy.exileGauge << " �I�L�k���m���ˮ`�I\n";
                    player.hp -= enemy.exileGauge;
                    enemy.exileGauge = 0;
                }
                cout << "(��v�p�q��: " << enemy.exileGauge << ")\n";
            }
            // ���q�ĤH�����޿�
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
                            cout << "�A��o�F���A�G�u" << chosenSkill.statusToInflict << "�v\n";
                        }
                        else if (chosenSkill.type == BUFF) {
                            enemy.addStatus(chosenSkill.statusToInflict, chosenSkill.statusDuration, chosenSkill.statusValue);
                            cout << "�ĤH�u" << enemy.name << "�v��ۤv�I�[�F���A�G�u" << chosenSkill.name << "�v\n";
                        }
                    }
                    
                    if (chosenSkill.type == ATTACK || chosenSkill.type == DEBUFF_ATTACK) {
                        player.takeDamage(damage, chosenSkill.damageType, enemy);
                    } else if (chosenSkill.type == DEBUFF || chosenSkill.type == BUFF) {
                        cout << enemy.name << " �ϥΤF�u" << chosenSkill.name << "�v�A�S�������y���ˮ`�C\n";
                        
                    }
                } else {
                    SetColor(8);
                    cout << enemy.name << " ���u" << chosenSkill.name << "�v���ѤF!\n";
                    SetColor();
                }
            }
        } else {
            cout << "�ĤH�u" << enemy.name << "�v�]�w�t�L�k��ʡC\n";
        }
        
        if (player.hp <= 0) {
            return;
        }
        
        bool canRegenHp = true, canRegenMp = true;
        if (enemy.name == "�_��") {
            if (enemy.lawbreakerAbility == 2) canRegenHp = false;
            if (enemy.lawbreakerAbility == 3) canRegenMp = false;
        }
        player.regenerate(canRegenHp, canRegenMp);
        enemy.regenerate();
        cout << endl;
    }
}

// ��l�ƩҦ��ޯ઺�禡
void initializeSkills() {
    PLAYER_SKILLS.push_back(Skill("���q����", ATTACK, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�]�k�u", ATTACK, 5, 10, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�]�k����", ATTACK, 5, 15, 0, 85, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�]�k�u+", ATTACK, 8, 20, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�]�k�j��", ATTACK, 15, 50, 0, 70, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("����", ATTACK, 10, 25, 0, 95, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("���y�N", DEBUFF_ATTACK, 10, 20, 0, 90, MAGICAL, BURN, 3, 10));
    PLAYER_SKILLS.push_back(Skill("����", DEBUFF_ATTACK, 5, 5, 0, 85, MAGICAL, BURN, 5, 10));
    PLAYER_SKILLS.push_back(Skill("�Һ�", DEBUFF_ATTACK, 30, 65, 0, 50, MAGICAL, BURN, 10, 10));
    PLAYER_SKILLS.push_back(Skill("����ľW", BUFF_ATTACK, 10, 40, 0, 90, MAGICAL, BURN, 3, 10));
    PLAYER_SKILLS.push_back(Skill("�B�b�N", DEBUFF_ATTACK, 8, 15, 0, 95, MAGICAL, FROST, 2, 15));
    PLAYER_SKILLS.push_back(Skill("�{�q�N", DEBUFF_ATTACK, 15, 30, 0, 80, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("���p", DEBUFF_ATTACK, 25, 70, 0, 70, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("�q��", DEBUFF_ATTACK, 5, 5, 0, 90, MAGICAL, STUN, 1, 0));
    PLAYER_SKILLS.push_back(Skill("�_������", ATTACK, 15, 5, 0, 80, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�v���N", HEAL, 15, 0, 30, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�ֳt�v��", HEAL, 5, 0, 12, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�j�O�v���N", HEAL, 30, 0, 55, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�j�ƨ��m", BUFF, 10, 0, 0, 100, MAGICAL, GUARD, 2, 30));
    PLAYER_SKILLS.push_back(Skill("�O�q����", BUFF, 8, 0, 0, 100, MAGICAL, RAGE, 4, 20));
    PLAYER_SKILLS.push_back(Skill("�W�T����", BUFF_ATTACK, 10, 10, 0, 100, MAGICAL, RAGE, 2, 20));
    PLAYER_SKILLS.push_back(Skill("��ǥ���", BUFF, 5, 0, 0, 100, MAGICAL, CALM, 4, 10));
    PLAYER_SKILLS.push_back(Skill("�ۼv�B�k", BUFF, 6, 0, 0, 100, MAGICAL, EVADE_CHANCE, 1, 50));
    PLAYER_SKILLS.push_back(Skill("�b��", UTILITY, 15, 0, 0, 100, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�r�b", DEBUFF_ATTACK, 5, 5, 0, 100, MAGICAL, POISON, 3, 3));
    PLAYER_SKILLS.push_back(Skill("�īB", DEBUFF_ATTACK, 10, 10, 0, 90, MAGICAL, POISON, 5, 3));
    PLAYER_SKILLS.push_back(Skill("�@�r����", DEBUFF_ATTACK, 25, 20, 0, 95, MAGICAL, POISON, 6, 5));
    PLAYER_SKILLS.push_back(Skill("�ɭ���", ATTACK, 25, 50, 0, 70, MAGICAL, FROST, 3, 15));
    PLAYER_SKILLS.push_back(Skill("���t����", ATTACK, 20, 40, 0, 85, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("�����i", ATTACK, 10, 30, 0, 80, MAGICAL, NONE, 0, 0));
    PLAYER_SKILLS.push_back(Skill("��z�A�G", DEBUFF, 10, 0, 0, 100, MAGICAL, WEAK, 3, 20));
    PLAYER_SKILLS.push_back(Skill("�X�����", DEBUFF_ATTACK, 15, 15, 0, 90, MAGICAL, BLEED, 3, 5));
    PLAYER_SKILLS.push_back(Skill("�߷Q", BUFF, 15, 0, 0, 100, MAGICAL, MEDITATION, 5, 0));

    // ���q�ĤH�ޯ�
    ENEMY_SKILLS.push_back(Skill("����", ATTACK, 0, 10, 0, 95, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�]�k�u", ATTACK, 0, 8, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("����", ATTACK, 0, 20, 0, 70, PHYSICAL, STUN, 1, 0));
    ENEMY_SKILLS.push_back(Skill("�r�G�Q�g", DEBUFF_ATTACK, 0, 5, 0, 85, PHYSICAL, POISON, 3, 5));
    ENEMY_SKILLS.push_back(Skill("�����¶", DEBUFF, 0, 0, 0, 80, PHYSICAL, STUN, 2, 0));
    ENEMY_SKILLS.push_back(Skill("��z�A�G", DEBUFF, 0, 0, 0, 90, MAGICAL, WEAK, 3, 30));
    ENEMY_SKILLS.push_back(Skill("�{�q�N", ATTACK, 0, 30, 0, 80, MAGICAL, STUN, 1, 0));
    ENEMY_SKILLS.push_back(Skill("����", DEBUFF_ATTACK, 0, 8, 0, 80, MAGICAL, BURN, 4, 8));
    ENEMY_SKILLS.push_back(Skill("�U�l����", ATTACK, 0, 30, 0, 90, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�n���H�S", BUFF, 0, 0, 0, 100, MAGICAL, RAGE, 3, 15));
    // Boss �ޯ�
    ENEMY_SKILLS.push_back(Skill("���", DEBUFF_ATTACK, 0, 25, 0, 100, MAGICAL, WEAK, 10, 20));
    ENEMY_SKILLS.push_back(Skill("�ĤQ�T��", HEAL, 0, 0, 50, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�ɶ�����", BUFF_ATTACK, 0, 40, 0, 100, PHYSICAL, RAGE, 10, 20));
    ENEMY_SKILLS.push_back(Skill("�I�h", ATTACK, 0, 30, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("���ध��", UTILITY, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�l���z�}", DEBUFF_ATTACK, 0, 50, 0, 80, MAGICAL, BURN, 5, 10));
    ENEMY_SKILLS.push_back(Skill("�B���j��", DEBUFF_ATTACK, 0, 40, 0, 90, PHYSICAL, FROST, 3, 15));
    ENEMY_SKILLS.push_back(Skill("�����@��", ATTACK, 0, 45, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�����Y��", ATTACK, 0, 50, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�ʧ@����", ATTACK, 0, 60, 0, 60, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("��������", DEBUFF, 0, 0, 0, 100, MAGICAL, HUNGER, 5, 5));
    ENEMY_SKILLS.push_back(Skill("���ɺ�", ATTACK, 0, 20, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�W�h���s", ATTACK, 0, 0, 0, 100, PHYSICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("��L���", BUFF, 0, 0, 0, 100, MAGICAL, GUARD, 5, 30));
    ENEMY_SKILLS.push_back(Skill("��v��ĳ", HEAL, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�{��ᦱ", UTILITY, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("��ѯ߽�", ATTACK, 0, 50, 0, 100, MAGICAL, NONE, 0, 0));
    ENEMY_SKILLS.push_back(Skill("�̲׼f�P", ATTACK, 0, 0, 0, 100, MAGICAL, NONE, 0, 0));

    // �s�W��h�˳ƼҪO
    ALL_EQUIPMENT_TEMPLATES.push_back({"��s�Y��", HELMET, M_DEFENSE_BONUS, 5, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�K��", HELMET, DEFENSE_BONUS, 8, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"���F�Y�a", HELMET, MP_BONUS, 10, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"���t����", HELMET, HP_REGEN_BONUS, 3, 1, 3});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�֥�", ARMOR, DEFENSE_BONUS, 10, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"��l��", ARMOR, DEFENSE_BONUS, 15, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�s���Z��", ARMOR, M_DEFENSE_BONUS, 12, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�k�v���T", ARMOR, MP_BONUS, 15, 3, 6});
    ALL_EQUIPMENT_TEMPLATES.push_back({"���k��", STAFF, M_ATTACK_BONUS, 8, 1, 4});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�����k��", STAFF, M_ATTACK_BONUS, 12, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"���K�k��", STAFF, BURN_DAMAGE_BONUS, 5, 2, 5});
    ALL_EQUIPMENT_TEMPLATES.push_back({"�H�B�k��", STAFF, STATUS_DURATION_BONUS, 1, 1, 2});
    
    // ��l�ƩҦ��ĪG�����C��
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

// NEW: �B�z�Ӽh�����ɪ��H���ƥ�
void handleFloorEvents(Player& player) {
    int eventRoll = getRandomNumber(1, 100);
    char choice;
    int roll;

    // �C�Өƥ� 2% �����v
    if (eventRoll <= 2) { // �ƥ� 1: �������Ĥ�
        SetColor(11);
        cout << "\n�A�o�{�F�@�~�_�ۮ�w�������Ĥ��A�n�ܤU���ܡH (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            roll = getRandomNumber(1, 100);
            if (roll <= 60) {
                player.hp += 20;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                SetColor(10);
                cout << "�A�P��믫�@���A��_�F 20 �I�ͩR�I\n";
            } else {
                player.hp -= 10;
                if (player.hp < 0) player.hp = 0;
                player.addStatus(POISON, 2, 5);
                SetColor(12);
                cout << "�Ĥ��|�_�ӫ����ߡI�A���h�F 10 �I�ͩR�ä��r�F�I\n";
            }
        } else {
            cout << "�A�p���l�l�a���Ĥ���^�F��B�C\n";
        }

    } else if (eventRoll <= 4) { // �ƥ� 2: ����
        SetColor(12);
        cout << "\n�A���F�@�����ê������I\n";
        roll = getRandomNumber(1, 100);
        if (roll <= 30) {
            cout << "�a�O�Q�X���K�A�A����F 3 �^�X���`�ˡI\n";
            player.addStatus(BURN, 3, 5);
        } else if (roll <= 60) {
            cout << "�y��q����g�X�A�A����F 20 �I�ˮ`�I\n";
            player.hp -= 20;
            if (player.hp < 0) player.hp = 0;
        } else if (roll <= 90) {
            cout << "�A�l�J�F�r��A����F 4 �^�X�����r�I\n";
            player.addStatus(POISON, 4, 5);
        } else {
            SetColor(10);
            cout << "�b�d�v�@�v���ڡA�A���I�a���L�F�����I�A�]����o�F 20 �I�g��ȡC\n";
            player.gainExp(20);
        }

    } else if (eventRoll <= 6) { // �ƥ� 3: ���������y
        int expGained = getRandomNumber(10, 100);
        SetColor(10);
        cout << "\n�A���F�@�Ǵ��������y�A�\Ū����]�k���z�ѧ�`�F�C\n";
        player.gainExp(expGained);

    } else if (eventRoll <= 8) { // �ƥ� 4: ����
        SetColor(11);
        cout << "\n�A���F�@�Ǭݰ_���ٯ�Y�������A�n�Y�����ܡH (y/n)\n";
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
                cout << "�A��_�F 30 �I�ͩR�M 15 �I�]�O�C\n";
            } else {
                player.addStatus(POISON, 5, 5);
                SetColor(12);
                cout << "�����w�g�G�a�F�I�A���r�F�I\n";
            }
        } else {
            cout << "�A�M�w���_�o���I�C\n";
        }

    } else if (eventRoll <= 10) { // �ƥ� 5: ���� (HP)
        SetColor(10);
        cout << "\n�@�D�ŷx�����~Ţ�n�ۧA�A�A�Pı�ۤv�ܱo��j���F�C\n";
        cout << "�̤j�ͩR�ȼW�[�F 10 �I�I\n";
        player.bonusMaxHp += 10;
        player.calculateTotalStats();

    } else if (eventRoll <= 12) { // �ƥ� 6: ���� (MP)
        SetColor(10);
        cout << "\n�@�ѲM�D����q�y�L�A������A�A������ܱo��M���F�C\n";
        cout << "�̤j�]�O�W�[�F 5 �I�I\n";
        player.bonusMaxMp += 5;
        player.calculateTotalStats();

    } else if (eventRoll <= 14) { // �ƥ� 7: �A�G
        SetColor(12);
        cout << "\n�A�Pı��@�Ѩ��c���O�q��W�F�A�A���A�P���z�C\n";
        cout << "�A����F 3 �^�X����z���A�I\n";
        player.addStatus(WEAK, 3, 20);

    } else if (eventRoll <= 16) { // �ƥ� 8: �����߳�
        SetColor(11);
        cout << "\n�A���i�F�@�ӯ������߳��A�Pı���q�b�ԧ�A������C\n";
        cout << "1. �j�Ʀ���A�d�z�믫 (+5 ���m, -5 �]��)\n";
        cout << "2. �j�ƺ믫�A�d�z���� (+5 �]��, -5 ���m)\n";
        cout << "3. ���}�߳�\n";
        cout << "�п�J�A����� (1-3): ";
        SetColor();
        int fieldChoice;
        cin >> fieldChoice;
        if (fieldChoice == 1) {
            cout << "�A��ܤF�j�Ʀ���C\n";
            player.bonusDefense += 5;
            player.bonusMagicDefense -= 5;
            player.calculateTotalStats();
        } else if (fieldChoice == 2) {
            cout << "�A��ܤF�j�ƺ믫�C\n";
            player.bonusMagicDefense += 5;
            player.bonusDefense -= 5;
            player.calculateTotalStats();
        } else {
            cout << "�A������}�F�o�ө_�Ǫ��a��C\n";
        }

    } else if (eventRoll <= 18) { // �ƥ� 9: �_�Ǫ�����
        SetColor(11);
        cout << "\n�A�o�{�F�@�ө_�Ǫ����¡A�W�����G�i�H�m�W���~�C�n���� 10 �I�ͩR�Ӭ�ë�ܡH (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            player.hp -= 10;
            if (player.hp < 0) player.hp = 0;
            cout << "�A�N��G�w�b���¤W...\n";
            roll = getRandomNumber(1, 100);
            if (roll <= 30) {
                SetColor(10);
                cout << "���µo�X���~�A�A��o�F 50 �I�g��ȡI\n";
                player.gainExp(50);
            } else if (roll <= 50) {
                cout << "���¨S����������C\n";
            } else if (roll <= 90) {
                SetColor(10);
                cout << "�A�Pı���]�O�^�_���t���ܧ֤F�I (MP�^�_ +1)\n";
                player.mpRegen += 1;
            } else {
                SetColor(10);
                cout << "�A�Pı��ˤf¡�X���t���ܧ֤F�I (HP�^�_ +2)\n";
                player.hpRegen += 2;
            }
        } else {
            cout << "�A��o�ӥ��������«O���q�ȡA��ܤF���}�C\n";
        }

    } else if (eventRoll <= 20) { // �ƥ� 10: �]�k�}
        int magicGained = getRandomNumber(5, 10);
        SetColor(10);
        cout << "\n�A�b�a�W�o�{�@�Ӵݯ}���]�k�}�A�״_������A�A�q���Ǩ�F�@�Ǫ��ѡC\n";
        cout << "�]�k�����O�W�[�F " << magicGained << " �I�I\n";
        player.bonusMagicAttack += magicGained;
        player.calculateTotalStats();

    } else if (eventRoll <= 22) { // �ƥ� 11: �g��
        SetColor(12);
        cout << "\n�A�b�·t���g���F��V�A¶�F�ܤ[�~���X���A�Pı�{�l�ܾj�C\n";
        cout << "�A��o�F 5 �^�X�����j���A�I\n";
        player.addStatus(HUNGER, 5, 2);

    } else if (eventRoll <= 24) { // �ƥ� 12: �]ۣ
        SetColor(11);
        cout << "\n��W���ۤ@���_�Ǫ��]ۣ�A�ݰ_�ӫܻ��H�C�n�Y�����ܡH (y/n)\n";
        SetColor();
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            roll = getRandomNumber(1, 100);
            if (roll <= 20) {
                SetColor(12);
                cout << "�]ۣ���r�I�A���r�F 5 �^�X�I\n";
                player.addStatus(POISON, 5, 5);
            } else if (roll <= 50) {
                SetColor(13);
                cout << "�Y�U�]ۣ��A�P��@�ѵL�W���A�i�J�F 3 �^�X�����㪬�A�I\n";
                player.addStatus(RAGE, 3, 20);
            } else if (roll <= 70) {
                SetColor(10);
                cout << "�]ۣ�N�~�a�����A�A��_�F 20 �I�ͩR�C\n";
                player.hp += 20;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
            } else if (roll <= 90) {
                SetColor(12);
                cout << "�o�]ۣ�S������D�A���A���{�l�}�l�s�F�C�A�i�J�F 3 �^�X�����j���A�C\n";
                player.addStatus(HUNGER, 3, 2);
            } else {
                SetColor(10);
                cout << "�A�q�]ۣ���U�l����o�F 5 �I�g��ȡC\n";
                player.gainExp(5);
            }
        } else {
            cout << "�A�M�w���æY�F��C\n";
        }

    } else if (eventRoll <= 26) { // �ƥ� 13: ���e
        SetColor(9);
        cout << "\n�A�ݨ���䦳�@��_�I�̪����e�A���A��ͩR���F�s���鮩�C\n";
        cout << "�A��o�F 4 �^�X���N�R���A�C\n";
        player.addStatus(CALM, 4, 10);

    } else if (eventRoll <= 28) { // �ƥ� 14: �_�c
        SetColor(6);
        cout << "\n�A�o�{�F�@���_�c�I\n";
        roll = getRandomNumber(1, 100);
        if (roll <= 60) {
            cout << "�_�c�̦��@��˳ơI\n";
            int equipTypeChoice = getRandomNumber(1, 3);
            EquipmentType type;
            if (equipTypeChoice == 1) type = HELMET;
            else if (equipTypeChoice == 2) type = ARMOR;
            else type = STAFF;
            Equipment newEquip = generateRandomEquipment(type);
            player.equipItem(newEquip);
        } else if (roll <= 90) {
            cout << "�_�c�̦��@�ǭ����A�A��_�F 15 �I�ͩR�C\n";
            player.hp += 15;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
        } else {
            SetColor(12);
            cout << "�_�c��M�i�}�j�L�r�F�A�@�f�I��ӬO�_�c�ǡI\n";
            cout << "�A����F 30 �I�ˮ`�A�����ѥ�����o�F 20 �I�g��ȡC\n";
            player.hp -= 30;
            if (player.hp < 0) player.hp = 0;
            player.gainExp(20);
        }
    
    } else if (eventRoll <= 30) { // NEW: �ƥ� 15: ��ۯ�����ë
        SetColor(10);
        cout << "\n�A�o�{�@�L�j�Ѫ������A�A�@�ۦa��ë...\n";
        int statChoice = getRandomNumber(1, 5);
        switch (statChoice) {
            case 1:
                cout << "�A�Pı�����ܱo����F�I (�̤j�ͩR�����I�� +2)\n";
                player.bonusMaxHp += 2;
                break;
            case 2:
                cout << "�A�Pı�]�O��[�R�դF�I (�̤j�]�O�����I�� +2)\n";
                player.bonusMaxMp += 2;
                break;
            case 3:
                cout << "�A���]�k���⮩�[�`�F�I (�]�k���������I�� +2)\n";
                player.bonusMagicAttack += 2;
                break;
            case 4:
                cout << "�A�����m�ޥ����i�F�I (���z���m�����I�� +2)\n";
                player.bonusDefense += 2;
                break;
            case 5:
                cout << "�A�����F����F���@�I (�]�k���m�����I�� +2)\n";
                player.bonusMagicDefense += 2;
                break;
        }
        player.calculateTotalStats();

    } else if (eventRoll <= 50) { // �˳Ʊ��� (20% ���v, from 31 to 50)
        SetColor(14);
        cout << "\n�ĤH�����e�����G���{�{�o�����F��...\n";
        SetColor();
        int equipTypeChoice = getRandomNumber(1, 3);
        EquipmentType type;
        if (equipTypeChoice == 1) type = HELMET;
        else if (equipTypeChoice == 2) type = ARMOR;
        else type = STAFF;
        
        Equipment newEquip = generateRandomEquipment(type);
        player.equipItem(newEquip);
    } else {
        // ���򳣨S�o��
        cout << "\n�A�~��e�i�A�S���o�ͤ���S�O���ơC\n";
    }
    SetColor();
    system("pause");
}

int main() {
    srand(time(0));
    
    initializeSkills();

    Player player;
    // �ϥΫغc�l��l��
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
    cout << "           ��ܧA����l�ޯ�              \n";
    cout << "----------------------------------------\n";
    cout << "�A�w�g�Ƿ|�F�u" << player.skills[0].name << "�v�C\n";
    cout << "�бq�H�U�T�ӧޯत�A��ܤ@�ӧ@���A����l�ޯ�G\n";
    for (size_t i = 0; i < choiceSkills.size(); ++i) {
        Skill currentSkill = choiceSkills[i];
        cout << i + 1 << ". �u" << currentSkill.name << "�v (MP����: " << currentSkill.mpCost << ")";
        if (currentSkill.type == ATTACK || currentSkill.type == DEBUFF_ATTACK || currentSkill.type == BUFF_ATTACK) {
            cout << ", �ˮ`: " << currentSkill.baseDamage << ", �R��: " << currentSkill.hitRate << "%";
        } else if (currentSkill.type == HEAL) {
            cout << ", �v¡�q: " << currentSkill.value;
        }
        cout << endl;
    }
    cout << "----------------------------------------\n";
    SetColor();
    
    int choice = 0;
    bool validChoice = false;
    while (!validChoice) {
        cout << "�п�J�A����� (1-3): ";
        cin >> choice;
        if (choice > 0 && choice <= choiceSkills.size()) {
            player.skills.push_back(choiceSkills[choice - 1]);
            validChoice = true;
            SetColor(6);
            cout << "�A��ܤF�u" << player.skills.back().name << "�v�@���A����l�ޯ�I\n";
            SetColor();
            system("pause");
        } else {
            cout << "�L�Ī���ܡA�Э��s��J�C\n";
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
            enemy.skills.push_back(ENEMY_SKILLS[23]); // ��L���
            enemy.skills.push_back(ENEMY_SKILLS[24]); // ��v��ĳ
            enemy.skills.push_back(ENEMY_SKILLS[25]); // �{��ᦱ
            enemy.skills.push_back(ENEMY_SKILLS[26]); // ��ѯ߽�
            enemy.skills.push_back(ENEMY_SKILLS[27]); // �̲׼f�P
        } else if (floor == 40) {
            enemy.name = "�_��";
            enemy.hp = 500;
            enemy.maxHp = 500;
            enemy.expValue = 550;
            enemy.skills.push_back(ENEMY_SKILLS[19]); // �ʧ@����
            enemy.skills.push_back(ENEMY_SKILLS[20]); // ��������
            enemy.skills.push_back(ENEMY_SKILLS[21]); // ���ɺ�
            enemy.skills.push_back(ENEMY_SKILLS[22]); // �W�h���s
        } else if (floor == 30) {
            enemy.name = "��Ū�";
            enemy.hp = 400;
            enemy.maxHp = 400;
            enemy.expValue = 250;
            enemy.skills.push_back(ENEMY_SKILLS[15]); // �l���z�}
            enemy.skills.push_back(ENEMY_SKILLS[16]); // �B���j��
            enemy.skills.push_back(ENEMY_SKILLS[17]); // �����@��
            enemy.skills.push_back(ENEMY_SKILLS[18]); // �����Y��
        } else if (floor == 20) {
            enemy.name = "�Jù�մ�";
            enemy.hp = 300;
            enemy.maxHp = 300;
            enemy.expValue = 150;
            enemy.hpRegen = 10;
            enemy.skills.push_back(ENEMY_SKILLS[10]); // ���
            enemy.skills.push_back(ENEMY_SKILLS[11]); // �ĤQ�T��
            enemy.skills.push_back(ENEMY_SKILLS[12]); // �ɶ�����
            enemy.skills.push_back(ENEMY_SKILLS[13]); // �I�h
            enemy.skills.push_back(ENEMY_SKILLS[14]); // ���ध��
        } else {
            enemy = createEnemy(floor);
        }

        startBattle(player, enemy);

        if (player.hp <= 0) {
            cout << "\n�C�������C\n";
            break;
        }

        if (player.hp > 0 && floor < 50) {
            // �I�s�H���ƥ�禡
            handleFloorEvents(player);

            // �W�ߪ��ޯ�ǲ߾��|
            if (getRandomNumber(1, 100) > floor) {
                SetColor(6);
                cout << "\n�b�԰��L��A�A���]�O���x����[���m�F�A���G�i�H�Ƿ|�s���ޯ�I\n";
                
                vector<Skill> availableSkills;
                // ��X���a�|���Ƿ|���ޯ�
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
                    cout << "���A���G�w�g�Ƿ|�F�Ҧ��ޯ�C\n";
                    system("pause");
                } else {
                    random_shuffle(availableSkills.begin(), availableSkills.end());
                    vector<Skill> choiceSkills;
                    // ���ѳ̦h 3 �ӿﶵ
                    for (int i = 0; i < 3 && i < availableSkills.size(); ++i) {
                        choiceSkills.push_back(availableSkills[i]);
                    }

                    cout << "�бq�H�U�ޯत��ܤ@�Ӿǲ� (��J 0 ���)�G\n";
                    for (size_t i = 0; i < choiceSkills.size(); ++i) {
                        Skill currentSkill = choiceSkills[i];
                        cout << i + 1 << ". �u" << currentSkill.name << "�v (MP����: " << currentSkill.mpCost << ")\n";
                    }
                    int skillChoice;
                    cin >> skillChoice;
                    if (skillChoice > 0 && skillChoice <= choiceSkills.size()) {
                        player.learnSkill(choiceSkills[skillChoice - 1]);
                    } else {
                        cout << "�A�M�w�d�T�{�������ѡA�Ȯɤ��ǲ߷s�ޯ�C\n";
                        system("pause");
                    }
                }
            }
        }
    }

    if (player.hp > 0) {
        SetColor(10);
        cout << "\n���ߧA�I�A���\�a�q���F�a�c�I\n";
        SetColor();
    }

    return 0;
}
