/******************************************************************************
 *                                                                            *
 *  indoor_localization_ESP32.ino in C++  [Arduino code for ESP32]            *
 *                                                                            *
 ******************************************************************************
 * Author: Joao Nuno Carvalho                                                 *
 * Date: 2019.09.23                                                           *
 * License: MIT Open Source License                                           *
 *                                                                            *
 * Description: This is a implementation in C++ for Arduino ESP32             *
 *              microcontroller that makes Indoor WiFi Localization with the  *
 *              KNN K-Nearest-Neighbors machine learning algorithm.           *
 *              This is ML on the Edge with a simple and practical algorithm, *
 * implemented from scratch.                                                  * 
 * It writes to the serial monitor of the connected PC the name of the room   *
 * that the ESP32 is on. The names that appear are the prefix of the datafile *
 * name given to each room train file dataset.                                *
 * This program uses the .h file generated in the Python program, it as       *
 * to be the Arduino version, not the PC version because that uses a          *
 * different namespace. Project component 5.                                  *
 * The total number of train data points is limited to 250 because it gave an *
 * error while fitting on the ESP32 memory when I tried to make a 330 train   *
 * dataset. There is the option of tweaking the program memory size of ESP32  *
 * so it is possible to use more samples. And the samples currently are 80 %  *
 * train and 20 % test. In the final code you can make it 100 % test and 0 %  *
 * train dataset.                                                             *
 * This Arduino version doesn't use the Euclidean distance it uses the secret *
 * sauce custom tunned version. With it it obtained good results.             *
 * The unique routers of the .H file correspond to all routers that exist on  *
 * the train and test dataset. The same doesn't occur in the real life        *
 * Arduino ESP32 WiFI sensor data point, that's why the secret sauce is so    *
 * important.                                                                 *
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

#include "WiFi.h"

#include <cstring>

// using namespace std;

constexpr int k = 5;
constexpr float C_NOT_SEEN_ROUTER_SIGNAL_VALUE = 0.0;
std::vector<std::string> vec_class_strings;

float distance(std::vector<float> &point_A, std::vector<float> &point_B){
    float num_features_matches = 0;
    double sum = 0.0;
    for(size_t i = 0; i < point_A.size(); ++i){
        if (point_A[i] == C_NOT_SEEN_ROUTER_SIGNAL_VALUE || 
            point_B[i] == C_NOT_SEEN_ROUTER_SIGNAL_VALUE )
            continue;    
        double a = point_A[i]-point_B[i];
        sum += a*a;
        num_features_matches++;
    }
    sum =  sum / ((num_features_matches * num_features_matches) + num_features_matches / 4.0) ;
    float res = static_cast<float>(sqrt(sum));
    return res;
}

int KNN_classifier(std::vector<std::vector<float>> &vec_X_train, std::vector<int> &vec_Y_train,
                   int k, std::vector<float> data_point){
    // Returns y_point.

    // Calculate the distance between data_point and all points.
    std::vector<std::pair<int, float>> dist_vec {};
    std::pair<int, float> pair_tmp;
    for(size_t i = 0; i < vec_X_train.size(); ++i){   
        pair_tmp = std::make_pair(i, distance(vec_X_train[i], data_point));
        dist_vec.push_back(pair_tmp);
    }

    // Sort dist_vec by distance ascending.
    auto sortRuleLambda = [] (std::pair<int, float> const& s1, std::pair<int, float> const& s2) -> bool
    {
       return s1.second < s2.second;
    };
    
    sort(dist_vec.begin(), dist_vec.end(), sortRuleLambda);

    // Majority vote on the Y target int class.
    const size_t num_classes = vec_class_strings.size();
    for(size_t i = k; i > 0; --i){
        std::vector<int> vec_Y_class_target(num_classes);
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

void print_data_point(std::vector<float> data_point, int y_point){
    Serial.print("data_point: ");
    Serial.println(data_point.size());
    int counter = 0;    
    for(size_t i=0; i < data_point.size(); ++i){
        //cout << data_point[i] << ",";
        Serial.print(data_point[i]);
        Serial.print(",");
        if (counter % 20 == 0) {
           Serial.print("\n");
        }
        counter++;
    }
    //cout << vec_class_strings[y_point] << endl;
    Serial.println( vec_class_strings[y_point].c_str() ); 
}

void evaluate_all_dataset(std::vector<std::vector<float>> &vec_X_train, std::vector<int> &vec_Y_train, int k,
                          std::vector<std::vector<float>> vec_X_dataset, std::vector<int> vec_Y_dataset,
                          int &dataset_len, int &correct_dataset_pred, float &correct_dataset_pred_perc){
    dataset_len = vec_X_dataset.size();
    correct_dataset_pred = 0;
    correct_dataset_pred_perc = 0.0;    
    std::vector<float> data_point;
    for(size_t i = 0; i < vec_X_dataset.size(); ++i){
        std::vector<float> data_point = vec_X_dataset[i];
        int y_point = KNN_classifier(vec_X_train, vec_Y_train, k, data_point);
        if (y_point == vec_Y_dataset[i])
            correct_dataset_pred++;
        else{
            //print_data_point(data_point, y_point);
        }        
    }
    correct_dataset_pred_perc = (static_cast<float>(correct_dataset_pred) / dataset_len) * 100;
}


// Get the keys of a ordered map, not a hashtable, but a tree.
template<class KEY, class VALUE>
std::vector<KEY> getKeys(const std::map<KEY, VALUE>& map)
{
    std::vector<KEY> keys {};
    for (const auto& it : map)
        keys.push_back(it.first);
    return keys;
}

void get_wifi_networks_data_point(std::vector<float>  &data_point){

    std::vector<String> wifi_routers_names {};
    std::vector<float>  wifi_routers_signal {};

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
          /*
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          */
          
            wifi_routers_names.push_back(WiFi.SSID(i));
            wifi_routers_signal.push_back(static_cast<float>( abs(WiFi.RSSI(i)) ));
 
            delay(10);
        }
    }

    // Print on the serial monitor.
    for(size_t i = 0; i < wifi_routers_names.size(); ++i ){
      String router_name    = wifi_routers_names[i];
      float signal_strength = wifi_routers_signal[i];
        
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(router_name);
      Serial.print(" (");
      Serial.print(signal_strength);
      Serial.println(")");
      //Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
    }

    data_point.clear();

    for(size_t i = 0; i < vec_target_table_Y.size(); ++i){
        std::string router_name_def = vec_target_table_Y[i];
        float signal_strength = C_NOT_SEEN_ROUTER_SIGNAL_VALUE;
        for(size_t j = 0; j < wifi_routers_names.size(); ++j ){
            String router_name = wifi_routers_names[j];
            const char * rn_def = router_name_def.c_str();
            const char * rn     = router_name.c_str();     
            if (strcmp(rn_def, rn) == 0){
                signal_strength = wifi_routers_signal[j];
                break;
            }
        }
        data_point.push_back(signal_strength);
    }
}

