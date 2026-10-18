#include "tsp_engine/map_io.h"
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <istream>


std::vector<std::vector<int>> parse_dist_matrix(std::istream& in){

    std::vector<std::vector<int>> dist_matrix; 
    std::string line; 
    size_t line_count = 0;

    while(std::getline(in, line)){
        line_count++;

        //skip any empty lines that might be present
        if (line.empty()) continue;

        std::vector<int> row;
        std::stringstream ss(line);
        std::string element; 
        size_t used = 0; 

        while(std::getline(ss, element, ',')){

            try{
                //check for trailing control characters
                size_t last = element.find_last_not_of(" \t\r");   
                element.erase(last + 1);     

                int value = std::stoi(element, &used);
                
                //check if value is in range of >= -1
                if (value < -1){
                    throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+" is out of range. Values must be equal to or greater than -1.");
                }
                else if (used == element.size()){
                    // if value in range push to row
                    row.push_back(value);
                }
                else {
                    // if element contains multiple character types of ints non non-ints
                    throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+" is of wrong type. Matrix must only contain whole numbers.");
                }
            }
            catch (const std::out_of_range&) {
                // if value is too large for int assignment
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is too large for int assignment.");
                
            }
            catch (const std::invalid_argument&) {
                // if value contains non-interger values
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is of wrong type. Matrix must only contain whole numbers.");
                
            }
            
        }

        if (row.size() > 50u){
            //if there are too many nodes, throw an error
            throw std::runtime_error("Row "+std::to_string(line_count)+" contains too many nodes. Maximum nodes per row is 50.");
        }

        dist_matrix.push_back(row);

    }

    //check if its a square
    // .size() returns a size_t so keeping the convention to avoid compiler erros
    size_t height = dist_matrix.size();
    for (size_t i = 0; i < height; i++){

        // row i, where i counts from 0 and skips blank lines. 
        // it won't necessarily match line_count
        // so the user could be pointed to the wrong line. 

        if (dist_matrix[i].size() != height){
            
            throw std::runtime_error("Matrix is not square: row " + std::to_string(i) +
                                 " has " + std::to_string(dist_matrix[i].size()) +
                                 " values, expected " + std::to_string(height));
        } 
    }

    //add a check to find if diagonals are marked with with 0's

    //check if matrix is empty
    if (dist_matrix.size() == 0){
        throw std::runtime_error("Matrix contains no values.");
    }

    return dist_matrix;
}