#include "protocol.h"
#include "crypto.h"
#include "socket_utils.h"

/* Pack packet into buffer */
int pack_packet(vpn_packet_t* packet, unsigned char* buffer, size_t max_len) {
    size_t pos = 0;
    size_t header_size = sizeof(vpn_packet_header_t);
    
    if (!packet || !buffer) return VPN_ERROR;
    if (max_len < header_size + MAC_TAG_SIZE) return VPN_ERROR;
    
    /* Pack header */
    buffer[pos++] = packet->header.type;
    
    /* Pack sequence (4 bytes, network byte order) */
    buffer[pos++] = (packet->header.sequence >> 24) & 0xFF;
    buffer[pos++] = (packet->header.sequence >> 16) & 0xFF;
    buffer[pos++] = (packet->header.sequence >> 8) & 0xFF;
    buffer[pos++] = packet->header.sequence & 0xFF;
    
    buffer[pos++] = packet->header.flags;
    
    /* Pack payload length (2 bytes, network byte order) */
    buffer[pos++] = (packet->header.payload_len >> 8) & 0xFF;
    buffer[pos++] = packet->header.payload_len & 0xFF;
    
    /* Pack timestamp (4 bytes, network byte order) */
    buffer[pos++] = (packet->header.timestamp >> 24) & 0xFF;
    buffer[pos++] = (packet->header.timestamp >> 16) & 0xFF;
    buffer[pos++] = (packet->header.timestamp >> 8) & 0xFF;
    buffer[pos++] = packet->header.timestamp & 0xFF;
    
    /* Add payload */
    if (packet->header.payload_len > 0) {
        if (pos + packet->header.payload_len > max_len) {
            LOG_ERROR("Buffer too small for payload");
            return VPN_ERROR;
        }
        memcpy(buffer + pos, packet->payload, packet->header.payload_len);
        pos += packet->header.payload_len;
    }
    
    /* Add MAC tag */
    if (pos + MAC_TAG_SIZE > max_len) {
        LOG_ERROR("Buffer too small for MAC tag");
        return VPN_ERROR;
    }
    memcpy(buffer + pos, packet->mac_tag, MAC_TAG_SIZE);
    pos += MAC_TAG_SIZE;
    
    return (int)pos;
}

/* Unpack packet from buffer */
int unpack_packet(const unsigned char* buffer, size_t len, vpn_packet_t* packet) {
    size_t pos = 0;
    size_t header_size = sizeof(vpn_packet_header_t);
    
    if (!buffer || !packet) return VPN_ERROR;
    if (len < header_size + MAC_TAG_SIZE) {
        LOG_ERROR("Buffer too small for packet");
        return VPN_ERROR_PROTOCOL;
    }
    
    /* Parse header */
    packet->header.type = buffer[pos++];
    
    /* Parse sequence */
    packet->header.sequence = ((uint32_t)buffer[pos] << 24) |
                              ((uint32_t)buffer[pos+1] << 16) |
                              ((uint32_t)buffer[pos+2] << 8) |
                              (uint32_t)buffer[pos+3];
    pos += 4;
    
    packet->header.flags = buffer[pos++];
    
    /* Parse payload length */
    packet->header.payload_len = ((uint16_t)buffer[pos] << 8) |
                                 (uint16_t)buffer[pos+1];
    pos += 2;
    
    /* Parse timestamp */
    packet->header.timestamp = ((uint32_t)buffer[pos] << 24) |
                               ((uint32_t)buffer[pos+1] << 16) |
                               ((uint32_t)buffer[pos+2] << 8) |
                               (uint32_t)buffer[pos+3];
    pos += 4;
    
    /* Validate sizes */
    if (pos + packet->header.payload_len + MAC_TAG_SIZE != len) {
        LOG_ERROR("Packet size mismatch");
        return VPN_ERROR_PROTOCOL;
    }
    
    /* Extract payload */
    if (packet->header.payload_len > 0) {
        if (packet->header.payload_len > MAX_PAYLOAD_SIZE) {
            LOG_ERROR("Payload too large");
            return VPN_ERROR_PROTOCOL;
        }
        memcpy(packet->payload, buffer + pos, packet->header.payload_len);
        pos += packet->header.payload_len;
    }
    
    /* Extract MAC tag */
    memcpy(packet->mac_tag, buffer + pos, MAC_TAG_SIZE);
    pos += MAC_TAG_SIZE;
    
    return VPN_SUCCESS;
}

/* Send packet over UDP or TCP */
int send_packet(int sock, vpn_packet_t* packet, int is_tcp,
                struct sockaddr_in* dest_addr) {
    unsigned char buffer[MAX_PACKET_SIZE];
    int packed_len;
    
    if (!packet) return VPN_ERROR;
    
    /* Pack packet */
    packed_len = pack_packet(packet, buffer, sizeof(buffer));
    if (packed_len < 0) {
        LOG_ERROR("Failed to pack packet");
        return VPN_ERROR;
    }
    
    /* Send based on protocol */
    if (is_tcp) {
        return send_tcp_data(sock, buffer, packed_len);
    } else {
        if (!dest_addr) return VPN_ERROR;
        return send_udp_packet(sock, buffer, packed_len, dest_addr);
    }
}

/* Receive packet from UDP or TCP */
int recv_packet(int sock, vpn_packet_t* packet, int is_tcp,
                struct sockaddr_in* src_addr) {
    unsigned char buffer[MAX_PACKET_SIZE];
    int received;
    
    if (!packet) return VPN_ERROR;
    
    /* Receive based on protocol */
    if (is_tcp) {
        received = recv_tcp_data(sock, buffer, sizeof(buffer));
    } else {
        if (!src_addr) return VPN_ERROR;
        received = recv_udp_packet(sock, buffer, sizeof(buffer), src_addr);
    }
    
    if (received <= 0) return VPN_ERROR;
    
    /* Unpack packet */
    if (unpack_packet(buffer, received, packet) < 0) {
        LOG_ERROR("Failed to unpack packet");
        return VPN_ERROR;
    }
    
    return VPN_SUCCESS;
}

