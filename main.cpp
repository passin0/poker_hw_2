#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <map>
#include <algorithm>
#include "poker_game.h"
using namespace std;

string getHandTypeName(HandType type) {
    switch (type) {
        case HandType::SINGLE: return "Single";
        case HandType::PAIR: return "Pair";
        case HandType::STRAIGHT: return "Straight";
        case HandType::FULL_HOUSE: return "Full House";
        case HandType::FOUR_KIND: return "Four of a Kind";
        case HandType::STRAIGHT_FLUSH: return "Straight Flush";
        default: return "Invalid";
    }
}

int main()
{
    // ===== Game initialization =====
    int numPlayers = 0, numRounds = 0;

    cout << "===== Big Two Game =====" << endl;
    cout << "Enter number of players (2-4): ";
    while (!(cin >> numPlayers) || numPlayers < 2 || numPlayers > 4)
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input, enter 2-4: ";
    }
    

    cout << "Enter number of rounds: ";
    while (!(cin >> numRounds) || numRounds < 1)
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input, enter a positive integer: ";
    }

    // --- Display mode selection ---
    int displayMode = 0; 
    cout << "Choose display mode (0: text mode, 1: graphic card mode): ";
    while (!(cin >> displayMode) || (displayMode < 0 || displayMode > 1))
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input, enter 0 or 1: ";
    }
    bool useGraphicMode = (displayMode == 1);

    // 建立玩家容器
    vector<player *> players;
    players.push_back(new player(1)); // 玩家 1 由人類控制
    for (int i = 2; i <= numPlayers; i++)
    {
        players.push_back(new auto_player(i));
    }

    map<int, int> totalScores; // 記錄累積分數
    for (int i = 0; i < numPlayers; i++)
    {
        totalScores[i + 1] = 0;
    }

    // ===== 遊戲主迴圈 =====
    for (int round = 1; round <= numRounds; round++)
    {   
        int playedcount = 0;
        cout << "\n========== Round " << round << " ==========" << endl;

        // ----- Pre-round setup: initialize deck and deal cards -----
        poker_pool pool;
        pool.set_all_pool();
        pool.wash_pool();
        // 清空玩家手牌
        for (auto p : players)
        {
            p->hand_pool.clear();
            p->hasClub3 = false;
            p->score = 0;
        }
        vector<poker> allCards = pool.get_pool();
        int cardsPerPlayer = 52 / numPlayers;

        // 1. 輪流發放基礎牌數
        for (int i = 0; i < cardsPerPlayer; i++)
        {
            for (auto p : players)
            {
                p->getcard(allCards.back()); // 從牌堆頂端（後方）取牌
                allCards.pop_back();         // 移除已發出的牌
            }
        }

        // 2. 處理 3 人模式多出的牌 (剩餘 1 張)
        // 根據規則：多出的 1 張牌發給持有梅花 3 的玩家
        if (!allCards.empty())
        {
            bool dealtExtra = false;
            for (auto p : players)
            {
                if (p->hasClub3)
                { // 檢查誰在剛剛的發牌過程中拿到了梅花 3
                    p->getcard(allCards.back());
                    allCards.pop_back();
                    dealtExtra = true;
                    break;
                }
            }

            // 特殊狀況：如果梅花 3 剛好是最後那張牌，則發給 Player 1 (或依規則指定)
            if (!dealtExtra && !allCards.empty())
            {
                players[0]->getcard(allCards.back());
                allCards.pop_back();
            }
        }

        // 排序玩家手牌
        for (auto p : players)
        {
            p->sort_cards();
        }

        // ----- 判定首出玩家 -----
        int firstPlayerIdx = -1;
        for (int i = 0; i < numPlayers; i++)
        {
            if (players[i]->hasClub3)
            {
                firstPlayerIdx = i;
                break;
            }
        }

        if (firstPlayerIdx == -1)
        {
            firstPlayerIdx = 0;
        }

        cout << "First player: Player " << (firstPlayerIdx + 1) << endl;

        // ----- Core game loop =====

        hand tableHand = hand({}); // 桌面牌型 (初始為空)
        int lastPlayerIdx = -1;    // 上一手出牌者
        int currentPlayerIdx = firstPlayerIdx;
        int passCount = 0; // 連續 pass 計數

        vector<int> roundFinishOrder; // 本輪出完牌的玩家序列

        while (roundFinishOrder.size() < (size_t)numPlayers - 1)
        {   
            // 1. 跳過已經出完牌的玩家
            while (players[currentPlayerIdx]->hand_pool.empty())
            {
                currentPlayerIdx = (currentPlayerIdx + 1) % numPlayers;

                // 如果跳過的人剛好是最後出牌者，代表權限要移交給下一個「還在場上」的人
                if (currentPlayerIdx == lastPlayerIdx)
                {
                    tableHand = hand({}); // 依然要清空桌面
                }
            }

            // 2. 【關鍵：決策前判定】是否輪回最後出牌者？如果是，清空桌面
            // 這樣下方的 getLegalOptions 才會知道現在是「自由出牌」模式
            if (currentPlayerIdx == lastPlayerIdx && tableHand.type != HandType::INVALID)
            {
                cout << "\n--- All other players passed. Player " << (currentPlayerIdx + 1) << " plays again ---" << endl;
                tableHand = hand({});
                passCount = 0; // reset count
            }

            // 3. 獲取選項 (此時 tableHand 已經可能是 INVALID)
            bool isFirstTurn = (tableHand.type == HandType::INVALID && lastPlayerIdx == -1);
            vector<hand> legalOptions = game_roler::getLegalOptions(
                players[currentPlayerIdx]->hand_pool,
                tableHand,
                isFirstTurn);

            // 當前玩家出牌
            hand playedHand = hand({});
            int playerId = currentPlayerIdx + 1;
            playedcount++;
            if (playerId == 1)
            {
                // Player 1 (human)
                cout << "\n==================== Player 1's turn ====================" << " (turn " << playedcount/3+1 << ")" << endl;
                cout << "Your hand:" << endl;
                if (useGraphicMode) poker_print::print_list(players[0]->hand_pool, 7);
                else {
                    for (size_t i = 0; i < players[0]->hand_pool.size(); i++)
                        cout << "[" << i << "]" << players[0]->hand_pool[i].shape << players[0]->hand_pool[i].value << " ";
                    cout << endl;
                }

                cout << "Table status: ";
                if (tableHand.type == HandType::INVALID) cout << "(none)" << endl;
                else {
                    cout << getHandTypeName(tableHand.type) << endl;
                    if (useGraphicMode) poker_print::print_list(tableHand.cards, 5);
                    else {
                        for (const auto &c : tableHand.cards) cout << c.shape << c.value << " ";
                        cout << endl;
                    }
                }

                if (legalOptions.empty())
                {
                    cout << "No legal cards available, forced PASS." << endl;
                    playedHand = hand({});
                    passCount++;
                    // This section completes directly and will proceed to index movement logic below
                }
                else
                {
                    bool confirmedChoice = false;
                    while (!confirmedChoice)
                    {
                        // 1. 將選項按牌型分群
                        map<HandType, vector<int>> groupedOptions;
                        for (int i = 0; i < (int)legalOptions.size(); i++) {
                            groupedOptions[legalOptions[i].type].push_back(i);
                        }

                        cout << "\n--- Legal options categories ---" << endl;
                        vector<HandType> availableTypes;
                        int typeIdx = 1;
                        for (auto const& [type, indices] : groupedOptions) {
                            cout << "[" << typeIdx << "] View " << getHandTypeName(type) << " (" << indices.size() << " items)" << endl;
                            availableTypes.push_back(type);
                            typeIdx++;
                        }
                        cout << "[A] Show all options" << endl;
                        cout << "[0] Choose PASS" << endl;

                        cout << "Enter category number, 'A', or '0': ";
                        string cmd;
                        cin >> cmd;

                        // --- 處理 PASS 邏輯 (完全同步原版) ---
                        if (cmd == "0") {
                            if (tableHand.type != HandType::INVALID || !isFirstTurn) {
                                playedHand = hand({});
                                passCount++;
                                confirmedChoice = true;
                            } else {
                                cout << ">> The player with Club 3 cannot PASS on the first turn!" << endl;
                            }
                        }
                        // --- 顯示所有選項 ---
                        else if (cmd == "A" || cmd == "a") {
                            for (size_t i = 0; i < legalOptions.size(); i++) {
                                cout << "[" << (i + 1) << "]: " << getHandTypeName(legalOptions[i].type) << ": ";
                                for (const auto &c : legalOptions[i].cards) cout << c.shape << c.value << " ";
                                cout << endl;
                            }
                            cout << "Enter option number (1-" << legalOptions.size() << ") or enter 'R' to return: ";
                            string subCmd;
                            cin >> subCmd;
                            if (subCmd == "R" || subCmd == "r") continue;
                            try {
                                int choice = stoi(subCmd);
                                if (choice > 0 && choice <= (int)legalOptions.size()) {
                                    playedHand = legalOptions[choice - 1];
                                    passCount = 0; // original logic: reset count when a play is made
                                    confirmedChoice = true;
                                }
                            } catch (...) { cout << "Invalid option number!" << endl; }
                        }
                        // --- 進入特定分類 ---
                        else {
                            try {
                                int tChoice = stoi(cmd);
                                if (tChoice > 0 && tChoice <= (int)availableTypes.size()) {
                                    HandType selectedType = availableTypes[tChoice - 1];
                                    cout << "\n--- " << getHandTypeName(selectedType) << " 列表 ---" << endl;
                                    for (int idx : groupedOptions[selectedType]) {
                                        cout << "[" << (idx + 1) << "]: ";
                                        if (useGraphicMode) {
                                            cout << endl;
                                            poker_print::print_list(legalOptions[idx].cards, 5);
                                        } else {
                                            for (const auto &c : legalOptions[idx].cards) cout << c.shape << c.value << " ";
                                            cout << endl;
                                        }
                                    }
                                    cout << "Enter number (1-" << legalOptions.size() << ") or 'R' to return: ";
                                    string subCmd;
                                    cin >> subCmd;
                                    if (subCmd == "R" || subCmd == "r") continue;
                                    int choice = stoi(subCmd);
                                    if (choice > 0 && choice <= (int)legalOptions.size()) {
                                        playedHand = legalOptions[choice - 1];
                                        passCount = 0; // 原版邏輯：出牌則重置計數
                                        confirmedChoice = true;
                                    }
                                }
                            } catch (...) { cout << "Invalid input, please choose again." << endl; }
                        }
                    }
                }
            }
            else
            {
                // 電腦玩家 (Player 2-4)
                if (legalOptions.empty())
                {
                    cout << "Player " << playerId << " PASS" << endl;
                    playedHand = hand({});
                    passCount++;
                }
                else
                {
                    playedHand = ((auto_player *)players[currentPlayerIdx])->makeDecision(tableHand, isFirstTurn);
                    if (playedHand.type == HandType::INVALID)
                    {
                        cout << "Player " << playerId << " PASS" << endl;
                        passCount++;
                    }
                    else
                    {
                        cout << "Player " << playerId << " 出: " << getHandTypeName(playedHand.type) << endl;
                        if (useGraphicMode) {
                            poker_print::print_list(playedHand.cards, 5);
                        } else {
                            for (const auto &c : playedHand.cards)
                            {
                                cout << c.shape << c.value << " ";
                            }
                            cout << endl;
                        }
                        passCount = 0;
                    }
                }
            }
            if (playedHand.type != HandType::INVALID)
            {
                tableHand = playedHand;
                lastPlayerIdx = currentPlayerIdx; // 只要有人出牌，就更新最後出牌者
                players[currentPlayerIdx]->removeCards(playedHand);

                // 檢查是否剛好出完牌
                if (players[currentPlayerIdx]->hand_pool.empty())
                {
                    roundFinishOrder.push_back(currentPlayerIdx);
                }
            }
            else
            {
                // If Pass, do not update lastPlayerIdx
                cout << "Player " << (currentPlayerIdx + 1) << " chose Pass." << endl;
            }

            // 下一位玩家
            currentPlayerIdx = (currentPlayerIdx + 1) % numPlayers;
        }

        // ----- Round settlement -----
        cout << "\n--- Round results ---" << endl;
        int scores[4] = {4, 3, 2, 1};
        for (size_t i = 0; i < roundFinishOrder.size(); i++)
        {
            int playerIdx = roundFinishOrder[i];
            int score = scores[i];
            players[playerIdx]->score += score;
            totalScores[playerIdx + 1] += score;
            cout << "Player " << (playerIdx + 1) << " finished rank " << (i + 1)
                 << ", score " << score << " (total: " << totalScores[playerIdx + 1] << ")" << endl;
        }
        if (round < numRounds)
        {
            cout << "There are "<< (numRounds - round) <<" rounds left. Continue? (Y/N): ";
            char continueChoice;
            cin >> continueChoice;
            if (continueChoice != 'Y' && continueChoice != 'y')
            {
                break;
            }
        }
    }

    // ===== Game settlement =====
    cout << "\n========== Final rankings ==========" << endl;
    vector<pair<int, int>> finalRanking;
    for (int i = 1; i <= numPlayers; i++)
    {
        finalRanking.push_back({i, totalScores[i]});
    }
    sort(finalRanking.begin(), finalRanking.end(),
         [](const pair<int, int> &a, const pair<int, int> &b)
         {
             return a.second > b.second;
         });

    for (size_t i = 0; i < finalRanking.size(); i++)
    {
        cout << "Rank " << (i + 1) << ": Player " << finalRanking[i].first
             << " (Total: " << finalRanking[i].second << ")" << endl;
    }

    // 清理記憶體
    for (auto p : players)
    {
        delete p;
    }

    return 0;
}