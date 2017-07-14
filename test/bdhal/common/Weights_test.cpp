#include "Weights.h"

#include <iostream>
#include <cstdlib>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

template<typename T>
T* buildMatrix(uint32_t numRows, uint32_t numColumns) {
    T* newMatrix = (T*) calloc(numRows*numColumns,sizeof(T));

    return newMatrix;
}

TEST(TESTWeights, testConstructionValidParams) {
    uint32_t numRows = 3;
    uint32_t numColumns = 5;
    uint32_t* newMatrix = buildMatrix<uint32_t>(numRows,numColumns);

    Weights<uint32_t> * newWMatrix = nullptr;

    EXPECT_NO_THROW(newWMatrix = new Weights<uint32_t>(newMatrix,
        numRows, numColumns));
    
    delete newWMatrix;
}

TEST(TESTWeights, testConstructionInValidParams) {
    uint32_t numRows = 3;
    uint32_t numColumns = 5;
    uint32_t* newMatrix = buildMatrix<uint32_t>(numRows,numColumns);

    Weights<uint32_t> * newWMatrix = nullptr;

    EXPECT_THROW(newWMatrix = new Weights<uint32_t>(nullptr,
        numRows, numColumns),std::logic_error);

    delete newWMatrix;

    EXPECT_THROW(newWMatrix = new Weights<uint32_t>(newMatrix,
        0, numColumns), std::out_of_range);
    
    delete newWMatrix;

    EXPECT_THROW(newWMatrix = new Weights<uint32_t>(newMatrix,
        numRows, 0), std::out_of_range);
    
    delete newWMatrix;
}

TEST(TESTWeights, testConstructionGetRowsColumns) {
    uint32_t numRows = 5;
    uint32_t numColumns = 3;
    uint32_t* newMatrix = buildMatrix<uint32_t>(numRows,numColumns);

    for (uint32_t i = 0; i < numRows; i++) {
        for (uint32_t j = 0; j < numRows; j++) {
            newMatrix[i*numColumns + j] = i+j;
        }
    }

    Weights<uint32_t> * newWMatrix = new Weights<uint32_t>(newMatrix,
        numRows, numColumns);

    EXPECT_EQ(newWMatrix->GetNumRows(),numRows);
    EXPECT_EQ(newWMatrix->GetNumColumns(),numColumns);

}

TEST(TESTWeights, testConstructionGetElements) {
    uint32_t numRows = 5;
    uint32_t numColumns = 3;
    uint32_t* newMatrix = buildMatrix<uint32_t>(numRows,numColumns);

    for (uint32_t i = 0; i < numRows; i++) {
        for (uint32_t j = 0; j < numRows; j++) {
            newMatrix[i*numColumns + j] = i+j;
        }
    }

    Weights<uint32_t> * newWMatrix = new Weights<uint32_t>(newMatrix,
        numRows, numColumns);

    for (uint32_t i = 0; i < numRows; i++) {
        for (uint32_t j = 0; j < numRows; j++) {
            EXPECT_EQ(newWMatrix->GetElement(i,j), newMatrix[i*numColumns + j]);
        }
    }

    delete newWMatrix;
}

} // namespace bdhal
} // namespace pystorm
