# 🧠 C++17 Cache Memory Simulator

A 4-way set-associative L1 data cache simulator implemented in **C++17**, based on the BitLemon YouTube tutorial — enhanced with memory visualization, debug instrumentation, and GDB support. Compatible with **EDA Playground** and tested on **Ubuntu** with `g++` and `gdb`.

---

## ✨ Features

- **4-Way Set-Associative Cache**
  - 64 sets × 64B cache lines
  - Write-through and write-allocate policy
  - Random replacement algorithm
- **Main Memory**
  - 4MB addressable memory
  - Byte-addressed, 32-bit aligned
- **Debug Enhancements**
  - Memory slice printing
  - Cache set state dump
  - Test instrumentation for cache hits/misses and evictions

---

## 🧪 Test Cases Included

1. **Write-through Verification**
   - Writes to cache + main memory
   - Verifies with read-back and memory print
2. **Multiple Overwrites**
   - Writes different values to same address
   - Verifies final value through cache
3. **Cache Thrashing**
   - Accesses conflicting addresses across the same set
   - Triggers eviction and verifies via cache state inspection

---

## 🛠 Tech Stack

| Component         | Description                       |
|------------------|-----------------------------------|
| Language          | C++17                             |
| Build Tool        | `g++`                             |
| Debugging         | `gdb` + `layout split` (optional) |
| Platform          | Ubuntu 20.04+ / OnlineGDB / EDA Playground |
| Tutorial Base     | BitLemon (YouTube)                |

---

## 🖥️ Running the Project

### 🧱 Compile
```bash
g++ -std=c++17 -g -o cache_sim design.cpp
