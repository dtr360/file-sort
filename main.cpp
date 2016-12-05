//
//  main.cpp
//  Sorter
//
//  Created by Daniel Rencricca on 12/22/15.
//  Copyright © 2015 Daniel Rencricca. All rights reserved.
//
//  To inlcude arguments go to Product > Scheme > Edit Scheme > Arguments

#include "sortroutines.cpp"
#include <string>
//#include <iostream> do not include this else link errors

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 1) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
        
        // inform the user of how to use the program
        std::cout << "Usage is -i <infile> -o <outfile> -c1 <sort column 1> -c2 <sort column 2> -c3 <sort column 3>\n";         std::cin.get();
        exit(0);
    }
    else // we got enough parameters...
    {
        string  inFile, filePath, outFile;
        int     col1=0, col2=0, col3=0; // columns in file to sort in correct order

        for (int i = 1; i < argc; i++) // Iterate over argv[] to get the parameters.
        {                              // Start at 1 because we don't need to know the
                                       // path of the program, stored in argv[0]
            if (i + 1 != argc) // check that we haven't finished parsing already
            {
                if (strncmp(argv[i], "-i", 2) == 0) // then next argument is the input filename
                {
                    i++;
                    inFile = argv[i];
                }
                else if (strncmp(argv[i], "-p", 2) == 0)
                {
                    i++;
                    filePath = argv[i];
                }
                else if (strncmp(argv[i], "-o", 2) == 0)
                {
                    i++;
                    outFile = argv[i];
                }
                else if (strncmp(argv[i], "-c1", 3) == 0)
                {
                    i++;
                    col1 = stoi(argv[i]);
                }
                else if (strncmp(argv[i], "-c2", 3) == 0)
                {
                    i++;
                    col2 = stoi(argv[i]);
                }
                else if (strncmp(argv[i], "-c3", 3) == 0)
                {
                    i++;
                    col3 = stoi(argv[i]);
                }
                else
                {
                    std::cout << "Not enough or invalid arguments, please try again.\n";
                    exit(0);
                }
            }
            
            #ifdef _DEBUG
            std::cout << argv[i] << "  ";
            #endif
            
        } // for loop
        
        if ((col1 ==0 and col2 == 0 and col3 == 0) or (col1 <0 or col2 < 0 or col3<0))
        {
            std::cout << "Invalid arguments, please try again.\n";
            exit(0);
        }
        
        cout << "Running program...\n";
        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
        printf("Current dir: %s\n", dir);
        
        SortRoutines sorter(inFile, outFile, col1, col2, col3);
        sorter.SortFile();
    }
    return 0;
}

