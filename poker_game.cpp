#include<iostream>
#include<string>
#include<vector>
#include<random>
#include<algorithm>
#include<map>
#include "poker_game.h"
poker::poker(char shape,int value):shape(shape),value(value){
        //H:heart D:diamond C:clubs S:spades
        if(shape=='H'){shape_num = 3;}
        if(shape=='D'){shape_num = 2;}
        if(shape=='C'){shape_num = 1;}
        if(shape=='S'){shape_num = 4;}

};
int poker::getWeight()const{

    if (value == 1) return 12;
    if (value == 2) return 13; 
    return value - 3;   
};
bool poker::operator<(const poker& other) const {
    if (getWeight() != other.getWeight())
        return getWeight() < other.getWeight();
            
    return shape_num < other.shape_num;
};
player::player(int id):id(id),hasClub3(false),score(0){

};

void player::removeCards(const hand& playedHand) {
    for (const auto& targetCard : playedHand.cards) {
        // 使用 std::find_if 或直接 remove_if 找到特定花色與數值的牌並移除
        auto it = std::find_if(hand_pool.begin(), hand_pool.end(), [&](const poker& c) {
            return c.value == targetCard.value && c.shape == targetCard.shape;
        });
        if (it != hand_pool.end()) {
            hand_pool.erase(it);
        }
    }
};
hand auto_player::makeDecision(const hand& lastHand, bool isFirstTurn) {
    // 1. 取得所有合法的出牌組合
    std::vector<hand> options = game_roler::getLegalOptions(this->hand_pool, lastHand, isFirstTurn);

    if (options.empty()) return hand({}); 

    // 2. 簡單策略：主動出牌優先出五張牌型，被動出牌出最小的
    if (lastHand.type == HandType::INVALID) {
        for (const auto& h : options) {
            if (h.type >= HandType::STRAIGHT) return h;
        }
    }
    return options[0]; 
};
hand::hand(std::vector<poker> selectedCards) : cards(selectedCards), keyCard(selectedCards[0]) {
        std::sort(this->cards.begin(), this->cards.end());
        validate();
};
bool hand::canBeat(const hand& other) const {
        // 1. 如果對方是 INVALID，我方只要合法就能出
    if (other.type == HandType::INVALID) return this->type != HandType::INVALID;

    // 2. 如果牌型相同，直接比 keyCard
    if (this->type == other.type) {
        return other.keyCard < this->keyCard; 
    }

    // 3. 五張牌型之間的互壓 (順子 < 葫蘆 < 鐵支 < 同花順)
    // 只有當雙方都是五張牌型時才允許不同 type 互比
    if ((int)this->type >= (int)HandType::STRAIGHT && (int)other.type >= (int)HandType::STRAIGHT) {
        return (int)this->type > (int)other.type;
    }

    return false; // 類型不同且不是五張牌型互壓，不能出
};

static int big2Order(const poker& card) {
    int order = card.getWeight();
    if (card.value == 1) {
        order -= 1; // A needs to sit just below 2 for Big2 straight checks
    }
    return order;
}

static bool isStraight(const std::vector<poker>& cards) {
    if (cards.size() != 5) return false;

    std::vector<poker> sortedCards(cards);
    std::sort(sortedCards.begin(), sortedCards.end(), [](const poker& a, const poker& b) {
        return big2Order(a) < big2Order(b);
    });

    bool isLowA23 = (sortedCards[0].value == 3 && sortedCards[1].value == 4 && sortedCards[2].value == 5 && sortedCards[3].value == 1 && sortedCards[4].value == 2);
    if (isLowA23) {
        return true;
    }

    if (std::count_if(sortedCards.begin(), sortedCards.end(), [](const poker& c) { return c.value == 2; }) > 0) {
        return false;
    }

    for (int i = 1; i < 5; ++i) {
        if (big2Order(sortedCards[i]) != big2Order(sortedCards[i - 1]) + 1) {
            return false;
        }
    }

    return true;
}

