# Indoor WiFi Localization in ESP32 using Machine Leaning
In this project a ESP32 microcontroller can do self indoor localization in each room inside a building, using only the nearby WiFi routers public RSSI signal strength value.

## Description
This project permits you to localize an ESP32 inside a building using only the signal strength value of near by Wifi routers. <br>
For this project you will need a PC in windows or Linux (but can also work on MAC) and a ESP32 microcontroller. This project can also run on the PC with the collected WiFi data from the ESP32. <br>
I tested it on a 4 rooms case, in which the random probability of tossing a coin and finding the correct answer is 1/16 or 6.25 % of the times. It found correctly 3 of the rooms, almost 100% of the times tested, 4/16 + 4/16 + 4/16 = 75% and the one room that it didn't it had bad coverage of Wifi routers and the routers RSSI that it could find were always changing. The number of routers and the signal strength isn't constant along the day hours. I tested in the day, with the preceding results. The machine learning algorithm used is an implementation of KNN from scratch that I made and it has the following project on my GitHub [KNN__K_Nearest_Neighbors_in_C_Plus_Plus](https://github.com/joaocarvalhoopen/KNN__K_Nearest_Neighbors_in_C_Plus_Plus). <br>
There is a secret sauce in this project code, but i explain how it works, so that you can use the secret sauce in other projects. <br> 

## Components of the project
This project has 6 components. <br>

### 1 - Arduino Wifi_scan ESP32 (C++)
This simple C++ program file is a simple example that comes with Arduino IDE for ESP32, it's under Wifi. <br>
It simply reads the nearby routers RSSI signal strength and prints to the serial port. I read this info from the PC (windows / Linux) and use the data to train my indoor localization model that will run inside the ESP32 with the 6 component of this project. This small program is the only one that wasn't made by me, but it's open source (see Arduino ESP32 license). <br>
This program is run first on the Arduino IDE. <br>
The code [WiFiScan.ino](./Arduino_WiFiScan/WiFiScan.ino) <br>

### 2 - Collector of Wifi train data by serial port from ESP32 and save to files (Python)
This is a Python 3.6 or greater program. It is a simple program that you use in the PC after putting the first (1) component Wifi_scan in the ESP32 with the Arduino IDE. The ESP will be transmitting to the serial from the ESP32 to the PC (Windows/Linux/Mac). This program is very simple and the serial port has to be configured once and filename has to be configured each time you run it, so that the file isn't overlapped. You will generate a file for each room. In my example I used 4 rooms, but can be more. The name of the file has to end with "_data.dat" ex: "room_A_data.dat". <br>
Again you have to configure the serial port once and the name of the training datafile for each room. Monitor the data_file that is generated and stop it when it reaches 20KB. To stop it press on the terminal a CTRL+C. The files are saved inside the directory data_files, and after recording, they should be manually trimmed in the first record and in the last record to an empty line, in order to not cache a info register case that is just part of a register. So there is no incomplete information in the file. <br> 
This program is run second for each room corresponding to each train data file. <br>
To run it do in a terminal "python collect_ESP32_data_save_to_disc.py". <br>
The code [collect_ESP32_data_save_to_disc.py](./collect_ESP32_data_save_to_disc.py) <br>

### 3 - Processing of data files to generate train and test data .h file for C++ for PC target (Python)
The generated .h file from this program is for the PC version 4 - component of the project.
Python program to process of data files and generate train and test .h file for C++ code using the KNN algorithm. The KNN is the K-Nearest-neighbor's machine learning algorithm. <br> 
This program read's each file of the data_files directory corresponding to the data collected from each room WIFI routers signal strength (RSSI). This data will be used to make a indoor self localization system in which a KNN machine learning algorithm will classify which room the microcontroller is. <br>                                                     #
The processing will latter be done in C++ on the ESP32, on the edge or on the PC. But first the code will be designed on the PC to run on the PC (C++) and for that to happen this program in Python will prepare all the data to generate the C++ code for initialization in the form of a .H file that has to compiled in the 4 - component of this project for the PC. <br>
The file generated is "home_train_data.h" . <br>
NOTE: This version that doesn't have Arduino it it's name is to generate the PC version of the .h for the Arduino version use the one that terminates with _arduino.py . <br>
To run it do in a terminal "python collect_ESP32_data_save_to_disc.py". <br>
The code [proc_data_f_gen_train_test_code.py](./proc_data_f_gen_train_test_code.py) <br>

### 4 - Indoor Localization KNN PC (C++)
This is a implementation in C++ on a PC to make the WIFI Indoor Localization with the KNN K-Nearest-Neighbors machine learning algorithm. The following code is only here to facilitate the development of the final algorithms in the ESP32. But it can also be seen as one more example of the use of the my implementation of KNN to work on the PC and how simple it is to use it in C++. This program uses the .h file generated in the Python program, it as to be the PC version, not the Arduino version because that uses a different namespace. <br>
This PC version uses a Euclidean distance and doesn't use the secret sauce custom tunned version. <br>
The unique routers of the .H file correspond to all routers that exist on the train and test dataset. The same doesn't occur in the real life Arduino ESP32 sensor data point, that's why the secret sauce is so important. <br>
To compile this project use a C++ compiler, I used MINGW32 on windows, but you can use any compiler it is C++ standard. <br>
To compile in Linux do in a terminal <br>
 
 ```
     "make clean"
     "make" or "make TARGET=debug"
     "./indoor_localization_KNN_PC.exe"
 ```

 in windows do <br>

 ```
     "mingw32-make clean"
     "mingw32-make" or "mingw32-make TARGET=debug"
     "indoor_localization_KNN_PC.exe"
 ```

I used Visual Studio code for the development and debugging, it works on Windows/Linux/Mac.<br>
The code [indoor_localization_KNN_PC.cpp](./indoor_localization_KNN_PC.cpp) <br>

### 5 - Processing of data files to generate train and test data .h file for C++ for Arduino target (Python)
The generated .h file from this program is for the ESP32 microcontroller Machine Learning on the Edge version 6 - component of the project.
Python program to process of data files and generate train and test .h file for C++ code using the KNN algorithm. The KNN is the K-Nearest-neighbor's machine learning algorithm. <br> 
This program read's each file of the data_files directory corresponding to the data collected from each room WIFI routers signal strength (RSSI). This data will be used to make a indoor self localization system in which a KNN machine learning algorithm will classify which room the microcontroller is. <br>                                                     #
The processing will latter be done in C++ on the ESP32, on the edge. This program in Python will prepare all the data to generate the C++ code for initialization in the form of a .H file that has to be compiled in the 6 - component of this project for the ESP32. <br>
The file generated is "home_train_data.h" . <br>
NOTE: This version is for the Arduino ESP32 to run on the ESP32, the .h file is for the Arduino version, the name of this python program terminates "_arduino.py" . <br>
THIS .H FILE HAS TO BE MANUALLY COPIED TO THE Arduino_Indoor_Localization DIRECTORY AND COMPILED WITH THE ARDUINO IDE <br>
To run it do in a terminal "python proc_data_f_gen_train_test_code_arduino.py". <br>
The code [proc_data_f_gen_train_test_code_arduino.py](./proc_data_f_gen_train_test_code_arduino.py) <br>

### 6 - Arduino Indoor Localization ESP32 (C++)
This is a implementation in C++ for Arduino ESP32 microcontroller that makes Indoor WiFi Localization with the  KNN K-Nearest-Neighbors machine learning algorithm. <br>
This is ML on the Edge with a simple and practical algorithm, implemented from scratch. It writes to the serial monitor of the connected PC the name of the room that the ESP32 is on. The names that appear are the prefix of the datafile name given to each room train file dataset.<br>
This program uses the .h file generated in the Python program, it as to be the Arduino version, not the PC version because that uses a different namespace. Project component 5. <br>
The total number of train data points is limited to 250 because it gave an error while fitting on the ESP32 memory when I tried to make a 330 train dataset. There is the option of tweaking the program memory size of ESP32 so it is possible to use more samples. And the samples currently are 80 % train and 20 % test. In the final code you can make it 100 % test and 0 % train dataset. <br>
This Arduino version doesn't use the Euclidean distance it uses the secret sauce custom tunned version. With it it obtained good results. The unique routers of the .H file correspond to all routers that exist on the train and test dataset. The same doesn't occur in the real life Arduino ESP32 WiFI sensor data point, that's why the secret sauce is so important. <br>
I used Arduino IDE for the development it works Windows/Linux/Mac.<br>
The code [indoor_localization_ESP32.ino](./Arduino_Indor_localizer/indoor_localization_ESP32.ino) <br>


## Dependencies -Things that you will be using
* Linux or windows PC (Mac should also work)
* ESP32 Microcontroller
* Arduino IDE for ESP32
* Pyhton 3.6 or greater
* PySerial
* C++ Compiler, GCC on Linux or MINGW32 on windows, optionally
* Visual Studio Code (editor used in the development)

## Steps to install, collect training data, compile and run the projects code
Please follow the components descriptions. Each one of them has the steps necessary to replicate this project in your home with your specific rooms collected WiFi signal strength and routers names. Be aware that the available routers names and the signal strength changes from day to night, at least that's my experience. So to couple with that the best way is to sample from different periods of the day and then to shuffle the file and sample a percentage of it. In that way it will contain all the cases, or at least representative sample of all the cases. <br>
There are only 8 major steps: <br>
* Compile, upload and run the C++ Arduino code of the WifiScanner to collect the training data. Confirm with Serial monitor in Arduino IDE.
* Close the serial monitor and (instal Pyserial) run the Python script to collect and save the data, configuring once the serial port. At each collection of data in a room give a different data file name ending on "_data.dat", the first part will be name of the room that will be printed on the Arduino IDE Serial Monitor of the final program.
* Edit each the dataset file, and trim in a empty point at the beginning and at the end of a register. Each file has several register/sensed data_points.
* Rum the Python generator A of the .H file for the C++ program in the PC.
* Compile the code of the C++ PC program by following the steps of the terminal in component 4.
* Rum the Python generator B (ends in _arduino.py) of the .H file for the C++ program in the ESP32. Move the .H file to the Arduino directory.
* Compile, upload and run the Arduino code, and like me became amazed by the fact that this simple program really works and makes indoor localization!
* Be a believer in Machine Learning on the Edge, running on microcontrollers.

## The Secret Sauce - A custom made KNN distance metric :-D
The problem to use KNN for this application or purpose is the fact that there are 45 to 60 different router names is all the training set (of all rooms combined), each being a feature in a data_point. Each a different dimension in the feature space. But in in any data data_point there are only between 5 and 24 routers in any moment that we have the signal strength for. This means that when we apply the Euclidean distance ( the squar( n1*n1 + .... nj*nj ) ) we don't have a value for the majority of the dimensions. In Machine Learning this is normally called the missing value problem, but this is an extreme case that the simple mean value can't solve in a easy way. After much debugging and some thinkering I come up with this secrete sauce that makes all work out in the end!
In a previous project (see references in the end) I made an implementation of KNN - K-Nearest-neighbor algorithm, in which we want to find the nearest K (k=5) neighbors, and do a majority classification voting to determine the correct predicted class. So we needed to change the distance metric to be incensive to the number of unseen routers of the training dataset, and the unseen routers of the sensed/sampled data_point. To do this we ignore the distance between component dimensions in the data_point that have a unseen fixed value. For each distance between the point A and B we count the number of valid dimensions, and do sum += n1*n2 and  sum =  sum / ((num_features_matches * num_features_matches) + num_features_matches / 4.0)  this will correct and equalize for the fact that we are comparing different number of elements (num_features_matches * num_features_matches), and then we add factor to penalize when we have more matching routers (if we match 20 routers is better then when we match 5 routers (num_features_matches / 4.0). And with this secret sauce, we passed from not classifying correctly any room to classify correctly almost all rooms! <br>


```
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
```

## References
* [Wikipedia - Indoor positioning system](https://en.wikipedia.org/wiki/Indoor_positioning_system)
* [My GitHub project KNN K-Nearest-Neighbors in C++](https://github.com/joaocarvalhoopen/KNN__K_Nearest_Neighbors_in_C_Plus_Plus) 
* [How kNN algorithm works - Thales Sehn Korting](https://www.youtube.com/watch?v=UqYde-LULfs)
* [My GitHub page - How to learn modern Python](https://github.com/joaocarvalhoopen/How_to_learn_modern_Python)
* [My GitHub page - How to learn modern C++](https://github.com/joaocarvalhoopen/How_to_learn_modern_C_Plus_Plus)

## License
MIT Open Source License

## Have fun!
Best regards, <br>
Joao Nuno Carvalho <br>
