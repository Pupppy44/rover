#include "rover.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <random>

int main() {
    const std::string db_filename = "benchmark_data.rdb";
    const std::string table_name = "benchmark_table";
    const int num_rows = 1000;

    // Remove existing file if it exists
    std::remove(db_filename.c_str());

    rover_db db(db_filename);
    if (!db.open()) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    // --- Insertion Benchmark ---
    auto start_insert = std::chrono::high_resolution_clock::now();

    if (!db.create_table(table_name)) {
        std::cerr << "Failed to create table." << std::endl;
        db.close();
        return 1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);
    std::uniform_real_distribution<> fdistrib(0.0, 1.0);

    std::vector<rover_db_row> rows_to_insert;
    for (int i = 0; i < num_rows; ++i) {
        rover_db_row row;
        row.add_value("int_col", static_cast<int64_t>(distrib(gen)));
        row.add_value("float_col", fdistrib(gen));
        row.add_value("string_col", "Test string " + std::to_string(i));
        row.add_value("bool_col", i % 2 == 0);
        rows_to_insert.push_back(row);
        db.insert_row(table_name, row);
    }

    auto end_insert = std::chrono::high_resolution_clock::now();
    auto duration_insert = std::chrono::duration_cast<std::chrono::milliseconds>(end_insert - start_insert);

    std::cout << "Inserted " << num_rows << " rows in " << duration_insert.count() << " milliseconds." << std::endl;

    // --- Retrieval Benchmark ---
    auto start_retrieve = std::chrono::high_resolution_clock::now();

    std::vector<rover_db_row> retrieved_rows = db.get_table(table_name);

    auto end_retrieve = std::chrono::high_resolution_clock::now();
    auto duration_retrieve = std::chrono::duration_cast<std::chrono::milliseconds>(end_retrieve - start_retrieve);

    std::cout << "Retrieved " << retrieved_rows.size() << " rows in " << duration_retrieve.count() << " milliseconds." << std::endl;

    db.close();
    return 0;
}