/* Send data with encryption */
int send_encrypted_data(int sock, int is_tcp, struct sockaddr_in* dest_addr,
                        const unsigned char* data, int data_len,
                        session_t* session) {
    vpn_packet_t packet;
    unsigned char iv[IV_SIZE];
    unsigned char ciphertext[MAX_PAYLOAD_SIZE];
    unsigned char tag[MAC_TAG_SIZE];
    int cipher_len;
    
    if (!data || data_len <= 0 || !session) return VPN_ERROR;
    
    /* Generate random IV */
    if (generate_random_bytes(iv, IV_SIZE) < 0) {
        LOG_ERROR("Failed to generate IV");
        return VPN_ERROR;
    }
    
    /* Prepare packet header as AAD */
    unsigned char aad[12];
    aad[0] = PKT_DATA;
    aad[1] = (session->server_sequence >> 24) & 0xFF;
    aad[2] = (session->server_sequence >> 16) & 0xFF;
    aad[3] = (session->server_sequence >> 8) & 0xFF;
    aad[4] = session->server_sequence & 0xFF;
    aad[5] = 0;  /* flags */
    aad[6] = (data_len >> 8) & 0xFF;
    aad[7] = data_len & 0xFF;
    aad[8] = (uint32_t)time(NULL) >> 24;
    aad[9] = (uint32_t)time(NULL) >> 16;
    aad[10] = (uint32_t)time(NULL) >> 8;
    aad[11] = (uint32_t)time(NULL);
    
    /* Encrypt data */
    cipher_len = encrypt_aes256_gcm(data, data_len,
                                    session->session_key, KEY_SIZE,
                                    iv, IV_SIZE,
                                    aad, sizeof(aad),
                                    ciphertext, tag);
    
    if (cipher_len < 0) {
        LOG_ERROR("Encryption failed");
        return VPN_ERROR;
    }
    
    /* Create packet */
    packet.header.type = PKT_DATA;
    packet.header.sequence = session->server_sequence++;
    packet.header.flags = 0;
    packet.header.payload_len = cipher_len + IV_SIZE;
    packet.header.timestamp = (uint32_t)time(NULL);
    
    /* Copy IV and ciphertext to payload */
    memcpy(packet.payload, iv, IV_SIZE);
    memcpy(packet.payload + IV_SIZE, ciphertext, cipher_len);
    memcpy(packet.mac_tag, tag, MAC_TAG_SIZE);
    
    /* Send packet */
    return send_packet(sock, &packet, is_tcp, dest_addr);
}

/* Receive and decrypt data */
int recv_encrypted_data(int sock, int is_tcp, struct sockaddr_in* src_addr,
                        unsigned char* data, int max_len,
                        session_t* session) {
    vpn_packet_t packet;
    unsigned char iv[IV_SIZE];
    unsigned char ciphertext[MAX_PAYLOAD_SIZE];
    unsigned char plaintext[MAX_PAYLOAD_SIZE];
    int plain_len;
    
    if (!data || max_len <= 0 || !session) return VPN_ERROR;
    
    /* Receive packet */
    if (recv_packet(sock, &packet, is_tcp, src_addr) < 0) {
        LOG_ERROR("Failed to receive packet");
        return VPN_ERROR;
    }
    
    if (packet.header.type != PKT_DATA) {
        LOG_ERROR("Expected data packet, got type %d", packet.header.type);
        return VPN_ERROR;
    }
    
    /* Extract IV and ciphertext */
    if (packet.header.payload_len < IV_SIZE) {
        LOG_ERROR("Packet too small for IV");
        return VPN_ERROR;
    }
    
    memcpy(iv, packet.payload, IV_SIZE);
    int cipher_len = packet.header.payload_len - IV_SIZE;
    memcpy(ciphertext, packet.payload + IV_SIZE, cipher_len);
    
    /* Prepare AAD */
    unsigned char aad[12];
    aad[0] = packet.header.type;
    aad[1] = (packet.header.sequence >> 24) & 0xFF;
    aad[2] = (packet.header.sequence >> 16) & 0xFF;
    aad[3] = (packet.header.sequence >> 8) & 0xFF;
    aad[4] = packet.header.sequence & 0xFF;
    aad[5] = packet.header.flags;
    aad[6] = (cipher_len >> 8) & 0xFF;
    aad[7] = cipher_len & 0xFF;
    aad[8] = (packet.header.timestamp >> 24) & 0xFF;
    aad[9] = (packet.header.timestamp >> 16) & 0xFF;
    aad[10] = (packet.header.timestamp >> 8) & 0xFF;
    aad[11] = packet.header.timestamp & 0xFF;
    
    /* Decrypt data */
    plain_len = decrypt_aes256_gcm(ciphertext, cipher_len,
                                   session->session_key, KEY_SIZE,
                                   iv, IV_SIZE,
                                   aad, sizeof(aad),
                                   packet.mac_tag,
                                   plaintext);
    
    if (plain_len < 0) {
        LOG_ERROR("Decryption failed");
        return VPN_ERROR;
    }
    
    /* Copy to output buffer */
    if (plain_len > max_len) {
        LOG_ERROR("Output buffer too small");
        return VPN_ERROR;
    }
    
    memcpy(data, plaintext, plain_len);
    session->client_sequence = packet.header.sequence + 1;
    
    return plain_len;
}
