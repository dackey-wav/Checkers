#include <chrono>
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include "../include/Board.hpp"
#include "../include/AI.hpp"

// struktura do przechowywania wyników pomiarów
struct PerformanceResult {
    int depth;
    double averageTime;
    double minTime;
    double maxTime;
    int totalMoves;
    int nodesEvaluated;
};

// Klasa do mierzenia wydajności
class PerformanceMeasurer {
private:
    std::vector<PerformanceResult> results;
    
public:
    // pomiar czasu jednego ruchu
    double measureSingleMove(Board& board, int depth) {
        std::vector<Move> moves = board.getAllValidMoves(Piececolor::Black);
        if (moves.empty()) {
            std::cout << "Brak dostępnych ruchów dla AI" << std::endl;
            return -1.0;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            Move bestMove = findBestMove(board, depth);
        } catch (const std::exception& e) {
            std::cout << "Błąd podczas szukania ruchu: " << e.what() << std::endl;
            return -1.0;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        return duration.count() / 1000.0; // ms
    }
    
    // pomiar dla głębokości
    PerformanceResult measureDepth(int depth, int numTests = 10) {
        std::vector<double> times;
        int successfulTests = 0;
        
        std::cout << "Testowanie głębokości " << depth << " (" << numTests << " testów)..." << std::endl;
        
        for (int i = 0; i < numTests; ++i) {
            Board testBoard;
            
            testBoard.initialize();
            
            std::cout << "  Próba inicjalizacji planszy dla testu " << (i+1) << "..." << std::endl;
            
            double time = measureSingleMove(testBoard, depth);
            
            if (time >= 0) {
                times.push_back(time);
                successfulTests++;
                std::cout << "  Test " << (i+1) << ": " << std::fixed << std::setprecision(2) 
                         << time << " ms" << std::endl;
            }
        }
        
        PerformanceResult result;
        result.depth = depth;
        result.totalMoves = successfulTests;
        
        if (!times.empty()) {
            double total = 0;
            result.minTime = times[0];
            result.maxTime = times[0];
            
            for (double time : times) {
                total += time;
                result.minTime = std::min(result.minTime, time);
                result.maxTime = std::max(result.maxTime, time);
            }
            
            result.averageTime = total / times.size();
        } else {
            result.averageTime = result.minTime = result.maxTime = 0;
        }
        
        return result;
    }
    
    // testowanie wszystkich poziomów trudności
    void runFullPerformanceTest() {
        std::cout << "=== POMIAR WYDAJNOŚCI ALGORYTMU AI ===" << std::endl;
        std::cout << std::endl;
        
        std::vector<int> depths = {1, 2, 3, 4, 5, 6}; 
        
        for (int depth : depths) {
            PerformanceResult result = measureDepth(depth, 5); // 5 testów dla każdej głębokości
            results.push_back(result);
            std::cout << std::endl;
        }
        
        printResults();
        saveResultsToFile("wyniki_wydajnosci.txt");
    }
    
    // wypisywanie w konsoli
    void printResults() {
        std::cout << "=== WYNIKI POMIARÓW ===" << std::endl;
        std::cout << std::left << std::setw(10) << "Głębokość" 
                  << std::setw(15) << "Średnia (ms)" 
                  << std::setw(12) << "Min (ms)" 
                  << std::setw(12) << "Max (ms)" 
                  << std::setw(10) << "Testów" << std::endl;
        std::cout << std::string(65, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(10) << result.depth
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.averageTime
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.minTime
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.maxTime
                      << std::setw(10) << result.totalMoves << std::endl;
        }
        
        std::cout << std::endl;
        analyzeComplexity();
    }
    
    void analyzeComplexity() {
        std::cout << "=== ANALIZA ZŁOŻONOŚCI CZASOWEJ ===" << std::endl;
        
        if (results.size() >= 2) {
            for (size_t i = 1; i < results.size(); ++i) {
                if (results[i-1].averageTime > 0 && results[i].averageTime > 0) {
                    double ratio = results[i].averageTime / results[i-1].averageTime;
                    std::cout << "Stosunek czasu głębokości " << results[i].depth 
                             << " do głębokości " << results[i-1].depth << ": " 
                             << std::fixed << std::setprecision(2) << ratio << "x" << std::endl;
                } else {
                    std::cout << "Brak danych dla porównania głębokości " << results[i].depth 
                             << " i " << results[i-1].depth << std::endl;
                }
            }
            
            std::cout << std::endl;
            std::cout << "Teoretycznie dla minimax oczekiwany wzrost wykładniczy (~b^d)" << std::endl;
        }
    }
    
    void saveResultsToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Nie udało się utworzyć pliku " << filename << std::endl;
            return;
        }
        
