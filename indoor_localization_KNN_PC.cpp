/******************************************************************************
 *                                                                            *
 *  Indoor Self Localization in C++  [PC windows/Linux]                       *
 *                                                                            *
 ******************************************************************************
 * Author: Joao Nuno Carvalho                                                 *
 * Date: 2019.09.23                                                           *
 * License: MIT Open Source License                                           *
 *                                                                            *
 * Description: This is a implementation in C++ on a PC to make the WIFI      * 
 *              Indoor Localization with the KNN K-Nearest-Neighbors machine  *
 *              learning algorithm. The following code is only here to        *
 * facilitate the development of the final algorithms in the ESP32.           *
 * But it can also be seen as one more example of the use of the my           *
 * implementation of KNN to work on the PC and how simple it is to use it in  *
 * C++. This program uses the .h file generated in the Python program, it as  *
 * to be the PC version, not the Arduino version because that uses a          *
 * different namespace.                                                       *
 * This PC version uses a Euclidean distance and doesn't use the secret       *
 * sauce custom tunned version.                                               *
 * The unique routers of the .H file correspond to all routers that exist on  *
 * the train and test dataset. The same doesn't occur in the real life        *
 * Arduino ESP32 sensor data point, that's why the secret sauce is so         *
 * important.                                                                 *
 * To compile this project use a C++ compiler, I used MINGW32 on windows, but *
 * you can use any compiler it is C++ standard.                               *
 * To compile in Linux do in a terminal                                       *
 *    "make clean"                                                            *
 *    "make" or "make TARGET=debug"                                           *
 *    "./indoor_localization_KNN_PC.exe"                                      *
 * in windows do                                                              *
 *    "mingw32-make clean"                                                    *
 *    "mingw32-make" or "mingw32-make TARGET=debug"                           *
 *    "indoor_localization_KNN_PC.exe"                                        *
 * I used Visual Studio code for the development and debugging, it works on   *
 * Windows/Linux/Mac.                                                         *
 * For more details on secret sauce and the algorithm see github project page *
 * in The Secret Sauce and in the references                                  *
 ******************************************************************************
*/  

#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <math.h>       /* sqrt */
#include <utility>      /* std::pair, std::make_pair */
#include <algorithm>    /* std::count */
#include "home_train_data.h"

using namespace std;

constexpr int k = 5;
constexpr float C_NOT_SEEN_ROUTER_SIGNAL_VALUE = 120;
vector<string> vec_class_strings;

float distance(vector<float> &point_A, vector<float> &point_B){
    float sum = 0.0;
    for(size_t i = 0; i < point_A.size(); ++i){   
        float a = point_A[i]-point_B[i];
        sum += a*a;
    }
    return sqrt(sum);
}

int KNN_classifier(vector<vector<float>> &vec_X_train, vector<int> &vec_Y_train,
                   int k, vector<float> data_point){
    // Returns y_point.

    // Calculate the distance between data_point and all points.
    vector<pair<int, float>> dist_vec {};
    pair<int, float> pair_tmp;
    for(size_t i = 0; i < vec_X_train.size(); ++i){   
        pair_tmp = make_pair(i, distance(vec_X_train[i], data_point));
        dist_vec.push_back(pair_tmp);
    }

    // Sort dist_vec by distance ascending.
    auto sortRuleLambda = [] (pair<int, float> const& s1, pair<int, float> const& s2) -> bool
    {
       return s1.second < s2.second;
    };
    
    sort(dist_vec.begin(), dist_vec.end(), sortRuleLambda);

    // Majority vote on the Y target int class.
    const size_t num_classes = vec_class_strings.size();
    for(size_t i = k; i > 0; --i){
        vector<int> vec_Y_class_target(num_classes);
        for(size_t c = 0; c < num_classes; ++c){
            // c is the value of the class to be compared to the top K elements,
            // to see if we find a majority choosen class. 
            // If we didn't find we will try to find the majority in the best k-1
            // until we only compare to one and in that case it will be the majority.
            for(size_t j=0; j<i; ++j){
                int index = dist_vec[j].first;
                if (vec_Y_train[index] == static_cast<int>(c))
                    vec_Y_class_target[c]++;
            }
        }

        // Find the maximum value fo the counting histogram.
        float max = -1;
        size_t max_index = -1;
        for(size_t r = 0; r < vec_Y_class_target.size(); ++r){
            int elem = vec_Y_class_target[r];
            if (elem >= max){
                max = elem;
                max_index = r;
            }
        }

        bool flag_continue = false;
        // Find if there is only one max value on the counting histogram.
        for(size_t r = 0; r < vec_Y_class_target.size(); ++r){
            int elem = vec_Y_class_target[r];
            if ((elem == max) && (r != max_index)){
                flag_continue = true;
                break; 
            }
        }

        if (flag_continue == true)
            continue;
        else{
            return static_cast<int>(max_index);
        }
    }

    return -1;
}

void print_data_point(vector<float> data_point, int y_point){
    for(size_t i=0; i < data_point.size(); ++i)
        cout << data_point[i] << ",";
    cout << vec_class_strings[y_point] << endl; 
}

void evaluate_all_dataset(vector<vector<float>> &vec_X_train, vector<int> &vec_Y_train, int k,
                          vector<vector<float>> &vec_X_dataset, vector<int> &vec_Y_dataset,
                          int &dataset_len, int &correct_dataset_pred, float &correct_dataset_pred_perc){
    dataset_len = vec_X_dataset.size();
    correct_dataset_pred = 0;
    correct_dataset_pred_perc = 0.0;    
    vector<float> data_point;
    for(size_t i = 0; i < vec_X_dataset.size(); ++i){
        vector<float> data_point = vec_X_dataset[i];
        int y_point = KNN_classifier(vec_X_train, vec_Y_train, k, data_point);
        if (y_point == vec_Y_dataset[i])
            correct_dataset_pred++;
        else{
            print_data_point(data_point, y_point);
        }        
    }
    correct_dataset_pred_perc = (static_cast<float>(correct_dataset_pred) / dataset_len) * 100;
}

