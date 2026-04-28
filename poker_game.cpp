#include<iostream>
#include<string>
#include<vector>
#include<random>
#include<algorithm>
#include<map>
#include "poker_game.h"
poker::poker(char shape,int value){
        this->shape=shape;
        this->value=value;
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
player::player(int id):id(id){

};
std::vector<hand> player::findAllPairs(){
    std::vector<hand> pairs;
    std::map<int, std::vector<poker>> valueMap;
    for (const auto& card : hand_pool) {
        valueMap[card.value].push_back(card);
    }
    for (const auto& entry : valueMap) {
        if (entry.second.size() >= 2) {
            pairs.emplace_back(hand(std::vector<poker>{entry.second[0], entry.second[1]}));
        }
    }
    return pairs;
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

void player::removeCards(const hand& playedHand) {
    for (const auto& c : playedHand.cards) {
        for (const auto& targetCard : playedHand.cards) {
        // 使用 std::remove_if 尋找「花色」與「數值」完全相同的牌
        hand_pool.erase(std::remove_if(hand_pool.begin(), hand_pool.end(),
            [&](const poker& c) {
                return c.value == targetCard.value && c.shape == targetCard.shape;
            }), hand_pool.end());
    }
    }
};

std::vector<hand> game_roler::findAllValidHands(const std::vector<poker>& myCards, HandType targetType) {
    std::vector<hand> results;
    if (myCards.empty() || targetType == HandType::INVALID) {
        return results;
    }

    std::vector<poker> cards = myCards;
    std::sort(cards.begin(), cards.end());

    switch (targetType) {
        case HandType::SINGLE:
            for (const auto& card : cards) {
                results.emplace_back(hand(std::vector<poker>{card}));
            }
            break;

        case HandType::PAIR: {
            std::map<int, std::vector<poker>> valueMap;
            for (const auto& card : cards) {
                valueMap[card.value].push_back(card);
            }
            for (const auto& entry : valueMap) {
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
            // 1. 找出所有連續的 5 個數值序列 (例如: 3,4,5,6,7)
            // 提示：大老二需特殊處理 A,2 的權重。建議先取 Weight 排序。
            std::vector<int> weights;
            for(auto const& [val, group] : vMap) weights.push_back(group[0].getWeight());
            std::sort(weights.begin(), weights.end());

            for (size_t i = 0; i <= (weights.size() > 5 ? weights.size() - 5 : 0); ++i) {
                bool isSeq = true;
                for (int j = 0; j < 4; ++j) {
                    if (weights[i+j+1] != weights[i+j] + 1) { isSeq = false; break; }
                }
                
                if (isSeq) {
                    // 2. 找到數值序列後，對這 5 個數值的對應牌組做「笛卡兒積」
                    // 這部分通常建議限制只回傳最大花色的那一組，或用遞迴生成全組合
                    // 這裡為了簡化，示範概念：
                    // 
                }
            }
            break;
        }

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
