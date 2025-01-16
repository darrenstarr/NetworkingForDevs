#ifndef ETHERNET_FRAME_H
#define ETHERNET_FRAME_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <array>
#include <vector>
#include <memory>

/**
 * @brief Define a MAC address as an array of 6 bytes.
 * @link https://en.wikipedia.org/wiki/MAC_address
 */
typedef std::array<uint8_t, 6> MACAddress;

/**
 * @brief Ethernet frame header structure, packed to avoid padding.
 * @link https://en.wikipedia.org/wiki/Ethernet_frame
 */
#pragma pack(push, 1)
struct EthernetFrameHeader {
    MACAddress destination; // Destination MAC address
    MACAddress source;      // Source MAC address
};
#pragma pack(pop)

/**
 * @brief Class representing a generic Ethernet payload.
 * @link https://en.wikipedia.org/wiki/EtherType
 */
class EthernetPayload {
public:
    /**
     * @brief Constructor to initialize the payload with the given type.
     * @param type EtherType for the payload.
     */
    explicit EthernetPayload(uint16_t type) : type(type) {}

    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~EthernetPayload() = default;

    /**
     * @brief Get the EtherType of the payload.
     * @return EtherType of the payload.
     */
    uint16_t getType() const {
        return type;
    }

    /**
     * @brief Get the payload data.
     * @return Reference to the vector containing the payload data.
     */
    virtual const std::vector<uint8_t>& getData() const {
        return data;
    }

    /**
     * @brief Set the payload data.
     * @param data Vector containing the payload data.
     */
    virtual void setData(const std::vector<uint8_t>& data) {
        this->data = data;
    }

private:
    uint16_t type;                // EtherType for the payload

protected:
    std::vector<uint8_t> data;    // Payload data
};

/**
 * @brief Class representing an 802.1Q tag (VLAN tag) payload.
 * @link https://en.wikipedia.org/wiki/IEEE_802.1Q
 */
class Dot1qTag : public EthernetPayload {
public:
    Dot1qTag() : EthernetPayload(0x8100) {}

    /**
     * @brief Get the Tag Protocol Identifier (TPID).
     * @return TPID value.
     */
    uint16_t getTagProtocolIdentifier() const {
        return *reinterpret_cast<const uint16_t*>(data.data());
    }

    /**
     * @brief Get the Tag Control Information (TCI).
     * @return TCI value.
     */
    uint16_t getTagControlInformation() const {
        return *reinterpret_cast<const uint16_t*>(data.data() + 2);
    }

    /**
     * @brief Get the Priority Code Point (PCP).
     * @return PCP value.
     */
    uint8_t getPriorityCodePoint() const {
        return (getTagControlInformation() >> 13) & 0x07;
    }

    /**
     * @brief Get the Drop Eligible Indicator (DEI).
     * @return DEI value.
     */
    bool getDropEligibleIndicator() const {
        return (getTagControlInformation() >> 12) & 0x01;
    }

    /**
     * @brief Get the VLAN Identifier (VID).
     * @return VID value.
     */
    uint16_t getVLANIdentifier() const {
        return getTagControlInformation() & 0x0FFF;
    }

    /**
     * @brief Set the Tag Protocol Identifier (TPID).
     * @param tpid TPID value.
     */
    void setTagProtocolIdentifier(uint16_t tpid) {
        *reinterpret_cast<uint16_t*>(data.data()) = tpid;
    }

    /**
     * @brief Set the Tag Control Information (TCI).
     * @param tci TCI value.
     */
    void setTagControlInformation(uint16_t tci) {
        *reinterpret_cast<uint16_t*>(data.data() + 2) = tci;
    }

    /**
     * @brief Set the Priority Code Point (PCP).
     * @param pcp PCP value.
     */
    void setPriorityCodePoint(uint8_t pcp) {
        setTagControlInformation((getTagControlInformation() & 0x1FFF) | (pcp << 13));
    }

    /**
     * @brief Set the Drop Eligible Indicator (DEI).
     * @param dei DEI value.
     */
    void setDropEligibleIndicator(bool dei) {
        setTagControlInformation((getTagControlInformation() & 0xEFFF) | (dei << 12));
    }

    /**
     * @brief Set the VLAN Identifier (VID).
     * @param vid VID value.
     */
    void setVLANIdentifier(uint16_t vid) {
        setTagControlInformation((getTagControlInformation() & 0xF000) | (vid & 0x0FFF));
    }

