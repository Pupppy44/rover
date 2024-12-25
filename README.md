<p align="center"> <img width="default" height="300"
        src="https://i.ibb.co/gTNvN9j/download-1.png">
</p>

# rover - File-Based Database Library For C++

Rover is a single-header, file-based database library written in C++. It provides a simple, lightweight interface for managing structured data, supporting various data types like integers, floats, strings, and booleans, all stored in binary files for fast access and efficient storage. Rover works similarly to [SQLite](https://github.com/sqlite/sqlite).

## Features

- Allows table creation, row insertion, and data retrieval with ease
- Supports multiple data types including integers, floats, strings, and booleans
- Efficient data serialization for fast read and write operations
- Single-header with no dependencies

## Installation

1. Download the single-header library file `rover.hpp`.
2. Include the library in your project using `#include "rover.hpp"`.

## Examples

#### 1. **Creating a Database and Table**

```cpp
#include "rover.hpp"

int main() {
    rover_db db("example.rdb");

    if (!db.open()) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    db.create_table("users");

    db.close();
    return 0;
}
```

#### 2. **Inserting Data into a Table**

```cpp
#include "rover.hpp"

int main() {
    rover_db db("example.rdb");

    if (!db.open()) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    db.create_table("users");

    rover_db_row row;
    row.add_value("id", 1);
    row.add_value("name", "Alice");
    row.add_value("balance", 30.99);
    row.add_value("is_active", true);

    db.insert_row("users", row);

    db.close();
    return 0;
}
```

#### 3. **Retrieving Data from a Table**

```cpp
#include "rover.hpp"

int main() {
    rover_db db("example.rdb");

    if (!db.open()) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    auto rows = db.get_table("users");

    for (const auto& row : rows) {
        std::cout << "User ID: " << row.get_int("id") << std::endl;
        std::cout << "Name: " << row.get_string("name") << std::endl;
        std::cout << "Age: " << row.get_int("age") << std::endl;
        std::cout << "Active: " << row.get_bool("is_active") << std::endl;
    }

    db.close();
    return 0;
}
```


## Contact
If you have any questions or requests, feel free to create an Issue or Pull Request. Thanks!