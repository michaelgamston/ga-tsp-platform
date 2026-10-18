#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <string>
#include "tsp_engine/map_io.h"

// helper: parse a matrix straight from a string
static std::vector<std::vector<int>> parse(const std::string& text){
    std::istringstream input(text);
    return parse_dist_matrix(input);
}

// helper: build a valid n x n matrix as CSV text (0 on the diagonal, 1 elsewhere)
static std::string make_matrix(size_t n){
    std::string text;
    for (size_t row = 0; row < n; row++){
        for (size_t col = 0; col < n; col++){
            text += (row == col) ? "0" : "1";
            if (col < n - 1) text += ",";
        }
        text += "\n";
    }
    return text;
}

// fail cases

    // test incorrect character types

TEST(ParseDistMatrix, ThrowsOnLetters) {
    EXPECT_THROW(parse("0,1,2\nabc,0,3\n2,3,0\n"), std::runtime_error);
}

TEST(ParseDistMatrix, ThrowsOnNumberFollowedByLetters) {
    EXPECT_THROW(parse("0,12abc,2\n12,0,3\n2,3,0\n"), std::runtime_error);
}

    // test for numbers seperated by a space 

TEST(ParseDistMatrix, ThrowsOnNumbersSeperatedBySpace){
    EXPECT_THROW(parse("0,1 2,2\n1,0,3\n2,3,0\n"), std::runtime_error);

}

    // test non-square matrix

TEST(ParseDistMatrix, ThrowsOnRowTooShort) {
    // 4x4 with one row of 3, so the short row is still within the 3 node minimum
    EXPECT_THROW(parse("0,1,2,3\n1,0,4,5\n2,4,0\n3,5,6,0\n"), std::runtime_error);
}

TEST(ParseDistMatrix, ThrowsOnMoreRowsThanColumns) {
    EXPECT_THROW(parse("0,1,2\n1,0,3\n2,3,0\n4,5,6\n"), std::runtime_error);
}

    // test for intergers that are too large

TEST(ParseDistMatrix, ThrowsOnIntegerTooLarge) {
    EXPECT_THROW(parse("0,99999999999,2\n99999999999,0,3\n2,3,0\n"), std::runtime_error);
}

    // test for flaoting point numbers

TEST(ParseDistMatrix, ThrowsOnFloatingPoint) {
    EXPECT_THROW(parse("0,12.5,2\n12.5,0,3\n2,3,0\n"), std::runtime_error);
}


    // test for empty cells

TEST(ParseDistMatrix, ThrowsOnEmptyCell) {
    EXPECT_THROW(parse("0,,2\n1,0,3\n2,3,0\n"), std::runtime_error);
}

    // test for completely empty inputs (a 0x0 matrix)

TEST(ParseDistMatrix, ThrowsOnCompletelyEmptyInput) {
    EXPECT_THROW(parse(""), std::runtime_error);
}

TEST(ParseDistMatrix, ThrowsOnOnlyBlankLines) {
    EXPECT_THROW(parse("\n\n\n"), std::runtime_error);
}

    // test for negative inputs that are no -1

TEST(ParseDistMatrix, ThrowsOnNegativeOtherThanMinusOne) {
    EXPECT_THROW(parse("0,-5,2\n-5,0,3\n2,3,0\n"), std::runtime_error);
}

    // test for maps that are too big 

TEST(ParseDistMatrix, ThrowsOnMapLargerThanMaximum) {
    EXPECT_THROW(parse(make_matrix(51)), std::runtime_error);
}

TEST(ParseDistMatrix, AcceptsMapAtMaximumSize) {
    EXPECT_NO_THROW(parse(make_matrix(50)));
}

// test for maps that are too small 

TEST(ParseDistMatrix, ThrowsOnMapSmallerThanMinimum) {
    EXPECT_THROW(parse(make_matrix(2)), std::runtime_error);
}

TEST(ParseDistMatrix, AcceptsMapAtMinimumSize) {
    EXPECT_NO_THROW(parse(make_matrix(3)));
}


// pass cases

    // test a sqaure fully connected matrix

