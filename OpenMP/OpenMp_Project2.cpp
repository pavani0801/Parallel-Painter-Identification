#include <omp.h>
#include <iostream>
#include <fstream>

// Custom implementation of atoi (ASCII to integer)
int myAtoi(const char* str) {
    int res = 0;
    int i = 0;
    int sign = 1;

    while (str[i] == ' ' || str[i] == '\t') {
        i++;
    }

    if (str[i] == '-') {
        sign = -1;
        i++;
    }
    else if (str[i] == '+') {
        i++;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return sign * res;
}

// Convert a character to its binary representation
std::string charToBinary(char ch) {
    std::string bin;
    for (int bit = 7; bit >= 0; bit--) {
        bin.push_back(((ch >> bit) & 1) ? '1' : '0');
    }
    return bin;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
            << " <input_file> <output_file> <rows_painting1> [<rows_painting2> ...]"
            << std::endl;
        return 1;
    }

    const char* inputFile = "input1.txt";
    const char* outputFile = "output1.txt";

    int numPaintings = argc - 3;

    // Allocate an array for the row counts.
    int* paintingRows = new int[numPaintings];
    for (int i = 0; i < numPaintings; i++) {
        paintingRows[i] = myAtoi(argv[i + 3]);
    }

    std::ifstream fin(inputFile);
    if (!fin) {
        std::cout << "Error opening input file: " << inputFile << std::endl;
        delete[] paintingRows;
        return 1;
    }

    // Array to hold the detected painter name for each painting.
    const char** painters = new const char* [numPaintings];

    // Process each painting one by one.
    for (int p = 0; p < numPaintings; p++) {
        long long onesSum = 0;
        int rows = paintingRows[p];

        // Allocate an array to store the painting lines.
        char** paintingLines = new char* [rows];
        for (int i = 0; i < rows; i++) {
            paintingLines[i] = new char[10240];  // Max line size 

            // Manually read a line from the file into paintingLines[i]
            if (!fin.getline(paintingLines[i], 10240)) {
                paintingLines[i][0] = '\0';  // Fallback on unexpected EOF
            }
        }

        // Use OpenMP to sum the number of '1's over all rows in this painting.
        long long localSum = 0;
#pragma omp parallel for reduction(+:localSum)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; paintingLines[i][j] != '\0'; j++) {
                if (paintingLines[i][j] == '1') {
                    localSum++;
                }
            }
        }
        onesSum = localSum;

        // For demonstration, choose the painter based on the sum of ones.
        if (onesSum % 3 == 0)
            painters[p] = "Leonardo da Vinci";
        else if (onesSum % 3 == 1)
            painters[p] = "Vincent van Gogh";
        else
            painters[p] = "Claude Monet";  // Adding Monet

        for (int i = 0; i < rows; i++) {
            delete[] paintingLines[i];
        }
        delete[] paintingLines;
    }

    fin.close();

    std::ofstream fout(outputFile);
    if (!fout) {
        std::cout << "Error opening output file: " << outputFile << std::endl;
        delete[] paintingRows;
        delete[] painters;
        return 1;
    }

    for (int p = 0; p < numPaintings; p++) {
        const char* name = painters[p];

        // First, output the painter's name as text.
        fout << name << " ";

        // Then, output the binary representation of the name.
        while (*name) {
            fout << charToBinary(*name) << ' ';
            name++;
        }

        fout << std::endl;
    }

    fout.close();

    delete[] paintingRows;
    delete[] painters;

    return 0;
}
