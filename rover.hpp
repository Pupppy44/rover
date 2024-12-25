// rover.hpp
// Version 1.0

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <cstdint>
#include <cstring>
#include <stdexcept>

enum class rover_db_type : uint8_t {
    NONE,
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN
};

struct rover_db_value {
    rover_db_type type;
    std::variant<int64_t, double, std::string, bool> value;
};

struct rover_db_row {
private:
    std::unordered_map<std::string, rover_db_value> data_;

public:
    // Add an integer value to the row
    void add_value(const std::string& key, int64_t value) {
        data_[key] = { rover_db_type::INTEGER, value };
    }

    // Add a float value to the row
    void add_value(const std::string& key, double value) {
        data_[key] = { rover_db_type::FLOAT, value };
    }

    // Add a string value to the row
    void add_value(const std::string& key, const std::string& value) {
        data_[key] = { rover_db_type::STRING, value };
    }

    // Add a boolean value to the row
    void add_value(const std::string& key, bool value) {
        data_[key] = { rover_db_type::BOOLEAN, value };
    }

    // Get an integer value from the row by key
    int64_t get_int(const std::string& key) const {
        if (data_.count(key) && data_.at(key).type == rover_db_type::INTEGER) {
            return std::get<int64_t>(data_.at(key).value);
        }
        throw std::runtime_error("Key not found or type mismatch");
    }

    // Get a float value from the row by key
    double get_float(const std::string& key) const {
        if (data_.count(key) && data_.at(key).type == rover_db_type::FLOAT) {
            return std::get<double>(data_.at(key).value);
        }
        throw std::runtime_error("Key not found or type mismatch");
    }

    // Get a string value from the row by key
    std::string get_string(const std::string& key) const {
        if (data_.count(key) && data_.at(key).type == rover_db_type::STRING) {
            return std::get<std::string>(data_.at(key).value);
        }
        throw std::runtime_error("Key not found or type mismatch");
    }

    // Get a boolean value from the row by key
    bool get_bool(const std::string& key) const {
        if (data_.count(key) && data_.at(key).type == rover_db_type::BOOLEAN) {
            return std::get<bool>(data_.at(key).value);
        }
        throw std::runtime_error("Key not found or type mismatch");
    }

    // Get the internal data map (const version)
    const std::unordered_map<std::string, rover_db_value>& get_internal_data() const {
        return data_;
    }

    // Get the internal data map (non-const version)
    std::unordered_map<std::string, rover_db_value>& get_internal_data() {
        return data_;
    }
};

// Helper Functions

template <typename T>
bool serialize(std::fstream& fs, const T& value) {
    fs.write(reinterpret_cast<const char*>(&value), sizeof(T));
    return fs.good();
}

template <typename T>
bool deserialize(std::fstream& fs, T& value) {
    fs.read(reinterpret_cast<char*>(&value), sizeof(T));
    return fs.good();
}

bool serialize_string(std::fstream& fs, const std::string& str) {
    uint32_t size = static_cast<uint32_t>(str.size());
    if (!serialize(fs, size)) return false;
    fs.write(str.data(), size);
    return fs.good();
}

bool deserialize_string(std::fstream& fs, std::string& str) {
    uint32_t size;
    if (!deserialize(fs, size)) return false;
    str.resize(size);
    fs.read(str.data(), size);
    return fs.good();
}

bool serialize_rover_db_value(std::fstream& fs, const rover_db_value& value) {
    if (!serialize(fs, value.type)) return false;
    switch (value.type) {
    case rover_db_type::INTEGER: return serialize(fs, std::get<int64_t>(value.value));
    case rover_db_type::FLOAT:   return serialize(fs, std::get<double>(value.value));
    case rover_db_type::STRING:  return serialize_string(fs, std::get<std::string>(value.value));
    case rover_db_type::BOOLEAN: return serialize(fs, std::get<bool>(value.value));
    default: return false;
    }
}

