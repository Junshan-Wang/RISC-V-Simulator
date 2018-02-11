#include <stdio.h>

int main() {  
    int aMatrix[100][100];
    int bMatrix[100][100];
    int product[100][100];

    for (int row = 0; row < 100; row++) {
        for (int col = 0; col < 100; col++) {
            for (int inner = 0; inner < 100; inner++) {
                product[row][col] += aMatrix[row][inner] * bMatrix[inner][col];
            }
        }
    }
    return 0;
} 
