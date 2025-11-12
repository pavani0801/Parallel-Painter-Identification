#include <mpi.h>
#include <iostream>
#include <fstream>

using namespace std;

const int MAX_ARTWORKS = 10;
const int MAX_TITLE_LEN = 128;
const int MAX_BUFFER_LEN = 1024;

// Convert ASCII C‑string into 8‑bit binary
void convertAsciiToBinary(const char* asciiStr, char* binStr) {
    while (*asciiStr) {
        unsigned char c = *asciiStr++;
        for (int b = 7; b >= 0; --b) {
            *binStr++ = ((c >> b) & 1) ? '1' : '0';
        }
    }
    *binStr = '\0';
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // number of artworks
    int numArtworks = 0;
    // titles buffer
    char artworkTitles[MAX_ARTWORKS][MAX_TITLE_LEN];

    // zero‑initialize titles
    for (int i = 0; i < MAX_ARTWORKS; ++i)
        for (int j = 0; j < MAX_TITLE_LEN; ++j)
            artworkTitles[i][j] = '\0';

    if (rank == 0) {
        if (argc < 2) {
            cerr << "Usage: " << argv[0] << " <inputfile>\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        ifstream in(argv[1]);
        if (!in) {
            cerr << "Error: cannot open " << argv[1] << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // count lines using C‑style buffer
        int lines = 0;
        char buffer[MAX_BUFFER_LEN];
        while (in.getline(buffer, MAX_BUFFER_LEN))
            ++lines;
        in.close();

        // one artwork per 1000 lines
        numArtworks = lines / 1000 + (lines % 1000 ? 1 : 0);
        if (numArtworks > MAX_ARTWORKS)
            numArtworks = MAX_ARTWORKS;

        // set titles manually
        {
            const char* t = "Pablo Picasso";
            int p = 0;
            while (t[p] && p < MAX_TITLE_LEN - 1) {
                artworkTitles[0][p] = t[p];
                ++p;
            }
        }
        if (numArtworks > 1) {
            const char* t = "Frida Kahlo";
            int p = 0;
            while (t[p] && p < MAX_TITLE_LEN - 1) {
                artworkTitles[1][p] = t[p];
                ++p;
            }
        }
        for (int i = 2; i < numArtworks; ++i) {
            const char* t = "Unknown Artist";
            int p = 0;
            while (t[p] && p < MAX_TITLE_LEN - 1) {
                artworkTitles[i][p] = t[p];
                ++p;
            }
        }
    }

    // broadcast count and entire titles block
    MPI_Bcast(&numArtworks, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(artworkTitles, MAX_ARTWORKS * MAX_TITLE_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

    // each rank converts its share
    char binaryBuf[MAX_TITLE_LEN * 8 + 1];
    for (int idx = rank; idx < numArtworks; idx += size) {
        convertAsciiToBinary(artworkTitles[idx], binaryBuf);
        cout << "Process " << rank
            << " - Artwork " << (idx + 1)
            << " Title (binary): " << binaryBuf << "\n";
    }

    // rank 0 writes all to file
    if (rank == 0) {
        ofstream out("output_mpi.txt");
        if (!out) {
            cerr << "Error: cannot create output_mpi.txt\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < numArtworks; ++i) {
            convertAsciiToBinary(artworkTitles[i], binaryBuf);
            out << "Artwork " << (i + 1)
                << " Title (binary): " << binaryBuf << "\n";
        }
        cout << "Output saved in output_mpi.txt\n";
    }

    MPI_Finalize();
    return 0;
}