static poker highestCard(const std::vector<poker>& cards) {
    return *std::max_element(cards.begin(), cards.end());
}

void hand::validate() {
    if (cards.empty()) {
        type = HandType::INVALID;
        return;
    }

    if (cards.size() == 1) {
        type = HandType::SINGLE;
        keyCard = cards[0];
        return;
    }

    if (cards.size() == 2) {
        if (cards[0].value == cards[1].value) {
            type = HandType::PAIR;
            keyCard = cards[1];
        } else {
            type = HandType::INVALID;
        }
        return;
    }

    if (cards.size() == 5) {
        bool flush = true;
        for (int i = 1; i < 5; ++i) {
            if (cards[i].shape != cards[0].shape) {
                flush = false;
                break;
            }
        }

        std::map<int, int> counts;
        for (const auto& card : cards) {
            counts[card.value]++;
        }

        bool straight = isStraight(cards);
        if (straight && flush) {
            type = HandType::STRAIGHT_FLUSH;
            keyCard = highestCard(cards);
            return;
        }

        if (straight) {
            type = HandType::STRAIGHT;
            keyCard = highestCard(cards);
            return;
        }

        if (counts.size() == 2) {
            auto iter = counts.begin();
            int firstCount = iter->second;
            int firstValue = iter->first;
            ++iter;
            int secondCount = iter->second;
            int secondValue = iter->first;

            if ((firstCount == 4 && secondCount == 1) || (firstCount == 1 && secondCount == 4)) {
                type = HandType::FOUR_KIND;
                int fourValue = firstCount == 4 ? firstValue : secondValue;
                std::vector<poker> fourCards;
                for (const auto& card : cards) {
                    if (card.value == fourValue) {
                        fourCards.push_back(card);
                    }
                }
                keyCard = highestCard(fourCards);
                return;
            }

            if ((firstCount == 3 && secondCount == 2) || (firstCount == 2 && secondCount == 3)) {
                type = HandType::FULL_HOUSE;
                int tripleValue = firstCount == 3 ? firstValue : secondValue;
                std::vector<poker> tripleCards;
                for (const auto& card : cards) {
                    if (card.value == tripleValue) {
                        tripleCards.push_back(card);
                    }
                }
                keyCard = highestCard(tripleCards);
                return;
            }
        }
    }

    type = HandType::INVALID;
}


