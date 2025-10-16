#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <functional>
#include <cstring>

using namespace std;

//==============================================================================
//                                PRODUCT CLASS
//==============================================================================

class Product {
private:
    string name;
    string productID;
    int quantity;
    double price;

public:
    // Constructors
    Product() : name(""), productID(""), quantity(0), price(0.0) {}
    
    Product(const string& name, const string& id, int qty, double price)
        : name(name), productID(id), quantity(qty), price(price) {}
    
    // Destructor
    ~Product() {}
    
    // Getters
    string getName() const { return name; }
    string getProductID() const { return productID; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    double getTotalValue() const { return quantity * price; }
    
    // Setters with validation
    void setName(const string& name) { this->name = name; }
    void setProductID(const string& id) { this->productID = id; }
    
    bool setQuantity(int qty) {
        if (qty < 0) {
            cerr << "Error: Quantity cannot be negative.\n";
            return false;
        }
        quantity = qty;
        return true;
    }
    
    bool setPrice(double price) {
        if (price < 0.0) {
            cerr << "Error: Price cannot be negative.\n";
            return false;
        }
        this->price = price;
        return true;
    }
    
    // Display product information
    void display() const {
        cout << left << setw(15) << productID
             << setw(25) << name
             << setw(12) << quantity
             << setw(12) << fixed << setprecision(2) << price
             << setw(15) << getTotalValue();
        
        if (isLowStock()) {
            cout << " [LOW STOCK]";
        }
        cout << "\n";
    }
    
    // Check if product is low stock
    bool isLowStock(int threshold = 10) const {
        return quantity <= threshold;
    }
    
    // Serialize product to binary file
    void serialize(ostream& out) const {
        size_t nameLen = name.length();
        size_t idLen = productID.length();
        
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        out.write(name.c_str(), nameLen);
        
        out.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
        out.write(productID.c_str(), idLen);
        
        out.write(reinterpret_cast<const char*>(&quantity), sizeof(quantity));
        out.write(reinterpret_cast<const char*>(&price), sizeof(price));
    }
    
    // Deserialize product from binary file
    void deserialize(istream& in) {
        size_t nameLen, idLen;
        
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        name.resize(nameLen);
        in.read(&name[0], nameLen);
        
        in.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        productID.resize(idLen);
        in.read(&productID[0], idLen);
        
        in.read(reinterpret_cast<char*>(&quantity), sizeof(quantity));
        in.read(reinterpret_cast<char*>(&price), sizeof(price));
    }
    
    // Operator overloading for comparison
    bool operator==(const Product& other) const {
        return productID == other.productID;
    }
};

//==============================================================================
//                              AUTHENTICATION CLASS
//==============================================================================

class Authentication {
private:
    map<string, string> users; // username -> hashed password
    string filename;
    string currentUser;
    
    // Simple hash function (in production, use bcrypt or similar)
    string hashPassword(const string& password) const {
        hash<string> hasher;
        return to_string(hasher(password));
    }

public:
    // Constructor
    Authentication(const string& filename = "users.dat") 
        : filename(filename), currentUser("") {
        loadUsers();
        
        // Create default admin account if no users exist
        if (users.empty()) {
            registerUser("admin", "admin123");
            cout << "Default admin account created (username: admin, password: admin123)\n";
        }
    }
    
    // Destructor
    ~Authentication() {
        saveUsers();
    }
    
    // Register new user
    bool registerUser(const string& username, const string& password) {
        if (username.empty() || password.empty()) {
            cerr << "Error: Username and password cannot be empty.\n";
            return false;
        }
        
        if (users.find(username) != users.end()) {
            cerr << "Error: Username already exists.\n";
            return false;
        }
        
        if (password.length() < 6) {
            cerr << "Error: Password must be at least 6 characters long.\n";
            return false;
        }
        
        users[username] = hashPassword(password);
        saveUsers();
        cout << "User registered successfully!\n";
        return true;
    }
    
