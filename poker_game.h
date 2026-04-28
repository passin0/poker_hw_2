#ifndef POKER_GAME_H
#define POKER_GAME_H
#include<iostream>
#include<string>
#include<vector>
//H:heart D:diamond C:clubs S:spades
class poker{
public:
    char shape;
    int value;
    int shape_num;
    poker(char shape,int value);
   
    int getWeight() const;

    bool operator<(const poker& other) const;
};
enum class HandType {
    INVALID = 0, SINGLE, PAIR, STRAIGHT, FULL_HOUSE, FOUR_KIND, STRAIGHT_FLUSH
};

class hand{
public:
    std::vector<poker> cards;
    HandType type;
    poker keyCard;

    hand(std::vector<poker> selectedCards);
    bool canBeat(const hand& other) const;
private:
    void validate();
};
class player{//TODO
public:
    int id;
    std::vector<poker> hand_pool;
    int score;
    
    player(int id);
    std::vector<hand> findAllPairs();
    void removeCards(const hand& playedHand);
//TODO　出牌 (合法出牌選項)　接收牌　理牌　看牌  回合確定 要分數  
};
class game_roler{//TODO 玩家註冊給分 比較規則(大小 輔助建議出牌) 流程給chore main 初始化發牌(大池與手牌) 紀錄上一個牌 
// 核心功能：輸入手牌 + 目標牌型 -> 回傳所有可能的組合
public:
    static std::vector<hand> findAllValidHands(const std::vector<poker>& myCards, HandType targetType);
    /*遊戲前置作業
1. 一開始每位玩家所有積分皆為 0。
2. 玩家數目限定 2~4 人，程式一開始助教要可以輸入人數以及要玩幾輪，除了助教
外的玩家皆由電腦控制。
3. 當玩家數為 3 人時，牌數無法平均分配，多出的 1 張牌發給持有梅花 3 的玩
家。
牌面大小
點數大小
3<4<5<6<7<8<9<10<J<Q<K<A<2
花色大小
梅花<方塊<紅心<黑桃
出牌種類  這裡要處理一個了
單張：先比點數，點數相同再比花色。
一對：先比點數，點數相同則比該對中最大花色。
順子：5 張連續牌，允許 A、2，例如：A 2 3 4 5、10 J Q K A 可，J Q K A 2 不
可；大小以最大牌點數為準，點數相同再比該牌花色。
葫蘆：3 張相同點數 + 1 對，比「三張那組」的點數。
鐵支：4 張相同點數 + 1 張任意牌，比「四張那組」的點數。
同花順：5 張同花且連號，比順子中的最大牌。
pass：跳過，換下一位玩家出牌。
五張牌型大小順序：順子 < 葫蘆 < 鐵支 < 同花順。
本作業不包含同花、三條牌型。
流程
1. 玩家以 ID 表示，玩家 1 為助教，其餘為電腦玩家，出牌順序依 ID 遞增循環
進行。
2. 在每輪遊戲一開始由手上有梅花三的玩家出牌且不可 pass，若第一手出單張，必
須直接出梅花 3，若第一手出一對或五張牌型，必須包含梅花 3。
3. 之後出牌順序為由 ID 小到大的玩家依序出牌(例如:假設已決定好先手玩家為 2
號，那下一個出牌的玩家即為 3 號)。
4. 程式要可以印出目前是第幾輪、本輪出牌順序。
5. 每次出牌時，需顯示目前桌面牌型與上一手出牌玩家。輪到助教出牌時，要顯示
出目前手牌，並詢問要出哪張牌或 pass，需輸入手牌編號（以空白分隔）來出牌
或輸入 pass。
6. 玩家出牌時，必須和目前桌面牌型張數相同，且必須比桌面牌更大。
7. 當所有人都 pass，只剩最後出牌者時，桌面清空，該玩家可自由出新的合法牌
型。
8. 第一個出完牌的玩家拿 4 分，第二個拿 3 分，以此類推。
9. 每輪遊戲結束後，輸出所有玩家目前積分。
10. 遊戲結束後比較各位玩家最終排名依總積分由高到低排序，並顯示出來(總積分相
同時，依照最後一輪遊戲
名次比較)*/
};
class auto_player : protected player{


};
class poker_pool{
    std::vector<poker> pool;
    std::string shape={'H','D','C','S'};
public:
    bool isnumber(int a);
    bool isshape(char a);
    void set_shape_pool(const int type);
    void set_num_pool(const int num);
    void set_ip_pool(const std::vector<std::string>& inlist);
    void set_random_pool();
    void set_all_pool();
    std::vector<poker> get_pool();
    void wash_pool();

};
class poker_print{
    static const std::vector<std::string>pok;
    static std::string pok_shape(char shape,int row);
    static std::string card_row(const poker& card,int row);
public:
    static void print_onepok (char a,int v);
    static void print_list (const std::vector<poker>& p,int row);
};
class chore{
    std::string line;
    std::string space;
    std::string shape={'H','D','C','S'};
    public:
    chore();
    bool isnumber(int a);
    bool isshape(char a);

};

//____________________
//|                  |
//| A                |
//|                  |
//|    +++   +++     |           
//|   +++++ +++++    |
//|   +++++++++++    |
//|    +++++++++     |
//|      +++++       |
//|       +++        |
//|                  |
//|                A |
//|__________________|
//____________________
//|                  |
//| 2                |
//|        +         |
//|       +++        |
//|     +++++++      |
//|    +++++++++     |
//|     +++++++      |
//|       +++        |
//|        +         |
//|                2 |
//|__________________|
//____________________
//|                  |
//| J                |
//|        +         |
//|     +++++++      |
//|   +++++++++++    |
//|  +++++++++++++   |
//|  +++++++++++++   |
//|   +++ +++ +++    |
//|       +++        |
//|                J |
//|__________________|
//____________________
//|                  |
//| Q                |
//|       +++        |
//|      +++++       |
//|  +++ +++++ +++   |
//| +++++++++++++++  |
//| +++++++++++++++  |
//|  +++  +++  +++   |
//|       +++        |
//|                Q |
//|__________________|


#endif