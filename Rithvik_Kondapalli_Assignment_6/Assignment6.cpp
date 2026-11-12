// Name: EECS348 Assignment 6
// Desc.: This program takes Sudoku puzzles and solves them.
// Input: Takes the puzzle files named puzzle1.txt through puzzle5.txt.
// Output: solutions.txt with solutions for all puzzles.
// Sources: Miakah Komardley, ChatGPT, Gemini, Claude
// Revised by: Rithvik Kondapalli
// Revision date: November 12, 2026

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Sudoku {
private:

    static constexpr int SIZE = 9;
    static constexpr int BOX_SIZE = 3;
    static constexpr int CELL_COUNT = SIZE * SIZE;

    // Inherited: 0 indicates an empty cell; other values are Sudoku digits.
    unsigned char grid[SIZE][SIZE] = {};
    int solutionCount = 0;       
    ofstream solutionFile;       // Claude change: shared output across puzzles.

    // Check row, column, and box together instead of using separate loops.
    bool canPlace(int row, int col, int number) {
        const int startRow = (row / BOX_SIZE) * BOX_SIZE;
        const int startCol = (col / BOX_SIZE) * BOX_SIZE;
        // Check the row, column, and each box cell in one pass.
        for (int i = 0; i < SIZE; i++) {
            if (grid[row][i] == number ||
                grid[i][col] == number ||
                grid[startRow + i / BOX_SIZE][startCol + i % BOX_SIZE] == number) {
                return false;
            }
        }
        return true;
    }

    void printGrid(ostream& output = cout) {
        for (int row = 0; row < SIZE; row++) { // Visit every row.
            for (int col = 0; col < SIZE; col++) { // Visit each row's cells.
                if (grid[row][col] == 0) output << "_ "; // Empty cell.
                else output << static_cast<int>(grid[row][col]) << ' '; // Digit.
            }
            output << '\n'; // Finish the current row.
        }
    }

    // Inherited recursive backtracking, with a ChatGPT stack-depth improvement.
    void solve(int position) {
        while (position < CELL_COUNT &&
               grid[position / SIZE][position % SIZE] != 0) {
            position++;
        }
        // Reaching the end means the entire grid is a valid solution.
        if (position == CELL_COUNT) {
            solutionCount++;       // Count this solution.
            printGrid(solutionFile); // Save its nine rows.
            solutionFile << '\n';  // Separate multiple solutions.
            return;
        }
        const int row = position / SIZE;
        const int col = position % SIZE;
        // Try every digit, then undo each placement to explore alternatives.
        for (int number = 1; number <= SIZE; number++) {
            if (canPlace(row, col, number)) {
                grid[row][col] = number; // Make a valid trial placement.
                solve(position + 1);     // Continue with the next cell.
                grid[row][col] = 0;      // Backtrack for the next trial.
            }
        }
    }

public:
    // Claude change: open one output file for all five puzzles; overwrite old runs.
    explicit Sudoku(const string& solutionFilename) {
        solutionFile.open(solutionFilename);
        if (!solutionFile) cout << "Could not open " << solutionFilename << '\n';
    }

    // Close the shared file when this Sudoku object goes out of scope.
    ~Sudoku() {
        if (solutionFile.is_open()) solutionFile.close();
    }

    // Check that the program can write the results before it starts solving.
    bool ready() const { return solutionFile.is_open(); }

    // Process one input puzzle and append its results to the shared output.
    void run(const string& filename) {
        solutionFile << "Puzzle: " << filename << '\n'; // Required heading.
        ifstream file(filename); // Open the current puzzle's input.
        if (!file) {
            cout << "Could not open " << filename << '\n';
            solutionFile << "Invalid puzzle file.\n\n";
            return;
        }

        // Inherited input validation: read exactly 81 digits or underscores.
        char cell;
        for (int row = 0; row < SIZE; row++) {
            for (int col = 0; col < SIZE; col++) {
                if (!(file >> cell) || (cell != '_' && (cell < '1' || cell > '9'))) {
                    cout << "Invalid puzzle file: " << filename
                         << " at row " << row + 1 << ", column " << col + 1 << '\n';
                    solutionFile << "Invalid puzzle file.\n\n";
                    return;
                }
                // Translate underscores to empty cells and digits to numbers.
                grid[row][col] = (cell == '_') ? 0 : cell - '0';
            }
        }
        // Reject extra non-whitespace characters after the 81 cells.
        if (file >> cell) {
            cout << "Invalid puzzle file: too much data in " << filename << '\n';
            solutionFile << "Invalid puzzle file.\n\n";
            return;
        }

        // Reset the counter and print the loaded puzzle for reference.
        solutionCount = 0;
        cout << "File: " << filename << "\nOriginal puzzle:\n";
        printGrid();
        cout << '\n';

        // Inherited conflict check: existing clues must not break Sudoku rules.
        bool valid = true;
        for (int row = 0; row < SIZE && valid; row++) {
            for (int col = 0; col < SIZE && valid; col++) {
                if (grid[row][col] != 0) {
                    int number = grid[row][col];
                    grid[row][col] = 0; // Temporarily remove the clue.
                    if (!canPlace(row, col, number)) valid = false; // Check it.
                    grid[row][col] = number; // Restore the original clue.
                }
            }
        }

        // Solve valid puzzles, including puzzles with multiple solutions.
        if (valid) solve(0);
        // Write the required summary for this puzzle.
        if (solutionCount == 0) {
            solutionFile << "No solution found.\n";
            cout << "No solution found.\n";
        } else {
            solutionFile << "Solutions found: " << solutionCount << '\n';
        }
        solutionFile << '\n'; // Separate this puzzle from the next.
        cout << "Total solutions: " << solutionCount << "\n\n";
    }
};

// ChatGPT improvement: the named count avoids a repeated magic number.
int main() {
    constexpr int PUZZLE_COUNT = 5;
    Sudoku sudoku("solutions.txt"); // Create one shared output file.
    if (!sudoku.ready()) return 1; // Stop if it cannot be opened.
    // Process puzzle1.txt through puzzle5.txt in order.
    for (int i = 1; i <= PUZZLE_COUNT; i++) {
        sudoku.run("puzzle" + to_string(i) + ".txt");
    }
    return 0; // Finished processing all puzzles.
}
