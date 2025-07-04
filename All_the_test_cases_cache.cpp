// Code your design here
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <array>
#include <memory>
#include <cstdlib>
#include <cstdint>
#include <string>

// -------------------- Replacement Policy --------------------
class RandomReplacement {
public:
    void SetWays(uint8_t numOfWays) { ways = numOfWays; }
    uint8_t GetVictim() const { return rand() % ways; }
private:
    uint8_t ways = 0;
};

// -------------------- Main Memory --------------------
const uint32_t MAIN_MEMORY_SIZE = 4 * 1024 * 1024;

class MainMemory {
public:
    MainMemory() {
        memory = std::make_unique<std::array<uint8_t, MAIN_MEMORY_SIZE>>();
        memory->fill(0x00);
    }
    void Read(uint32_t startAddress, uint8_t size, uint8_t* destination) {
        std::memcpy(destination, &memory->at(startAddress), size);
    }
    void Write(uint32_t startAddress, uint8_t size, uint8_t* source) {
        std::memcpy(&memory->at(startAddress), source, size);
    }
    void Print() {
        const uint32_t ROWS = 24;
        const uint8_t COLUMNS = 12;
        for (uint32_t row = 0; row < ROWS; row++) {
            std::ostringstream oss;
            oss << "0x" << std::setw(4) << std::setfill('0') << std::hex << row * COLUMNS << ": ";
            for (uint32_t column = 0; column < COLUMNS; column++) {
                uint32_t addr = row * COLUMNS + column;
                uint8_t val = memory->at(addr);
                if ((addr >= 0x10 && addr < 0x14) || (addr >= 0x20 && addr < 0x24) || (addr == 0x40))
                    oss << "[*" << std::setw(2) << std::setfill('0') << std::hex << (int)val << "] ";
                else
                    oss << "[ " << std::setw(2) << std::setfill('0') << std::hex << (int)val << "] ";
            }
            std::cout << oss.str() << std::endl;
        }
        std::cout << std::string(80, '-') << std::endl;
    }
private:
    std::unique_ptr<std::array<uint8_t, MAIN_MEMORY_SIZE>> memory;
};

// -------------------- Cache Definitions --------------------
const uint8_t CACHE_LINE_SIZE = 64;
const uint32_t CACHE_SETS = 64;
const uint8_t CACHE_WAYS = 4;

const uint8_t CACHE_LINE_BYTE_OFFSET_SIZE = 6;
const uint8_t CACHE_LINE_SET_INDEX_SIZE = 6;

struct CacheLine {
    uint32_t tag = 0;
    std::array<uint8_t, CACHE_LINE_SIZE> data;
    bool valid = false;
};

struct AddressParts {
    uint32_t tag;
    uint8_t setIndex;
    uint8_t byteOffset;
    AddressParts(uint32_t address) {
        byteOffset = address & (CACHE_LINE_SIZE - 1);
        setIndex = (address >> CACHE_LINE_BYTE_OFFSET_SIZE) & ((1 << CACHE_LINE_SET_INDEX_SIZE) - 1);
        tag = address >> (CACHE_LINE_BYTE_OFFSET_SIZE + CACHE_LINE_SET_INDEX_SIZE);
    }
};

class CacheSet {
public:
    CacheSet() { replacement.SetWays(CACHE_WAYS); }
    CacheLine* Find(uint32_t tag) {
        for (auto& line : lines) if (line.valid && line.tag == tag) return &line;
        return nullptr;
    }
    CacheLine* Replace(uint32_t tag, uint8_t* sourceData) {
        uint8_t victim = replacement.GetVictim();
        lines[victim].valid = true;
        lines[victim].tag = tag;
        std::memcpy(lines[victim].data.data(), sourceData, CACHE_LINE_SIZE);
        return &lines[victim];
    }
    void DebugPrint(uint8_t setIndex) const {
        std::cout << "  Set[" << std::dec << (int)setIndex << "] State:\n";
        for (uint8_t i = 0; i < CACHE_WAYS; i++) {
            if (lines[i].valid)
                std::cout << "    Way " << (int)i << ": VALID | Tag = 0x" << std::hex << lines[i].tag << "\n";
            else
                std::cout << "    Way " << (int)i << ": INVALID\n";
        }
    }
private:
    std::array<CacheLine, CACHE_WAYS> lines;
    RandomReplacement replacement;
};