        file << "Wyniki pomiaru wydajności algorytmu AI dla warcabów\n";
        file << "Data: " << getCurrentDateTime() << "\n\n";
        
        file << std::left << std::setw(10) << "Głębokość" 
             << std::setw(15) << "Średnia (ms)" 
             << std::setw(12) << "Min (ms)" 
             << std::setw(12) << "Max (ms)" 
             << std::setw(10) << "Testów" << "\n";
        file << std::string(65, '-') << "\n";
        
        for (const auto& result : results) {
            file << std::left << std::setw(10) << result.depth
                 << std::setw(15) << std::fixed << std::setprecision(2) << result.averageTime
                 << std::setw(12) << std::fixed << std::setprecision(2) << result.minTime
                 << std::setw(12) << std::fixed << std::setprecision(2) << result.maxTime
                 << std::setw(10) << result.totalMoves << "\n";
        }
        
        file << "\nAnaliza złożoności czasowej:\n";
        for (size_t i = 1; i < results.size(); ++i) {
            double ratio = results[i].averageTime / results[i-1].averageTime;
            file << "Stosunek czasu głębokości " << results[i].depth 
                 << " do głębokości " << results[i-1].depth << ": " 
                 << std::fixed << std::setprecision(2) << ratio << "x\n";
        }
        
        file.close();
        std::cout << "Wyniki zapisane w pliku: " << filename << std::endl;
    }
    
private:
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// debug
void debugBoardState() {
    std::cout << "=== DEBUG: SPRAWDZANIE STANU PLANSZY ===" << std::endl;
    
    Board testBoard;
    
    testBoard.initialize();
    
    std::cout << "Sprawdzanie dostępnych ruchów dla czarnych..." << std::endl;
    std::vector<Move> blackMoves = testBoard.getAllValidMoves(Piececolor::Black);
    std::cout << "Liczba dostępnych ruchów dla czarnych: " << blackMoves.size() << std::endl;
    
    std::vector<Move> whiteMoves = testBoard.getAllValidMoves(Piececolor::White);
    std::cout << "Liczba dostępnych ruchów dla białych: " << whiteMoves.size() << std::endl;
    
    if (!blackMoves.empty()) {
        std::cout << "Próba wywołania findBestMove..." << std::endl;
        try {
            Move bestMove = findBestMove(testBoard, 1);
            std::cout << "findBestMove zakończone pomyślnie!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "BŁĄD w findBestMove: " << e.what() << std::endl;
        }
    } else {
        std::cout << "PROBLEM: Brak ruchów dla czarnych - plansza nie została zainicjalizowana!" << std::endl;
    }
}
void testSpecificPosition() {
    std::cout << "=== TEST KONKRETNEJ POZYCJI ===" << std::endl;
    
    Board testBoard;
    testBoard.initialize();
    
    PerformanceMeasurer measurer;
    
    for (int depth = 1; depth <= 5; ++depth) {
        double time = measurer.measureSingleMove(testBoard, depth);
        std::cout << "Głębokość " << depth << ": " << std::fixed << std::setprecision(2) 
                 << time << " ms" << std::endl;
    }
}

int main() {
    try {
        debugBoardState();
        
        std::cout << "\nCzy chcesz kontynuować pełne testy? (t/n): ";
        char choice;
        std::cin >> choice;
        
        if (choice == 't' || choice == 'T') {
            PerformanceMeasurer measurer;
            measurer.runFullPerformanceTest();
        }
        
    } catch (const std::exception& e) {
        std::cout << "Błąd: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}