    /**
     * @brief Set the payload data, ensuring it is the correct size for a Dot1qTag.
     * @param data Vector containing the payload data.
     * @throws std::out_of_range if the data size is not 4 bytes.
     */
    void setData(const std::vector<uint8_t>& data) override {
        if (data.size() != 4) {
            throw std::out_of_range("Dot1qTag data size must be 4 bytes");
        }
        EthernetPayload::setData(data);
    }
};

/**
 * @brief Class representing an Ethernet frame.
 * @link https://en.wikipedia.org/wiki/Ethernet_frame
 */
class EthernetFrame {
public:
    static const size_t MAX_FRAME_SIZE = 1518; // Maximum Ethernet frame size

    /**
     * @brief Constructor to initialize the frame buffer and length.
     */
    EthernetFrame() : buffer(new uint8_t[MAX_FRAME_SIZE]), length(0) {
        std::memset(buffer.get(), 0, MAX_FRAME_SIZE);
    }

    // No custom destructor is needed because std::unique_ptr will automatically release the memory

    /**
     * @brief Set the frame data and parse it into header and payloads.
     * @param data Pointer to the frame data.
     * @param len Length of the frame data.
     * @throws std::out_of_range if the frame size exceeds the maximum allowed length.
     */
    void setFrame(const uint8_t* data, size_t len) {
        if (len > MAX_FRAME_SIZE) {
            throw std::out_of_range("Frame size exceeds maximum allowed length");
        }
        std::memcpy(buffer.get(), data, len);
        length = len;
        parseFrame();
    }

    /**
     * @brief Get the Ethernet frame header.
     * @return Reference to the Ethernet frame header.
     */
    const EthernetFrameHeader& getHeader() const {
        return *reinterpret_cast<const EthernetFrameHeader*>(buffer.get());
    }

    /**
     * @brief Get the payloads of the Ethernet frame.
     * @return Vector of unique pointers to the payloads.
     */
    const std::vector<std::unique_ptr<EthernetPayload>>& getPayloads() const {
        return payloads;
    }

    /**
     * @brief Get the Frame Check Sequence (FCS) value.
     * @return FCS value.
     */
    uint32_t getFCS() const {
        return *reinterpret_cast<const uint32_t*>(buffer.get() + length - sizeof(uint32_t));
    }

    /**
     * @brief Validate the Frame Check Sequence (FCS) using CRC32.
     * @return True if the FCS is valid, false otherwise.
     */
    bool validateFCS() const {
        return calculateCRC32(buffer.get(), length - sizeof(uint32_t)) == getFCS();
    }

private:
    std::unique_ptr<uint8_t[]> buffer; // Buffer to hold the frame data
    size_t length;                     // Actual length of the frame data
    std::vector<std::unique_ptr<EthernetPayload>> payloads; // Vector to hold the payloads

    /**
     * @brief Parse the frame data into header and payloads.
     * @throws std::out_of_range if an invalid payload length is encountered.
     */
    void parseFrame() {
        payloads.clear();
        size_t offset = sizeof(EthernetFrameHeader);
        while (offset < length - sizeof(uint32_t)) {
            // Check if the offset is within bounds for the payload length
            if (offset + sizeof(uint16_t) > length - sizeof(uint32_t)) {
                throw std::out_of_range("Invalid payload length");
            }

            // Read the EtherType to determine the payload type
            uint16_t type = *reinterpret_cast<const uint16_t*>(buffer.get() + offset);
            offset += sizeof(uint16_t);
            size_t payloadLength = 0;

            // Check if the type matches Dot1qTag (VLAN tag)
            if (type == 0x8100) {
                auto payload = std::make_unique<Dot1qTag>();
                payload->setData(std::vector<uint8_t>(buffer.get() + offset, buffer.get() + offset + 4));
                offset += 4;
                payloads.push_back(std::move(payload));
            } else {
                payloadLength = length - offset - sizeof(uint32_t);
                auto payload = std::make_unique<EthernetPayload>(type);
                payload->setData(std::vector<uint8_t>(buffer.get() + offset, buffer.get() + offset + payloadLength));
                offset += payloadLength;
                payloads.push_back(std::move(payload));
            }
        }
    }

    /**
     * @brief Calculate CRC32 for data validation.
     * @param data Pointer to the data.
     * @param len Length of the data.
     * @return CRC32 value.
     */
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