// Get the keys of a ordered map, not a hashtable, but a tree.
template<class KEY, class VALUE>
vector<KEY> getKeys(const map<KEY, VALUE>& map)
{
    vector<KEY> keys {};
    for (const auto& it : map)
        keys.push_back(it.first);
    return keys;
}


vector<float> get_wifi_networks_data_point(){


    vector<pair<string, float>> wifi_routers {};

    // TODO: Uncomment the following code for ARDUINO IDE.
/*
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
       Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
     
            pair<string, float> wifi_pair = make_pair( WiFi.SSID(i), static_cast<float>( WiFi.RSSI(i) ) );    
            wifi_routers.push_back(wifi_pair);
 
            delay(10);
        }
    }

*/

    vector<float> data_point {};

    for(size_t i = 0; i < vec_target_table_Y.size(); ++i){
        string router_name_def = vec_target_table_Y[i];
        for( pair<string, float> pair : wifi_routers){
            string router_name = pair.first;
            if (router_name_def == router_name){
                float signal_strength = pair.second;        
                data_point.push_back(signal_strength);
            }else{
                data_point.push_back(C_NOT_SEEN_ROUTER_SIGNAL_VALUE);
            }
        }
    }

    return data_point;
}

int main(){
/*  
    #### .h global variables generated by python script:

    vector<string> vec_target_table_Y 
    // HashTable -> X router_name (string) to feature_index (size_t).
    map<string,int> map_x_router_name_to_index 
    vector<vector<float>> vec_X_train 
    vector<int> vec_Y_train
    vector<vector<float>> vec_X_test 
    vector<int> vec_Y_test
*/

    cout << endl << "Indoor localization with KNN K-Nearest-Neighbors in C++" << endl << endl;

    vec_class_strings = getKeys(map_x_router_name_to_index);

    // Evaluate the correct train set percentage.
    int train_len = 0;
    int correct_train_pred = 0;
    float correct_train_pred_perc = 0.0;    
    evaluate_all_dataset(vec_X_train, vec_Y_train, k,
                         vec_X_train, vec_Y_train, // This are the one being tested.
                         train_len, correct_train_pred, correct_train_pred_perc);

    cout << "Correct classification in train set \n\ttrain_len: " << train_len 
         << "\n\t correct_train_pred: "      << correct_train_pred
         << "\n\t correct_train_pred_perc: " << correct_train_pred_perc << endl;

    // Evaluate the correct test set percentage.
    int test_len = 0;
    int correct_test_pred = 0;
    float correct_test_pred_perc = 0.0;    
    evaluate_all_dataset(vec_X_train, vec_Y_train, k,
                         vec_X_test, vec_Y_test,  // This are the one being tested.
                         test_len, correct_test_pred, correct_test_pred_perc);

    cout << "Correct classification in test set \n\ttest_len: " << test_len 
         << "\n\t correct_test_pred: "      << correct_test_pred
         << "\n\t correct_test_pred_perc: " << correct_test_pred_perc << endl;

    cout << endl;

    // More detailed tests, the first 5 cases from test_dataset, but used once.

    vector<vector<float>> data_points_list
    {
        {120,120,120,120,120,120,120,120,120,120,89,120,120,120,88,120,120,120,120,120,120,91,120,120,120,93,120,120,120,94,120,120,120,120,94,91,81,120,94,75,120,88,120,120,120,120,120,120,120,120,120,120,94,94,120,89,120,120,95},
        {120,90,120,120,120,120,120,120,93,120,120,120,120,92,95,120,120,120,120,120,120,120,120,120,120,120,120,120,120,86,120,120,120,120,93,120,120,120,120,82,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,86,120,120,120},
        {120,120,120,120,120,120,120,120,120,120,88,95,120,120,90,120,120,120,120,120,120,94,120,120,120,94,120,120,120,95,120,120,120,120,94,90,90,120,92,75,120,87,120,120,120,120,120,120,120,120,91,120,90,92,120,92,120,120,93},
        {120,88,120,85,120,120,93,120,120,95,120,120,93,120,90,120,120,120,120,120,120,120,120,120,120,120,120,90,120,94,92,120,120,93,85,120,84,120,120,89,120,120,120,94,120,120,120,120,120,89,120,120,120,120,120,120,93,120,120},
        {120,120,120,120,120,120,120,120,120,120,88,120,120,120,92,92,120,120,120,120,120,92,120,120,120,120,120,120,120,94,120,120,120,120,89,92,83,120,95,79,120,89,120,120,120,120,120,120,120,120,95,120,92,120,92,92,120,120,120}
    };

    vector<int> vec_Y_correct
    {
        4,
        0,
        4,
        2,
        4
    };

    int index = 0;
    for(auto data_point : data_points_list){
        int y_point = KNN_classifier(vec_X_train,vec_Y_train,
                                 k, data_point);
        cout << "data_point correct: " << vec_Y_correct[index] << " data_point previsto: " << y_point << endl;
        print_data_point(data_point, y_point);
        index++;
    }

    // ARDUINO code.

    // Sensor code aquisition of data_point.
    // vector<float> sensor_data_point = get_wifi_networks_data_point();
    // int sensor_y_point = KNN_classifier(vec_X_train,vec_Y_train,
    //                                     k, sensor_data_point);
    // Serial.print("Localization: ");
    // Serial.println(vec_class_strings[sensor_y_point]);

    return 0;
}