    // Login
    bool login(const string& username, const string& password) {
        auto it = users.find(username);
        
        if (it == users.end()) {
            cerr << "Error: Invalid username or password.\n";
            return false;
        }
        
        if (it->second != hashPassword(password)) {
            cerr << "Error: Invalid username or password.\n";
            return false;
        }
        
        currentUser = username;
        cout << "Login successful! Welcome, " << username << "!\n";
        return true;
    }
    
    // Logout
    void logout() {
        if (!currentUser.empty()) {
            cout << "Goodbye, " << currentUser << "!\n";
            currentUser = "";
        }
    }
    
    // Check if user is logged in
    bool isLoggedIn() const {
        return !currentUser.empty();
    }
    
    // Get current user
    string getCurrentUser() const {
        return currentUser;
    }
    
    // Save users to file
    bool saveUsers() {
        ofstream file(filename, ios::binary | ios::trunc);
        
        if (!file.is_open()) {
            return false;
        }
        
        size_t count = users.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        for (const auto& pair : users) {
            size_t usernameLen = pair.first.length();
            size_t passwordLen = pair.second.length();
            
            file.write(reinterpret_cast<const char*>(&usernameLen), sizeof(usernameLen));
            file.write(pair.first.c_str(), usernameLen);
            
            file.write(reinterpret_cast<const char*>(&passwordLen), sizeof(passwordLen));
            file.write(pair.second.c_str(), passwordLen);
        }
        
        file.close();
        return true;
    }
    
    // Load users from file
    bool loadUsers() {
        ifstream file(filename, ios::binary);
        
        if (!file.is_open()) {
            return true; // File doesn't exist yet
        }
        
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        if (file.fail()) {
            file.close();
            return false;
        }
        
        users.clear();
        for (size_t i = 0; i < count; ++i) {
            size_t usernameLen, passwordLen;
            
            file.read(reinterpret_cast<char*>(&usernameLen), sizeof(usernameLen));
            string username(usernameLen, ' ');
            file.read(&username[0], usernameLen);
            
            file.read(reinterpret_cast<char*>(&passwordLen), sizeof(passwordLen));
            string password(passwordLen, ' ');
            file.read(&password[0], passwordLen);
            
            if (file.fail()) {
                file.close();
                return false;
            }
            
            users[username] = password;
        }
        
        file.close();
        return true;
    }
};

//==============================================================================
//                               INVENTORY CLASS
//==============================================================================

class Inventory {
private:
    map<string, Product> products; // Using map for efficient search by ID
    string filename;
    
    // Helper function to validate product ID uniqueness
    bool isUniqueID(const string& id) const {
        return products.find(id) == products.end();
    }

public:
    // Constructor
    Inventory(const string& filename = "inventory.dat") : filename(filename) {
        loadFromFile();
    }
    
    // Destructor
    ~Inventory() {
        saveToFile();
    }
    
    // Add new product
    bool addProduct(const Product& product) {
        if (!isUniqueID(product.getProductID())) {
            cerr << "Error: Product ID already exists.\n";
            return false;
        }
        
        if (product.getProductID().empty() || product.getName().empty()) {
            cerr << "Error: Product ID and Name cannot be empty.\n";
            return false;
        }
        
        products[product.getProductID()] = product;
        cout << "Product added successfully!\n";
        saveToFile(); // Auto-save after modification
        return true;
    }
    
    // Update product details
    bool updateProduct(const string& id, int newQuantity, double newPrice) {
        auto it = products.find(id);
        
        if (it == products.end()) {
            cerr << "Error: Product not found.\n";
            return false;
        }
        
        if (!it->second.setQuantity(newQuantity) || !it->second.setPrice(newPrice)) {
            return false;
        }
        
        cout << "Product updated successfully!\n";
        saveToFile(); // Auto-save after modification
        return true;
    }
    
