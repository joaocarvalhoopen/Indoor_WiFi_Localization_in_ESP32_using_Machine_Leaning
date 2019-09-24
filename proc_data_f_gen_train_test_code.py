###############################################################################
#                                                                             #
# proc_data_f_gen_train_test_code.py                                          #
#                                                                             #
###############################################################################
# Author:  Joao Nuno Carvalho                                                 #
# Data:    2019.09.22                                                         #
# License: MIT Open Source License                                            #
#                                                                             #
# Description: Python program to process of data files and generate train     #
#              and test .h file for C++ code using the KNN algorithm.         #
#              The KNN is the K-Nearest-Neighbores machine learning algorithm.# 
#              This program read's each file of the data_files directory      #
# corresponding to the data collected from each room WIFI routers signal      #
# strength (RSSI). This data will be used to make a indoor self localization  #
# system in which a KNN machine learning algorithm will classify which room   #
# the microcontroller is.                                                     #
# The processing will be done in C++ on the ESP32, on the edge or on the PC.  #
# But first the code will be designed on the PC to run on the PC (C++) and    #
# for that to happen this program in Python will prepare all the data to      #
# generate the C++ code for initialization in the form of a .H file that has  #
# to compiled in the 5- component of this project for the PC.                 #
# The file generated is "home_train_data.h"                                   #
# NOTE: This version that doesn't have Arduino it it's name is to generate    #
#       the PC version of the .h for the Arduino version use the one that     #
#       terminates with _arduino.py .                                         #
###############################################################################  

#####
# What this program does:

# List all files in directory data_files.

# Read all files line by line and create representation in memory, maybe a class.

# Determine all the networks router names that are present / classes names for the classifier.

# Take care for writing all routers that are not present as 0.

# Not done
# Scale the outputs between 0 and 1 from -0 to -95 or -100 [Teste the data].

# Mix the data.

# Split the data between Train and Test data.

# Write the data to .h files for declaring and initializing the following variables:
#
# // Lookup table of routers, Y int target to Y classifier Class name target.
# // Usage pattern "vec_target_table_Y[Y_int]"   
# vector<string>        vec_target_table_Y;
# // HashTable -> X router_name (string) to feature_index (size_t).
# map<string,int>       map_x_router_name_to_index; 
# vector<vector<float>> vec_X_train;
# vector<int>           vec_Y_train;
# vector<vector<float>> vec_X_test;
# vector<int>           vec_Y_test;



import os
import random

C_FILE_SUFIX = '_data.dat'
# Has to be the same value that in the component 6 of the project.
C_NOT_SEEN_ROUTER_SIGNAL_VALUE = 0    
dot_H_code_filename = "home_train_data.h"

# List's all filenames in a directory.
def files(path):  
    for file in os.listdir(path):
        if os.path.isfile(os.path.join(path, file)):
            yield file

def parse_line_router_name_and_signal_strength(line):
    pos = line.find(":")
    num_router_in_sample = line[ : pos]
    num_router_in_sample = int(num_router_in_sample)
    pos_router_begin = pos + 2
    pos = line.find("(")
    pos_router_end = pos - 1
    router_name = line[pos_router_begin : pos_router_end]
    signal_strength_begin = pos + 1
    pos_end = line.find(")")
    signal_strength = line[signal_strength_begin : pos_end ]
    signal_strength = int(signal_strength)
    return (num_router_in_sample, router_name, signal_strength) 

def  reads_file_to_memory(path, file, list_all_router_names):   
    data = None
    num_sample_lists = 0
    num_duplicate_sample_lists = 0
    sample_list = []
    flag_in_sample = False
    sample = None
    with open(path + file, "r") as f:
        data = f.readlines()
        for line in data:
            if (len(line) != 0) and (":" in line):
                if flag_in_sample == False:
                    flag_in_sample = True
                    sample = []
                res = parse_line_router_name_and_signal_strength(line)
                router_name = res[1]
                list_all_router_names.append(router_name)        
                sample.append(res)
            else:
                if flag_in_sample == True:
                    flag_in_sample = False
                    if sample not in sample_list: 
                        sample_list.append(sample)
                        num_sample_lists += 1
                    else:
                        num_duplicate_sample_lists += 1
                        print("#################### duplicate case!")
                    sample = None
    id = file
    if id.endswith(C_FILE_SUFIX):
        id = id[ : -len(C_FILE_SUFIX)]
    return [id, sample_list, num_sample_lists, num_duplicate_sample_lists]


