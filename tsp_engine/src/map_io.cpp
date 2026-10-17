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
                int value = std::stoi(element, &used);

                if (used == element.size()){
                    row.push_back(value);
                }
                else {
                    throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+" is of wrong type. Matrix must only contain whole numbers.");
                }
            }
            catch (const std::out_of_range&) {
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is too large for int assignment.");
                
            }
            catch (const std::invalid_argument&) {
                throw std::runtime_error("Input value of '" +element
                                            + "' at line "+std::to_string(line_count)+ " is of wrong type. Matrix must only contain whole numbers.");
                
            }
            
        }

        dist_matrix.push_back(row);

    }

    //check it is square
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

    return dist_matrix;
}