bool deserialize_rover_db_value(std::fstream& fs, rover_db_value& value) {
    if (!deserialize(fs, value.type)) return false;
    switch (value.type) {
    case rover_db_type::INTEGER: { int64_t v; if (deserialize(fs, v)) { value.value = v; return true; } break; }
    case rover_db_type::FLOAT: { double v;  if (deserialize(fs, v)) { value.value = v; return true; } break; }
    case rover_db_type::STRING: { std::string v; if (deserialize_string(fs, v)) { value.value = v; return true; } break; }
    case rover_db_type::BOOLEAN: { bool v;   if (deserialize(fs, v)) { value.value = v; return true; } break; }
    default: return false;
    }
    return false;
}

// Main Classes

class rover_db {
public:
    // Constructor that takes a filename
    rover_db(const std::string& filename) : filename_(filename) {}

    // Open the database file
    bool open() {
        file_.open(filename_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        return file_.is_open();
    }

    // Close the database file
    void close() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    // Create a new table in the database
    bool create_table(const std::string& table_name) {
        if (!file_.is_open()) return false;
        uint8_t type = 1; // Indicate table creation
        if (!serialize(file_, type)) return false;
        return serialize_string(file_, table_name);
    }

    // Insert a new row into a table
    bool insert_row(const std::string& table_name, const rover_db_row& row_data) {
        if (!file_.is_open()) return false;
        uint8_t type = 2; // Indicate row insertion
        if (!serialize(file_, type)) return false;
        if (!serialize_string(file_, table_name)) return false;

        uint32_t num_columns = static_cast<uint32_t>(row_data.get_internal_data().size());
        if (!serialize(file_, num_columns)) return false;

        for (const auto& pair : row_data.get_internal_data()) {
            if (!serialize_string(file_, pair.first)) return false;
            if (!serialize_rover_db_value(file_, pair.second)) return false;
        }
        return true;
    }

    // Get all rows from a specific table
    std::vector<rover_db_row> get_table(const std::string& table_name) {
        std::vector<rover_db_row> rows;
        if (!file_.is_open()) return rows;

        file_.clear();
        file_.seekg(0, std::ios::beg);

        while (file_.peek() != EOF) {
            uint8_t record_type;
            if (!deserialize(file_, record_type)) break;

            if (record_type == 1) { // Table creation
                std::string stored_table_name;
                if (!deserialize_string(file_, stored_table_name)) break;
                // No action needed for now
            }
            else if (record_type == 2) { // Row insertion
                std::string stored_table_name;
                if (!deserialize_string(file_, stored_table_name)) break;
                if (stored_table_name == table_name) {
                    rover_db_row row;
                    uint32_t num_columns;
                    if (!deserialize(file_, num_columns)) break;
                    for (uint32_t i = 0; i < num_columns; ++i) {
                        std::string key;
                        rover_db_value value;
                        if (!deserialize_string(file_, key)) goto end_loop;
                        if (!deserialize_rover_db_value(file_, value)) goto end_loop;
                        row.get_internal_data()[key] = value;
                    }
                    rows.push_back(row);
                }
                else {
                    // Skip the row data
                    uint32_t num_columns;
                    if (!deserialize(file_, num_columns)) break;
                    for (uint32_t i = 0; i < num_columns; ++i) {
                        std::string key;
                        rover_db_value value;
                        if (!deserialize_string(file_, key)) goto end_loop;
                        uint8_t value_type_raw;
                        if (!deserialize(file_, value_type_raw)) goto end_loop;
                        rover_db_type value_type = static_cast<rover_db_type>(value_type_raw);
                        switch (value_type) {
                        case rover_db_type::INTEGER: file_.seekg(sizeof(int64_t), std::ios::cur); break;
                        case rover_db_type::FLOAT: file_.seekg(sizeof(double), std::ios::cur); break;
                        case rover_db_type::STRING: { uint32_t str_size; if (!deserialize(file_, str_size)) goto end_loop; file_.seekg(str_size, std::ios::cur); break; }
                        case rover_db_type::BOOLEAN: file_.seekg(sizeof(bool), std::ios::cur); break;
                        default: goto end_loop;
                        }
                    }
                }
            }
        }
    end_loop:
        return rows;
    }

    // Insert multiple rows into a table
    bool bulk_insert(const std::string& table_name, const std::vector<rover_db_row>& rows_to_insert) {
        if (!file_.is_open()) return false;
        for (const auto& row : rows_to_insert) {
            if (!insert_row(table_name, row)) return false;
        }
        return true;
    }

private:
    std::string filename_;
    std::fstream file_;
};