def create_all_samples_dataset(list_unique_router_names, list_of_y_target, 
                               data_files_in_memory):
    all_samples_data_set = []
    for data_file in data_files_in_memory:
        data_y_target_name = data_file[0]   # id
        # Find the Y target int value of the classifier class name.
        data_y_target_int = [target[1] for target in list_of_y_target if target[0] == data_y_target_name][0]
        list_of_all_samples_lists_in_file = data_file[1] 
        for sample_list in list_of_all_samples_lists_in_file: 
            # Now we iterate over all router_names in predefined order that is on list_unique_router_names.
            # And find the correct names in the sample_list.      
            x_data = []
            for pos, router_name_def in enumerate(list_unique_router_names):
                #print(pos, router_name)
                value = C_NOT_SEEN_ROUTER_SIGNAL_VALUE
                for _, router_name, signal_strength in sample_list:
                    if router_name_def == router_name:
                        value = abs(signal_strength)
                        break;
                x_data.append(value)
            
            # Add the case to the list.
            all_samples_data_set.append([data_y_target_name, data_y_target_int, x_data])

    return all_samples_data_set

def split_train_test(all_samples_data_set, percentage_of_train_cases):
    X_train = []
    Y_train = []
    X_test  = []
    Y_test  = []
    total_num_cases = len(all_samples_data_set)
    devision_point = int((total_num_cases * percentage_of_train_cases) / 100.0)
    index = 0
    for case in all_samples_data_set:
        x_values = case[2]
        y_value = case[1]
        if index < devision_point:
            X_train.append(x_values)
            Y_train.append(y_value)
        else:
            X_test.append(x_values)
            Y_test.append(y_value)
        index += 1
    return (X_train, Y_train, X_test, Y_test)

#    dot_H_code_filename = "home_train_data.h"

def write_header(dot_H_code_filename):
    """
#pragma once


#ifndef __HOME_TRAIN_DATA_H_
#define __HOME_TRAIN_DATA_H_

#include <string>
#include <vector>
#include <map>

using namespace std;


"""

    filename = dot_H_code_filename.replace(".", "_")
    filename = filename.upper()
    filename = "__" + filename + "_"

    first_str = """
#pragma once


#ifndef """

    second_str = "#define "

    third_str = """
    
#include <string>
#include <vector>
#include <map>

using namespace std;

"""

    d_header= "".join((first_str,
                    filename,
                    "\n",
                    second_str,
                    filename,
                    "\n",
                    third_str) )
    return d_header

def write_footer(dot_H_code_filename):
    d_footer = "\n\n#endif\n"

    return d_footer

def write_target_table_vector(var_name, target_table_names_in_order):
    """
// Lookup table of routers, Y int target to Y classifier Class name target.
// Usage pattern "vec_target_table_Y[Y_int]"   
vector<string> vec_target_table_Y 
{ 
    "router_name_0",
    "router_name_1",
    "router_name_3" 
};

"""

    list_str = []
    str_0 = """
// Lookup table of routers, Y int target to Y classifier Class name target.
// Usage pattern "vec_target_table_Y[Y_int]"   
vector<string> vec_target_table_Y 
{
"""
    list_str.append(str_0)

    num_elem = len(target_table_names_in_order)
    elem_index = 0
    # print(target_table_names_in_order)    
    for router_name in target_table_names_in_order:
        str_tmp = None
        if elem_index != (num_elem - 1):
            str_tmp = "".join( ('    "', router_name, '",\n') )  # Comma
        else:
            str_tmp = "".join( ('    "', router_name, '"\n') )   # No comma 
        list_str.append( str_tmp)
        elem_index += 1
    
    str_1 = "};\n\n"
    list_str.append(str_1)

    d_target_table = "".join(list_str)
    return d_target_table