TEST(ParseDistMatrix, ParsesValid3x3Matrix) {
    auto matrix = parse("0,1,2\n1,0,3\n2,3,0\n");

    ASSERT_EQ(matrix.size(), 3u);
    for (const auto& row : matrix) ASSERT_EQ(row.size(), 3u);
    EXPECT_EQ(matrix[0][1], 1);
    EXPECT_EQ(matrix[2][1], 3);
}

    // test a sqaure partially connected matrix

TEST(ParseDistMatrix, AcceptsMinusOneForUnconnectedNodes) {
    auto matrix = parse("0,-1,2\n-1,0,3\n2,3,0\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][1], -1);
    EXPECT_EQ(matrix[1][0], -1);
    EXPECT_EQ(matrix[0][2], 2);
}

    // test blank lines are skipped

TEST(ParseDistMatrix, SkipsBlankLines) {
    auto matrix = parse("\n0,1,2\n\n1,0,3\n\n2,3,0\n\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][1], 1);
    EXPECT_EQ(matrix[2][1], 3);
}

    // test for leading blank spaces

TEST(ParseDistMatrix, AcceptsLeadingSpaces) {
    auto matrix = parse("0, 1, 2\n 1,0, 3\n 2, 3,0\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][1], 1);
    EXPECT_EQ(matrix[2][1], 3);
}

    // test for trailing blank spaces

TEST(ParseDistMatrix, AcceptsTrailingSpaces) {
    auto matrix = parse("0,1 ,2 \n1 ,0,3 \n2 ,3 ,0\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][1], 1);
    EXPECT_EQ(matrix[2][1], 3);
}

    // test for trailing commas

TEST(ParseDistMatrix, AcceptsTrailingCommas) {
    auto matrix = parse("0,1,2,\n1,0,3,\n2,3,0,\n");

    ASSERT_EQ(matrix.size(), 3u);
    for (const auto& row : matrix) ASSERT_EQ(row.size(), 3u);
    EXPECT_EQ(matrix[0][1], 1);
    EXPECT_EQ(matrix[2][1], 3);

}

    // test windows line endings accepted

TEST(ParseDistMatrix, AcceptsTrailingLineBreaks) {
    auto matrix = parse("0,1,2\r\n1,0,3\r\n2,3,0\r\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][2], 2);   // last value on each line is the one next to the \r
    EXPECT_EQ(matrix[1][2], 3);

}

    // test trailing tabs accepted

TEST(ParseDistMatrix, AcceptsTrailingTabs) {
    auto matrix = parse("0,1,2\t\n1,0,3\t\n2,3,0\t\n");

    ASSERT_EQ(matrix.size(), 3u);
    EXPECT_EQ(matrix[0][2], 2);   // last value on each line is the one next to the \t
    EXPECT_EQ(matrix[1][2], 3);

}

// final test

    // load each map from maps and check for successfull completion

// maps/ sits two folders above this test file (tests/ -> tsp_engine/ -> repo root)
static std::string maps_dir(){
    std::string here = __FILE__;
    return here.substr(0, here.find_last_of('/')) + "/../../maps/";
}

TEST(ParseDistMatrix, LoadsAllExampleMaps) {
    const std::vector<std::pair<std::string, size_t>> maps = {
        {"map_05.csv", 5}, {"map_08.csv", 8}, {"map_12.csv", 12},
        {"map_16.csv", 16}, {"map_20.csv", 20},
    };

    for (const auto& [name, expected_size] : maps) {
        SCOPED_TRACE(name);   // names the map in any failure message

        std::ifstream file(maps_dir() + name);
        ASSERT_TRUE(file.is_open()) << "Could not open " << maps_dir() + name;

        std::vector<std::vector<int>> matrix;
        ASSERT_NO_THROW(matrix = parse_dist_matrix(file));
        EXPECT_EQ(matrix.size(), expected_size);
    }
}

TEST(ParseDistMatrix, Map05HasExpectedValues) {
    std::ifstream file(maps_dir() + "map_05.csv");
    ASSERT_TRUE(file.is_open());

    auto matrix = parse_dist_matrix(file);

    EXPECT_EQ(matrix[0][1], 86);
    EXPECT_EQ(matrix[1][4], -1);   // the unconnected pair in this map
    EXPECT_EQ(matrix[4][1], -1);
}
