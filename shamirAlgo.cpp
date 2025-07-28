#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using Share = pair<long long, long long>;

const long long MOD = 1e9 + 7;

// Modular exponentiation
long long modPow(long long a, long long b, long long mod) {
    long long result = 1;
    a %= mod;
    while (b > 0) {
        if (b & 1) result = (result * a) % mod;
        a = (a * a) % mod;
        b >>= 1;
    }
    return result;
}

// Modular inverse using Fermat's Little Theorem
long long modInverse(long long a, long long mod) {
    if (a == 0) throw invalid_argument("Division by zero in modular inverse");
    return modPow(a, mod - 2, mod);
}

// Convert base-N string to decimal with validation
long long baseToDecimal(const string& value, int base) {
    if (base < 2 || base > 36)
        throw invalid_argument("Unsupported base: " + to_string(base));

    long long result = 0;
    for (char c : value) {
        int digit;
        if (isdigit(c)) digit = c - '0';
        else if (isalpha(c)) digit = toupper(c) - 'A' + 10;
        else throw invalid_argument("Invalid character in base string: " + string(1, c));

        if (digit >= base)
            throw invalid_argument("Digit " + string(1, c) + " out of range for base " + to_string(base));

        result = result * base + digit;
        if (result >= MOD) result %= MOD;  // Avoid overflow
    }
    return result;
}

// Perform Lagrange interpolation at x = 0 under modulo
long long lagrangeInterpolation(const vector<Share>& points) {
    long long result = 0;
    int k = points.size();

    for (int i = 0; i < k; ++i) {
        long long xi = points[i].first;
        long long yi = points[i].second;

        long long numerator = 1;
        long long denominator = 1;

        for (int j = 0; j < k; ++j) {
            if (i != j) {
                long long xj = points[j].first;
                numerator = (numerator * (-xj + MOD)) % MOD;
                denominator = (denominator * (xi - xj + MOD)) % MOD;
            }
        }

        long long term = (yi * numerator % MOD) * modInverse(denominator, MOD) % MOD;
        result = (result + term) % MOD;
    }

    return (result + MOD) % MOD;  // Ensure non-negative
}

// Parse JSON and extract shares as (x, y) pairs
vector<Share> parseInput(const string& filename, int& k) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw runtime_error("Failed to open file: " + filename);
    }

    json j;
    inFile >> j;

    vector<Share> points;
    if (!j.contains("keys") || !j["keys"].contains("k")) {
        throw invalid_argument("Missing 'keys.k' in JSON file: " + filename);
    }

    k = j["keys"]["k"];

    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;

        try {
            long long x = stoll(el.key());
            string baseStr = el.value().at("base");
            string val = el.value().at("value");

            int base = stoi(baseStr);
            long long y = baseToDecimal(val, base);

            points.emplace_back(x, y);
        } catch (const exception& e) {
            cerr << "Skipping entry in " << filename << " due to error: " << e.what() << endl;
        }
    }

    return points;
}

int main() {
    vector<string> files = {"input1.json", "input2.json"};

    for (const auto& file : files) {
        try {
            int k;
            vector<Share> points = parseInput(file, k);

            if (points.size() < k) {
                cerr << "Not enough points in " << file << " to interpolate (need " << k << ", got " << points.size() << ")." << endl;
                continue;
            }

            vector<Share> subset(points.begin(), points.begin() + k);
            long long secret = lagrangeInterpolation(subset);

            cout << "Secret from " << file << ": " << secret << endl;

        } catch (const exception& e) {
            cerr << "Error processing " << file << ": " << e.what() << endl;
        }
    }

    return 0;
}