class Cache {
public:
    void Initialize(MainMemory* memory) { mainMemory = memory; }
    uint32_t Read(uint32_t address) {
        AddressParts a(address);
        CacheLine* line = sets[a.setIndex].Find(a.tag);
        if (line) {
            std::cout << "Reading from cache (address: 0x" << std::hex << address
                      << ", set: " << std::dec << (int)a.setIndex << ", tag: " << std::hex << a.tag << ")\n";
            return *reinterpret_cast<uint32_t*>(&line->data[a.byteOffset]);
        } else {
            uint32_t base = address & ~(CACHE_LINE_SIZE - 1);
            std::array<uint8_t, CACHE_LINE_SIZE> buffer;
            mainMemory->Read(base, CACHE_LINE_SIZE, buffer.data());
            CacheLine* newLine = sets[a.setIndex].Replace(a.tag, buffer.data());
            return *reinterpret_cast<uint32_t*>(&newLine->data[a.byteOffset]);
        }
    }
    void Write(uint32_t address, uint32_t data) {
        AddressParts a(address);
        CacheLine* line = sets[a.setIndex].Find(a.tag);
        if (line) {
            std::cout << "Writing to cache (address: 0x" << std::hex << address
                      << ", set: " << std::dec << (int)a.setIndex << ", tag: " << std::hex << a.tag << ")\n";
            *reinterpret_cast<uint32_t*>(&line->data[a.byteOffset]) = data;
        }
        mainMemory->Write(address, sizeof(uint32_t), reinterpret_cast<uint8_t*>(&data));
    }
    void PrintSet(uint8_t setIndex) const {
        std::cout << "\n[DEBUG] Cache Set Dump (Set Index: " << std::dec << (int)setIndex << ")\n";
        sets[setIndex].DebugPrint(setIndex);
    }
private:
    std::array<CacheSet, CACHE_SETS> sets;
    MainMemory* mainMemory;
};

// -------------------- Memory System --------------------
class MemorySystem {
public:
    MemorySystem() { cache.Initialize(&mainMemory); }
    uint32_t Read(uint32_t address) { return cache.Read(address); }
    void Write(uint32_t address, uint32_t data) { cache.Write(address, data); }
    void PrintMainMemory() { mainMemory.Print(); }
    void PrintCacheSet(uint8_t setIndex) const { cache.PrintSet(setIndex); }
private:
    MainMemory mainMemory;
    Cache cache;
};

// -------------------- Main Test Program --------------------
int main() {
    MemorySystem memory;

    std::cout << "Initial memory slice:\n";
    memory.PrintMainMemory();

    //  Test 1
    memory.Read(0x20); memory.Read(0x20);
    memory.Write(0x20, 0x6139);
    memory.Read(0x20);
    std::cout << "\nMemory slice after writing 0x6139 to address 0x20:\n";
    memory.PrintMainMemory();

    //  Test 2
    std::cout << "\nWriting multiple values to address 0x10...\n";
    memory.Write(0x10, 0x12345678);
    memory.Write(0x10, 0x77777777);
    memory.Write(0x10, 0x52690723);
    uint32_t result = memory.Read(0x10);
    std::cout << "Value at address 0x10 (from cache): 0x" << std::hex << result << std::endl;
    std::cout << "\nMemory slice after multiple writes to address 0x10:\n";
    memory.PrintMainMemory();

    //  Test 3: Cache Thrashing
    std::cout << "\n--- Test 3: Cache Thrashing ---\n";
    std::array<uint32_t, 4> conflictAddresses = {0x0040, 0x1040, 0x2040, 0x3040};
    std::cout << "\nFilling cache set with 4 entries (expecting misses)...\n";
    for (uint32_t addr : conflictAddresses) memory.Read(addr);

    std::cout << "\nAfter filling 4 entries:\n";
    memory.PrintCacheSet(1);

    std::cout << "\nReading same addresses again (expecting hits)...\n";
    for (uint32_t addr : conflictAddresses) memory.Read(addr);

    std::cout << "\nAccessing 0x4040 (should cause eviction)...\n";
    memory.Read(0x4040);

    std::cout << "\nAfter inserting 5th conflicting address:\n";
    memory.PrintCacheSet(1);

    std::cout << "\nRe-reading original 4 addresses (expect one miss due to eviction)...\n";
    for (uint32_t addr : conflictAddresses) memory.Read(addr);

    return 0;
}
