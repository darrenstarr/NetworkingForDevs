#ifndef ETHERNET_FRAME_H
#define ETHERNET_FRAME_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <array>
#include <memory>

// Define a MAC address as an array of 6 bytes
typedef std::array<uint8_t, 6> MACAddress;

// Use #pragma pack to ensure the structure is packed without padding
#pragma pack(push, 1)
struct EthernetFrameHeader {
    MACAddress destination; // Destination MAC address
    MACAddress source;      // Source MAC address
    uint16_t type;          // EtherType field to indicate the protocol
};
#pragma pack(pop)

class EthernetFrame {
public:
    static const size_t MAX_FRAME_SIZE = 1518; // Maximum Ethernet frame size

    // Constructor to initialize the frame buffer and length
    EthernetFrame() : buffer(new uint8_t[MAX_FRAME_SIZE]), length(0) {
        std::memset(buffer.get(), 0, MAX_FRAME_SIZE);
    }

    // No custom destructor is needed because std::unique_ptr will automatically release the memory

    // Method to set the frame data
    void setFrame(const uint8_t* data, size_t len) {
        if (len > MAX_FRAME_SIZE) {
            throw std::out_of_range("Frame size exceeds maximum allowed length");
        }
        std::memcpy(buffer.get(), data, len);
        length = len;
    }

    // Method to get the Ethernet frame header
    const EthernetFrameHeader& getHeader() const {
        return *reinterpret_cast<const EthernetFrameHeader*>(buffer.get());
    }

    // Method to get the payload data of the frame
    const uint8_t* getPayload() const {
        return buffer.get() + sizeof(EthernetFrameHeader);
    }

    // Method to get the size of the payload, excluding the FCS
    size_t getPayloadSize() const {
        // sizeof(uint32_t) is used to exclude the Frame Check Sequence (FCS) from the payload size
        return length - sizeof(EthernetFrameHeader) - sizeof(uint32_t);
    }

    // Method to get the Frame Check Sequence (FCS) value
    uint32_t getFCS() const {
        return *reinterpret_cast<const uint32_t*>(buffer.get() + length - sizeof(uint32_t));
    }

    // Method to validate the FCS using CRC32
    bool validateFCS() const {
        return calculateCRC32(buffer.get(), length - sizeof(uint32_t)) == getFCS();
    }

private:
    std::unique_ptr<uint8_t[]> buffer; // Buffer to hold the frame data
    size_t length;                     // Actual length of the frame data

    // Method to calculate CRC32 for data validation
    uint32_t calculateCRC32(const uint8_t* data, size_t len) const {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < len; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc >>= 1;
                }
            }
        }
        return ~crc;
    }
};

#endif // ETHERNET_FRAME_H