def write_router_name_hashtable(var_name, router_names_feature_correspond_list):
    """
// HashTable -> X router_name (string) to feature_index (size_t).
map<string,int> map_x_router_name_to_index 
{
    {"router_name_0", 0},
    {"router_name_1", 1},
    {"router_name_2", 2} 
};

"""

    list_str = []
    str_0 = """
// HashTable -> X router_name (string) to feature_index (size_t).
map<string,int> map_x_router_name_to_index 
{
"""
    list_str.append(str_0)

    num_elem = len(router_names_feature_correspond_list)
    elem_index = 0
    # print(router_names_feature_correspond_list)    
    for router_name, int_index in router_names_feature_correspond_list:
        str_tmp = None

        if elem_index != (num_elem - 1):
            str_tmp = "".join( ('    {"', router_name, '", ', str(int_index), '},\n') )  # Comma
        else:
            str_tmp = "".join( ('    {"', router_name, '", ', str(int_index), '}\n') )   # No comma 
        list_str.append( str_tmp)
        elem_index += 1
    
    str_1 = "};\n\n"
    list_str.append(str_1)

    d_router_name_hashtable = "".join(list_str)
    return d_router_name_hashtable

# Called for train and for test dataset's.
def write_x_vector(var_name, X_dataset):
    """
vector<vector<float>> vec_X_train [vec_X_test] 
{
    {11,120,31,40,60,50,70},
    {12,120,32,40,60,50,70},
    {13,120,33,40,60,50,70},
    //...
    {14,120,34,40,60,50,70},
    {15,120,35,40,60,50,70}
};

"""

    list_str = []
    str_0 = "\nvector<vector<float>> " + var_name + "\n{\n"
    list_str.append(str_0)

    num_case = len(X_dataset)
    case_index = 0
    for x_case in X_dataset:
        str_tmp = "    {"
        list_str.append(str_tmp)
        num_elem = len(x_case)
        elem_index = 0
        for x_value in x_case:
            str_tmp = None
            if elem_index != (num_elem - 1):
                str_tmp = "".join( (str(x_value), ',') )  # Comma
            else:
                str_tmp = str(x_value)   # No comma 
            list_str.append(str_tmp)
            elem_index += 1
        str_tmp = None
        if case_index != (num_case - 1):
            str_tmp = "},\n" # Comma
        else:
            str_tmp = "}\n"   # No comma 
        list_str.append(str_tmp)
        case_index += 1
    
    str_1 = "};\n\n"
    list_str.append(str_1)

    d_x_dataset = "".join(list_str)
    return d_x_dataset

# Called for train and for test dataset's.
def write_y_vector(var_name, Y_dataset):
    """
vector<int> vec_Y_train  [vec_Y_test]
{
    0,1,2,0,1,
    //...
    2,0,1,2
};

"""    

    list_str = []
    str_0 = "\nvector<int> " + var_name + "\n{\n"
    list_str.append(str_0)

    num_elem = len(Y_dataset)
    elem_index = 0
    for y_value in Y_dataset:
        str_tmp = None
        if elem_index != (num_elem - 1):
            str_tmp = "".join( ("    ", str(y_value), ',\n') )  # Comma
        else:
            str_tmp = "".join( ("    ", str(y_value), '\n') )   # No comma 
        list_str.append( str_tmp)
        elem_index += 1
    
    str_1 = "};\n\n"
    list_str.append(str_1)

    d_y_dataset = "".join(list_str)
    return d_y_dataset

def write_dot_H_file_for_C_plus_plus(dot_H_code_filename,
                                     list_unique_router_names,
                                     list_of_y_target,
                                     X_train, Y_train, X_test, Y_test):

    d_header = write_header(dot_H_code_filename)

    # Y int target to Y classifier Class name target.
    target_table_names_in_order = list_unique_router_names
    var_name = "vec_target_table_Y"
    d_target_table = write_target_table_vector(var_name, target_table_names_in_order)

    # HashTable -> X router_name (string) to feature_index (size_t).
    router_names_feature_correspond_list = list_of_y_target
    var_name = "map_x_router_name_to_index"
    d_router_name_hashtable = write_router_name_hashtable(var_name, router_names_feature_correspond_list)

    var_name = "vec_X_train"
    d_x_train = write_x_vector(var_name, X_train)

    var_name = "vec_Y_train"
    d_y_train = write_y_vector(var_name, Y_train)

    var_name = "vec_X_test"
    d_x_test = write_x_vector(var_name, X_test)

    var_name = "vec_Y_test"
    d_y_test = write_y_vector(var_name, Y_test)


    d_footer = write_footer(dot_H_code_filename)
    
    # Write file to disc.
    with open(dot_H_code_filename, "w") as f:
        f.write(d_header)

        f.write(d_target_table)
        f.write(d_router_name_hashtable)

        f.write(d_x_train)
        f.write(d_y_train)
        f.write(d_x_test)
        f.write(d_y_test)

        f.write(d_footer)
    