void internal_test( ) {
    // Evaluate the correct train set percentage.
    int train_len = 0;
    int correct_train_pred = 0;
    float correct_train_pred_perc = 0.0;    
    evaluate_all_dataset(vec_X_train, vec_Y_train, k,
                         vec_X_train, vec_Y_train, // This are the one being tested.
                         train_len, correct_train_pred, correct_train_pred_perc);

    //cout << "Correct classification in train set \n\ttrain_len: " << train_len 
    //     << "\n\t correct_train_pred: "      << correct_train_pred
    //     << "\n\t correct_train_pred_perc: " << correct_train_pred_perc << endl;
    
    Serial.print("Correct classification in train set \n\ttrain_len: ");
    Serial.print(train_len);
    Serial.print("\n\t correct_train_pred: ");
    Serial.print(correct_train_pred);
    Serial.print("\n\t correct_train_pred_perc: ");
    Serial.println(correct_train_pred_perc);

    // Evaluate the correct test set percentage.
    int test_len = 0;
    int correct_test_pred = 0;
    float correct_test_pred_perc = 0.0;    
    evaluate_all_dataset(vec_X_train, vec_Y_train, k,
                         vec_X_test, vec_Y_test,  // This are the one being tested.
                         test_len, correct_test_pred, correct_test_pred_perc);

    //cout << "Correct classification in test set \n\ttest_len: " << test_len 
    //     << "\n\t correct_test_pred: "      << correct_test_pred
    //     << "\n\t correct_test_pred_perc: " << correct_test_pred_perc << endl;

    Serial.print("Correct classification in test set \n\ttest_len: ");
    Serial.print(test_len);
    Serial.print("\n\t correct_test_pred: ");
    Serial.print(correct_test_pred);
    Serial.print("\n\t correct_test_pred_perc: ");
    Serial.println(correct_test_pred_perc);
}

void setup()
{
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Indoor localization with KNN K-Nearest-Neighbors in C++\n\n");

    // Make a vector of the keys of the ordered map, so you can lookup the
    // name of the room that you are from it's int class id.
    vec_class_strings = getKeys(map_x_router_name_to_index);

    Serial.println("Setup done");
}

void loop()
{
  //  for(std::string my_classif_class : vec_class_strings){
  //    Serial.print(my_classif_class.c_str());
  //    Serial.print(" "); 
  //  }
    Serial.println("\n");

    Serial.println("scan start");

    // ARDUINO code.

    // Sensor code aquisition of data_point.
    std::vector<float> sensor_data_point {};
    Serial.print("sensor data_point _1: ");
    Serial.println(sensor_data_point.size());
    
    get_wifi_networks_data_point(sensor_data_point);
    Serial.print("sensor data_point _2: ");
    Serial.println(sensor_data_point.size());

    int sensor_y_point = KNN_classifier(vec_X_train,vec_Y_train,
                                        k, sensor_data_point);
    Serial.print("sensor data_point _3: ");
    Serial.println(sensor_data_point.size());

    
    Serial.print("Localization: ");
    Serial.println(vec_class_strings[sensor_y_point].c_str());
    print_data_point(sensor_data_point, sensor_y_point);

    Serial.println("");

    internal_test( );

    // Wait a bit before scanning again
    delay(2000);
}
