
#pragma once


#ifndef __HOME_TRAIN_DATA_MOCK_H_
#define __HOME_TRAIN_DATA_MOCK_H_

    
#include <string>
#include <vector>
#include <map>

using namespace std;

// Lookup table of routers, Y int target to Y classifier Class name target.
// Usage pattern "vec_target_table_Y[Y_int]"   
vector<string> vec_target_table_Y 
{ 
    "router_name_0",
    "router_name_1",
    "router_name_3" 
};

// HashTable -> X router_name (string) to feature_index (size_t).
map<string,int> map_x_router_name_to_index 
{
    {"router_name_0", 0},
    {"router_name_1", 1},
    {"router_name_2", 2} 
};

vector<vector<float>> vec_X_train 
{
    {11,120,31,40,60,50,70},
    {12,120,32,40,60,50,70},
    {13,120,33,40,60,50,70},
    //...
    {14,120,34,40,60,50,70},
    {15,120,35,40,60,50,70}
};

vector<int> vec_Y_train
{
    0,1,2,0,1,
    //...
    2,0,1,2
};

vector<vector<float>> vec_X_test 
{
    {21,120,31,40,60,50,70},
    {22,120,32,40,60,50,70},
    {23,120,33,40,60,50,70},
    //...
    {24,120,34,40,60,50,70},
    {25,120,35,40,60,50,70}
};

vector<int> vec_Y_test
{
    0,1,2,0,1,
    //...
    2,0,1,2
};


#endif