def main():
    # List all files in directory data_files.
    data_files_path = ".//data_files_tmp//"
    data_files_path = ".//data_files//"
    data_files = [file for file in files(data_files_path)]  
    # print(data_files)

    # Read all files line by line and create representation in memory, maybe a class.
    list_all_router_names = []
    data_files_in_memory = [reads_file_to_memory(data_files_path, file, list_all_router_names) 
                                for file in data_files]
    # print(data_files_in_memory)


    #print(list_all_router_names)
    list_unique_router_names = list(set(list_all_router_names))
    list_unique_router_names = sorted(list_unique_router_names)
    # print(list_unique_router_names)
    print("sorted router_names len: ", str(len(list_unique_router_names)))
    list_all_router_names = None

    # res = parse_line_router_name_and_signal_strength('12: MEJ-BB870J (-92)*\n')
    # print(res)

    # res = parse_line_router_name_and_signal_strength('6: Frases e candeeiros (-89)*\n')
    # print(res)

    # Remove equal data points.

    # Determine all the networks router names that are present / classes names for the classifier.

    # Take care for writing all routers that are not present as 0.0.

    # NOT DONE!
    # Scale the outputs between 0 and 1 from -0 to -95 or -100 [Teste the data].

    # Join all the data into a list in the correct positions with the first position Y target value.
    #
    #  y_target_0 <= ['cozinha_data.dat' = 0, 'quartoA_data.dat' = 1, 'quartoB_data.dat' = 2, 'quartoC_data.dat'= 3, 'sala_data.dat'= 4]
    #
    # [ [y_target_0,  [120, 30, 40, 50, 60....59]]
    #   [y_target_0,  [120, 20, 50, 40, 120....59]]
    #   ...
    #   [y_target_4,  [40, 50, 60, 30, 120....59]]
    # ]
    list_of_y_target = [(target_name[0], target_int) 
                               for target_name, target_int 
                               in zip(data_files_in_memory, range( 0, len(data_files_in_memory)))]
    print(list_of_y_target)
    all_samples_data_set = create_all_samples_dataset(list_unique_router_names, list_of_y_target,
                                                      data_files_in_memory) 
    # print(all_samples_data_set)

    # Mix the data samples so we can split them next into train and test dataset's.
    random.seed(42) # We seed so that it gives always the some result.
    random.shuffle(all_samples_data_set)
    # print(all_samples_data_set)
    print("len  all_samples_data_set: ", len(all_samples_data_set))

    # Split the data between Train and Test data.
    percentage_of_train_cases = 80.0
    percentage_of_test_cases  = 100.0 - percentage_of_train_cases
    X_train, Y_train, X_test, Y_test = split_train_test(all_samples_data_set, percentage_of_train_cases)

    print("len X_train: ", len(X_train), " Y_train: ", len(Y_train) )
    print("len X_test: ",  len(X_test),  " Y_test: ",  len(Y_test) )


    # Write the data to .h files for declaring and initializing the following variables:
    #
    # // Lookup table of routers, Y int target to Y classifier Class name target.
    # // Usage pattern "vec_target_table_Y[Y_int]"   
    # vector<string>        vec_target_table_Y;
    # // HashTable -> X router_name (string) to feature_index (size_t).
    # map<string,int>       map_x_router_name_to_index; 
    # vector<vector<float>> vec_X_train;
    # vector<int>           vec_Y_train;
    # vector<vector<float>> vec_X_test;
    # vector<int>           vec_Y_test;

    # dot_H_code_filename = "home_train_data.h"
    write_dot_H_file_for_C_plus_plus(dot_H_code_filename,
                                     list_unique_router_names,
                                     list_of_y_target,
                                     X_train, Y_train, X_test, Y_test)

    print("...end\n")

if __name__ == "__main__":
    main()