    // Delete product
    bool deleteProduct(const string& id) {
        auto it = products.find(id);
        
        if (it == products.end()) {
            cerr << "Error: Product not found.\n";
            return false;
        }
        
        products.erase(it);
        cout << "Product deleted successfully!\n";
        saveToFile(); // Auto-save after modification
        return true;
    }
    
    // Search by ID
    Product* searchByID(const string& id) {
        auto it = products.find(id);
        if (it != products.end()) {
            return &(it->second);
        }
        return nullptr;
    }
    
    // Search by name (partial match)
    vector<Product*> searchByName(const string& name) {
        vector<Product*> results;
        string lowerName = name;
        transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        for (auto& pair : products) {
            string productName = pair.second.getName();
            transform(productName.begin(), productName.end(), productName.begin(), ::tolower);
            
            if (productName.find(lowerName) != string::npos) {
                results.push_back(&pair.second);
            }
        }
        
        return results;
    }
    
    // Display all products
    void displayAll() const {
        if (products.empty()) {
            cout << "Inventory is empty.\n";
            return;
        }
        
        cout << "\n" << string(85, '=') << "\n";
        cout << "                        INVENTORY LIST\n";
        cout << string(85, '=') << "\n";
        cout << left << setw(15) << "Product ID"
             << setw(25) << "Product Name"
             << setw(12) << "Quantity"
             << setw(12) << "Price"
             << setw(15) << "Total Value"
             << "Status\n";
        cout << string(85, '-') << "\n";
        
        for (const auto& pair : products) {
            pair.second.display();
        }
        
        cout << string(85, '=') << "\n";
        cout << "Total Products: " << products.size() << "\n";
        cout << "Total Inventory Value: $" << fixed << setprecision(2) 
             << getTotalInventoryValue() << "\n";
        cout << string(85, '=') << "\n\n";
    }
    
    // Display low stock products
    void displayLowStock(int threshold = 10) const {
        cout << "\n" << string(85, '=') << "\n";
        cout << "                    LOW STOCK ALERT (Threshold: " << threshold << ")\n";
        cout << string(85, '=') << "\n";
        
        bool foundLowStock = false;
        for (const auto& pair : products) {
            if (pair.second.isLowStock(threshold)) {
                if (!foundLowStock) {
                    cout << left << setw(15) << "Product ID"
                         << setw(25) << "Product Name"
                         << setw(12) << "Quantity"
                         << setw(12) << "Price"
                         << "Status\n";
                    cout << string(85, '-') << "\n";
                    foundLowStock = true;
                }
                pair.second.display();
            }
        }
        
        if (!foundLowStock) {
            cout << "No low stock items found.\n";
        }
        cout << string(85, '=') << "\n\n";
    }
    
    // Calculate total inventory value
    double getTotalInventoryValue() const {
        double total = 0.0;
        for (const auto& pair : products) {
            total += pair.second.getTotalValue();
        }
        return total;
    }
    
    // Save to file
    bool saveToFile() {
        ofstream file(filename, ios::binary | ios::trunc);
        
        if (!file.is_open()) {
            cerr << "Error: Unable to open file for writing.\n";
            return false;
        }
        
        try {
            // Write number of products
            size_t count = products.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(count));
            
            // Write each product
            for (const auto& pair : products) {
                pair.second.serialize(file);
            }
            
            file.close();
            return true;
        }
        catch (const exception& e) {
            cerr << "Error saving to file: " << e.what() << "\n";
            file.close();
            return false;
        }
    }
    
    // Load from file
    bool loadFromFile() {
        ifstream file(filename, ios::binary);
        
        if (!file.is_open()) {
            // File doesn't exist yet - this is normal for first run
            return true;
        }
        
        try {
            // Read number of products
            size_t count;
            file.read(reinterpret_cast<char*>(&count), sizeof(count));
            
            if (file.fail()) {
                file.close();
                return false;
            }
            
            // Read each product
            products.clear();
            for (size_t i = 0; i < count; ++i) {
                Product product;
                product.deserialize(file);
                
                if (file.fail()) {
                    cerr << "Error reading product data.\n";
                    file.close();
                    return false;
                }
                
                products[product.getProductID()] = product;
            }
            
            file.close();
            cout << "Loaded " << count << " products from file.\n";
            return true;
        }
        catch (const exception& e) {
            cerr << "Error loading from file: " << e.what() << "\n";
            file.close();
            return false;
        }
    }
    
