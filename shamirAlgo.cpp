#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using Share = pair<long long, long long>;

const long long MOD = 1e9 + 7;

// Fast modular exponentiation
long long modPow(long long base, long long exp, long long mod = MOD) {
    long long result = 1;
    base %= mod;
    while (exp) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

// Modular inverse using Fermat's little theorem
long long modInverse(long long a, long long mod = MOD) {
    return modPow(a, mod - 2, mod);
}

// Base-N to decimal converter
long long baseToDecimal(const string& value, int base) {
    long long result = 0;
    for (char c : value) {
        if (c < '0' || c > '9') return -1;  // Invalid character
        result = result * base + (c - '0');
    }
    return result;
}

// Lagrange interpolation at x = 0 under modulo MOD
long long lagrangeInterpolation(const vector<Share>& points) {
    long long result = 0;
    int k = points.size();

    for (int i = 0; i < k; ++i) {
        long long xi = points[i].first;
        long long yi = points[i].second;

        long long numerator = 1;
        long long denominator = 1;

        for (int j = 0; j < k; ++j) {
            if (i == j) continue;
            long long xj = points[j].first;
            numerator = (numerator * (-xj + MOD)) % MOD;
            denominator = (denominator * (xi - xj + MOD)) % MOD;
        }

        long long term = yi * numerator % MOD * modInverse(denominator) % MOD;
        result = (result + term) % MOD;
    }

    return (result + MOD) % MOD;
}

// Parse input JSON and validate base/x uniqueness
vector<Share> parseInput(const string& filename, int& k) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        exit(1);
    }

    json j;
    inFile >> j;

    vector<Share> points;
    set<long long> seenX;
    k = j["keys"]["k"];

    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        long long x = stoll(el.key());
        if (seenX.count(x)) continue;
        seenX.insert(x);

        int base = stoi(el.value()["base"].get<string>());
        string val = el.value()["value"];
        long long y = baseToDecimal(val, base);
        if (y == -1) continue;  // Invalid base representation

        points.emplace_back(x, y);
    }

    return points;
}

int main() {
    vector<string> files = {"input1.json", "input2.json"};

    for (const auto& file : files) {
        int k;
        vector<Share> points = parseInput(file, k);
        if (points.size() < k) {
            cerr << "Not enough valid points in " << file << " to interpolate." << endl;
            continue;
        }

        // Use only the first k points for interpolation
        vector<Share> subset(points.begin(), points.begin() + k);
        long long secret = lagrangeInterpolation(subset);

        cout << "Secret from " << file << ": " << secret << endl;
    }

    return 0;
}