std::map<int, std::vector<poker>> game_roler::groupByValue(const std::vector<poker>& cards) {
    std::map<int, std::vector<poker>> vMap;
    for (const auto& c : cards) vMap[c.value].push_back(c);
    return vMap;
};
std::vector<hand> game_roler::findAllValidHands(const std::vector<poker>& myCards, HandType targetType) {
    std::vector<hand> results;
    auto vMap = groupByValue(myCards);
    std::vector<poker> cards = myCards;
    std::sort(cards.begin(), cards.end());

    switch (targetType) {
        case HandType::SINGLE:
            for (const auto& card : cards) {
                results.emplace_back(hand(std::vector<poker>{card}));
            }
            break;
        case HandType::PAIR: {
            for (const auto& entry : vMap) {
                if (entry.second.size() >= 2) {
                    // 對於每對價值相同的牌，生成所有可能的組合
                    for (size_t i = 0; i < entry.second.size() - 1; ++i) {
                        for (size_t j = i + 1; j < entry.second.size(); ++j) {
                            results.emplace_back(hand(std::vector<poker>{entry.second[i], entry.second[j]}));
                        }
                    }
                }
            }
            break;
        }
        case HandType::STRAIGHT: {
        // 1. 取得所有不重複的數字 (Value)，並按「大老二權重」排序
            std::vector<int> distinctValues;
            for(auto const& [val, group] : vMap) distinctValues.push_back(val);
    
    // 排序邏輯：按 getWeight() 排，這樣 A, 2 會在最後，方便檢查 3-4-5-6-7 或 10-J-Q-K-A
            std::sort(distinctValues.begin(), distinctValues.end(), [](int a, int b) {
                poker tempA('S', a), tempB('S', b);
                return tempA.getWeight() < tempB.getWeight();
            });
            if (distinctValues.size() < 5) break;
    // 2. 檢查所有連續 5 個數字的組合
            for (size_t i = 0; i <= distinctValues.size() - 5; ++i) {
                std::vector<poker> testSuite;
                for(int j=0; j<5; ++j) testSuite.push_back(vMap[distinctValues[i+j]][0]);

        // 直接利用你寫好的 static bool isStraight
                    if (isStraight(testSuite)) {
            // 3. 笛卡兒積產生所有花色組合
                        auto& v1 = vMap[distinctValues[i]];
                        auto& v2 = vMap[distinctValues[i+1]];
                        auto& v3 = vMap[distinctValues[i+2]];
                        auto& v4 = vMap[distinctValues[i+3]];
                        auto& v5 = vMap[distinctValues[i+4]];

                        for (auto& p1 : v1) {
                            for (auto& p2 : v2) {
                                for (auto& p3 : v3) {
                                    for (auto& p4 : v4) {
                                        for (auto& p5 : v5) {
                                            results.emplace_back(std::vector<poker>{p1, p2, p3, p4, p5});
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
    // 4. 特殊處理 A-2-3-4-5 (因為在 Weight 排序中，3,4,5 在前，A,2 在後)
            if (vMap.count(1) && vMap.count(2) && vMap.count(3) && vMap.count(4) && vMap.count(5)) {
        
                for (auto& p1 : vMap[3]) 
                for (auto& p2 : vMap[4]) 
                for (auto& p3 : vMap[5]) 
                for (auto& p4 : vMap[1]) 
                for (auto& p5 : vMap[2])
                    results.emplace_back(std::vector<poker>{p1, p2, p3, p4, p5});
                }
                break;            }

        case HandType::FULL_HOUSE: {
            // 先找出所有可能的三條和對子
            std::vector<std::vector<poker>> allTriples, allPairs;
            for (auto const& [val, group] : vMap) {
                if (group.size() >= 3) {
                    // C(n, 3) 組合
                    for (size_t i = 0; i < group.size(); ++i)
                        for (size_t j = i+1; j < group.size(); ++j)
                            for (size_t k = j+1; k < group.size(); ++k)
                                allTriples.push_back({group[i], group[j], group[k]});
                }
                if (group.size() >= 2) {
                    // C(n, 2) 組合
                    for (size_t i = 0; i < group.size(); ++i)
                        for (size_t j = i+1; j < group.size(); ++j)
                            allPairs.push_back({group[i], group[j]});
                }
            }
            // 交叉組合：三條 + 對子 (且數值不同)
            for (const auto& t : allTriples) {
                for (const auto& p : allPairs) {
                    if (t[0].value != p[0].value) {
                        std::vector<poker> comb = t;
                        comb.insert(comb.end(), p.begin(), p.end());
                        results.emplace_back(comb);
                    }
                }
            }
            break;
        }

        case HandType::FOUR_KIND: {
            for (auto const& [val, group] : vMap) {
                if (group.size() == 4) {
                    for (const auto& kicker : cards) {
                        if (kicker.value != val) {
                            std::vector<poker> comb = group;
                            comb.push_back(kicker);
                            results.emplace_back(comb);
                        }
                    }
                }
            }
            break;
        }

        case HandType::STRAIGHT_FLUSH: {
            // 先按花色分組，再在每個花色內找連續 5 張
            std::map<char, std::vector<poker>> sMap;
            for (const auto& c : cards) sMap[c.shape].push_back(c);
            for (auto& [shape, sCards] : sMap) {
                if (sCards.size() < 5) continue;
                std::sort(sCards.begin(), sCards.end()); // 按權重排
                for (size_t i = 0; i <= sCards.size() - 5; ++i) {
                    std::vector<poker> window(sCards.begin() + i, sCards.begin() + i + 5);
                    hand h(window);
                    if (h.type == HandType::STRAIGHT_FLUSH) results.push_back(h);
                }
            }
            break;
        }

        default:
            break;
    }

    return results;
};
std::vector<hand> game_roler::getLegalOptions(const std::vector<poker>& myCards, const hand& lastHand, bool isFirstTurn) {
    std::vector<hand> candidates;

    // 1. 根據桌面狀態，先用 findAllValidHands 找出所有「潛在」組合
    if (lastHand.type == HandType::INVALID) {
        // 主動出牌：搜尋所有可能的牌型
        std::vector<HandType> allTypes = { 
            HandType::SINGLE, HandType::PAIR, HandType::STRAIGHT, 
            HandType::FULL_HOUSE, HandType::FOUR_KIND, HandType::STRAIGHT_FLUSH 
        };
        for (auto t : allTypes) {
            std::vector<hand> found = findAllValidHands(myCards, t);
            candidates.insert(candidates.end(), found.begin(), found.end());
        }
    } else {
        // 被動跟牌：找同類型，如果是五張牌型則搜尋更高等級的
        candidates = findAllValidHands(myCards, lastHand.type);
        if ((int)lastHand.type >= (int)HandType::STRAIGHT) {
            for (int t = (int)lastHand.type + 1; t <= (int)HandType::STRAIGHT_FLUSH; ++t) {
                std::vector<hand> higher = findAllValidHands(myCards, (HandType)t);
                candidates.insert(candidates.end(), higher.begin(), higher.end());
            }
        }
    }

    // 2. 進行關鍵過濾
    std::vector<hand> legalOptions;
    for (const auto& h : candidates) {
        // A. 檢查是否能壓過對面 (主動出牌時 lastHand 為 INVALID，canBeat 應處理或在此跳過)
        bool canBeatLast = (lastHand.type == HandType::INVALID) || h.canBeat(lastHand);
        
        // B. 檢查「第一回合」規則：組合內必須包含梅花 3
        
        bool meetsFirstTurnRule = true;
        if (isFirstTurn) {
            bool hasClub3 = game_roler::hasClub3(h.cards);
            if (!hasClub3) meetsFirstTurnRule = false;
        }

        if (canBeatLast && meetsFirstTurnRule) {
            legalOptions.push_back(h);
        }
    }

    // 3. 排序選項（從小到大），方便 AI 選擇或人類閱讀
    std::sort(legalOptions.begin(), legalOptions.end(), [](const hand& a, const hand& b) {
        return a.keyCard < b.keyCard;
    });

    return legalOptions;
}
bool game_roler::hasClub3(const std::vector<poker>& cards) {
    for (const auto& c : cards) {
        if (c.shape == 'C' && c.value == 3) return true;
    }
    return false;
}
hand interface::selectHandFromOptions(const std::vector<hand>& options, bool canPass) {
    if (options.empty()) {
        std::cout << "您沒有合法的牌可以出，強制 PASS。" << std::endl;
        return hand({});
    }

    std::cout << "--- 請選擇你要出的牌型 (輸入編號) ---" << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << "[" << i + 1 << "]: ";
        // 顯示牌型
        for (const auto& c : options[i].cards) std::cout << c.shape << c.value << " ";
        std::cout << std::endl;
    }
    
    if (canPass) std::cout << "[0]: PASS" << std::endl;

    int choice;
    while (true) {
        std::cout << "輸入編號: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }
        if (choice == 0 && canPass) return hand({}); 
        if (choice > 0 && choice <= (int)options.size()) return options[choice - 1];
        
        std::cout << "無效輸入" << (canPass ? "" : " (本回合不可 PASS)") << "，請重試。" << std::endl;
    }
};
bool poker_pool::isshape(char a){
    if (shape.find(a) != std::string::npos) { 
    return true;
    }else{
        return false;
    }

};
bool poker_pool::isnumber(int a){
    return a<14 && a>0;
}
void poker_pool::set_shape_pool(int type){
    pool.clear();
    for(int j=1;j<=13;j++){
        pool.push_back(poker(shape[type],j));
    }
    
};
void poker_pool::set_num_pool(const int num){
    pool.clear();
    for(int i=0;i<4;i++){
        pool.push_back(poker(shape[i],num));
    }

};
void poker_pool::set_ip_pool(const std::vector<std::string>& inlist){
    pool.clear();
    for(std::string a:inlist){
        if(isshape(a[0])&&(stoi(a.substr(1))<14))
        pool.push_back(poker(a[0],stoi(a.substr(1))));
    }

};
void poker_pool::set_random_pool(){
    set_all_pool();
    wash_pool();
    if (pool.empty()) return;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<> dis(1, pool.size());
    int n = dis(gen);

    if (n < pool.size()) {
    pool.erase(pool.begin() + n, pool.end());
    }
};
void poker_pool::set_all_pool(){
     for(int i=0;i<4;i++){
        for(int j=1;j<=13;j++){
        pool.push_back(poker(shape[i],j));
    }    
    }
};
std::vector<poker> poker_pool::get_pool(){
    return poker_pool::pool;
};
void poker_pool::wash_pool(){
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pool.begin(), pool.end(), g);
};
std::string poker_print::pok_shape(char shape,int row){
    static std::map<char,std::vector<std::string>> shapes = {
        {'H',{{"|    +++   +++     |"},{"|   +++++ +++++    |"},{"|   +++++++++++    |"},{"|    +++++++++     |"},{"|      +++++       |"},{"|       +++        |"},{"|                  |"}}},
        {'D',{{"|        +         |"},{"|       +++        |"},{"|     +++++++      |"},{"|    +++++++++     |"},{"|     +++++++      |"},{"|       +++        |"},{"|        +         |"}}},
        {'S',{{"|        +         |"},{"|     +++++++      |"},{"|   +++++++++++    |"},{"|  +++++++++++++   |"},{"|  +++++++++++++   |"},{"|   +++ +++ +++    |"},{"|       +++        |"}}},
        {'C',{{"|       +++        |"},{"|      +++++       |"},{"|  +++ +++++ +++   |"},{"| +++++++++++++++  |"},{"| +++++++++++++++  |"},{"|  +++  +++  +++   |"},{"|       +++        |"}}}
    };
    

    if (shapes.find(shape) == shapes.end()) {
        return "|      ERROR      |"+std::to_string(row); 
    }
    if (row < 0 || row >= shapes[shape].size()) {
        return "|   ROW ERROR     |"+std::to_string(row);
    }

    return shapes[shape][row];
};
std::string poker_print::card_row(const poker& card,int row){
    int num_space=17;
    std::string value;
    if(card.value==11) value="J";
    else if(card.value==12) value="Q";
    else if(card.value==13) value="K";
    else if(card.value==1) value="A";
    else value=std::to_string(card.value);

    if(card.value==10) num_space=16;
    if(row==1) return std::string(20, '_');
    else if(row==2) return "|"+std::string(18, ' ')+"|";
    else if(row==3) return "| "+value+std::string(num_space-1, ' ')+"|";
    else if(row==11) return "|"+std::string(num_space-1, ' ')+value+" |";
    else if(row==12) return "|"+std::string(18, '_')+"|";
    else return pok_shape(card.shape,row-4);

};
void poker_print::print_onepok(char a,int v){
    
    poker pok(a,v);
    for(int i=1;i<13;i++){
        std::cout<<card_row(pok,i)<<std::endl;
    }

};
void poker_print::print_list(const std::vector<poker>& p,int row){
    int total = p.size();
    for (int i = 0; i < total; i += row) {
        for (int line = 1; line < 13; ++line) {
            for (int j = i; j < i + row && j < total; ++j) {
                std::cout << card_row(p[j], line) << "  ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

};
chore::chore(){
    line = std::string(50,'-');
    space = std::string(5,' ');
};

bool chore::isshape(char a){
    if (shape.find(a) != std::string::npos) { 
    return true;
    }else{
        return false;
    }

};
bool chore::isnumber(int a){
    return a<14 && a>0;
}