    // Generate reports
    void generateLowStockReport() const { displayLowStock(); }
    void generateInventoryReport() const { displayAll(); }
    
    // Utility functions
    int getProductCount() const { return products.size(); }
    bool isEmpty() const { return products.empty(); }
};

//==============================================================================
//                             INPUT VALIDATION FUNCTIONS
//==============================================================================

// Get validated integer input
int getValidatedInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        
        if (cin.fail() || value < 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a positive integer.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// Get validated double input
double getValidatedDouble(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        
        if (cin.fail() || value < 0.0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a positive number.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// Get validated string input
string getValidatedString(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        getline(cin, value);
        
        if (value.empty()) {
            cout << "Input cannot be empty. Please try again.\n";
        } else {
            return value;
        }
    }
}

//==============================================================================
//                               USER INTERFACE FUNCTIONS
//==============================================================================

// Display main menu
void displayMenu() {
    cout << "\n" << string(50, '=') << "\n";
    cout << "     INVENTORY MANAGEMENT SYSTEM\n";
    cout << string(50, '=') << "\n";
    cout << "1.  Add New Product\n";
    cout << "2.  Display All Products\n";
    cout << "3.  Search Product by ID\n";
    cout << "4.  Search Product by Name\n";
    cout << "5.  Update Product\n";
    cout << "6.  Delete Product\n";
    cout << "7.  Generate Low Stock Report\n";
    cout << "8.  Generate Inventory Report\n";
    cout << "9.  Display Total Inventory Value\n";
    cout << "10. Logout\n";
    cout << string(50, '=') << "\n";
}

// Add product function
void addProduct(Inventory& inventory) {
    cout << "\n--- Add New Product ---\n";
    
    string id = getValidatedString("Enter Product ID: ");
    string name = getValidatedString("Enter Product Name: ");
    int quantity = getValidatedInt("Enter Quantity: ");
    double price = getValidatedDouble("Enter Price: $");
    
    Product product(name, id, quantity, price);
    inventory.addProduct(product);
}

// Update product function
void updateProduct(Inventory& inventory) {
    cout << "\n--- Update Product ---\n";
    
    string id = getValidatedString("Enter Product ID to update: ");
    
    Product* product = inventory.searchByID(id);
    if (product == nullptr) {
        cout << "Product not found.\n";
        return;
    }
    
    cout << "\nCurrent Product Details:\n";
    cout << string(85, '-') << "\n";
    product->display();
    cout << string(85, '-') << "\n";
    
    int newQuantity = getValidatedInt("Enter New Quantity: ");
    double newPrice = getValidatedDouble("Enter New Price: $");
    
    inventory.updateProduct(id, newQuantity, newPrice);
}

// Delete product function
void deleteProduct(Inventory& inventory) {
    cout << "\n--- Delete Product ---\n";
    
    string id = getValidatedString("Enter Product ID to delete: ");
    
    Product* product = inventory.searchByID(id);
    if (product == nullptr) {
        cout << "Product not found.\n";
        return;
    }
    
    cout << "\nProduct to be deleted:\n";
    cout << string(85, '-') << "\n";
    product->display();
    cout << string(85, '-') << "\n";
    
    cout << "Are you sure you want to delete this product? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (confirm == 'y' || confirm == 'Y') {
        inventory.deleteProduct(id);
    } else {
        cout << "Deletion cancelled.\n";
    }
}

// Search by ID function
void searchByID(Inventory& inventory) {
    cout << "\n--- Search Product by ID ---\n";
    
    string id = getValidatedString("Enter Product ID: ");
    
    Product* product = inventory.searchByID(id);
    if (product == nullptr) {
        cout << "Product not found.\n";
        return;
    }
    
    cout << "\nProduct Found:\n";
    cout << string(85, '-') << "\n";
    cout << left << setw(15) << "Product ID"
         << setw(25) << "Product Name"
         << setw(12) << "Quantity"
         << setw(12) << "Price"
         << setw(15) << "Total Value"
         << "Status\n";
    cout << string(85, '-') << "\n";
    product->display();
    cout << string(85, '-') << "\n";
}

// Search by name function
void searchByName(Inventory& inventory) {
    cout << "\n--- Search Product by Name ---\n";
    
    string name = getValidatedString("Enter Product Name (partial match supported): ");
    
    vector<Product*> results = inventory.searchByName(name);
    
    if (results.empty()) {
        cout << "No products found matching \"" << name << "\".\n";
        return;
    }
    
    cout << "\nSearch Results (" << results.size() << " products found):\n";
    cout << string(85, '-') << "\n";
    cout << left << setw(15) << "Product ID"
         << setw(25) << "Product Name"
         << setw(12) << "Quantity"
         << setw(12) << "Price"
         << setw(15) << "Total Value"
         << "Status\n";
    cout << string(85, '-') << "\n";
    
    for (Product* product : results) {
        product->display();
    }
    cout << string(85, '-') << "\n";
}

// Authentication menu
bool authenticationMenu(Authentication& auth) {
    while (!auth.isLoggedIn()) {
        cout << "\n" << string(50, '=') << "\n";
        cout << "     AUTHENTICATION\n";
        cout << string(50, '=') << "\n";
        cout << "1. Login\n";
        cout << "2. Register New User\n";
        cout << "3. Exit\n";
        cout << string(50, '=') << "\n";
        
        int choice = getValidatedInt("Enter your choice: ");
        
        switch (choice) {
            case 1: {
                string username = getValidatedString("Enter username: ");
                string password = getValidatedString("Enter password: ");
                auth.login(username, password);
                break;
            }
            case 2: {
                string username = getValidatedString("Enter new username: ");
                string password = getValidatedString("Enter new password (min 6 characters): ");
                auth.registerUser(username, password);
                break;
            }
            case 3:
                return false;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    }
    return true;
}

//==============================================================================
//                                 MAIN FUNCTION
//==============================================================================

int main() {
    try {
        cout << "==============================================================================\n";
        cout << "                    INVENTORY MANAGEMENT SYSTEM\n";
        cout << "                        C++ Implementation\n";
        cout << "==============================================================================\n";
        
        Authentication auth;
        
        // Authentication required
        if (!authenticationMenu(auth)) {
            cout << "Exiting program...\n";
            return 0;
        }
        
        Inventory inventory;
        
        cout << "\nWelcome to the Inventory Management System!\n";
        
        bool running = true;
        while (running) {
            displayMenu();
            
            int choice = getValidatedInt("Enter your choice: ");
            
            switch (choice) {
                case 1:
                    addProduct(inventory);
                    break;
                    
                case 2:
                    inventory.displayAll();
                    break;
                    
                case 3:
                    searchByID(inventory);
                    break;
                    
                case 4:
                    searchByName(inventory);
                    break;
                    
                case 5:
                    updateProduct(inventory);
                    break;
                    
                case 6:
                    deleteProduct(inventory);
                    break;
                    
                case 7:
                    inventory.generateLowStockReport();
                    break;
                    
                case 8:
                    inventory.generateInventoryReport();
                    break;
                    
                case 9:
                    cout << "\nTotal Inventory Value: $" << fixed << setprecision(2)
                         << inventory.getTotalInventoryValue() << "\n";
                    break;
                    
                case 10:
                    auth.logout();
                    cout << "Logging out...\n";
                    running = false;
                    break;
                    
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        }
        
        cout << "\nThank you for using the Inventory Management System!\n";
        cout << "==============================================================================\n";
        
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}