#include<iostream>
#include<string>
#include<vector>
#include<random>
#include<algorithm>
#include<map>
#include "poker_game.h"
poker::poker(char shape,int value){
        poker::shape=shape;
        poker::value=value;
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
std::string poker_print::card_row(const class poker::poker& card,int row){
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
