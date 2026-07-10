#include "server.h"
#include <signal.h>

static vpn_server_t* g_server = NULL;

/* Signal handler for graceful shutdown */
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        LOG_INFO("Shutdown signal received");
        if (g_server) {
            vpn_server_shutdown(g_server);
        }
        exit(0);
    }
}

/* Print usage */
void print_usage(const char* prog_name) {
    printf("Usage: %s <port>\n", prog_name);
    printf("  port - VPN server port (default: 8080)\n");
    printf("\nExample:\n");
    printf("  %s 8080\n", prog_name);
}

/* Main entry point */
int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    
    printf("VPN Server - Custom C Implementation\n");
    printf("Version 1.0\n\n");
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        port = (uint16_t)atoi(argv[1]);
    }
    
    if (port == 0) {
        LOG_ERROR("Invalid port number");
        print_usage(argv[0]);
        return 1;
    }
    
    /* Register signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Create server */
    g_server = vpn_server_create(port);
    if (!g_server) {
        LOG_ERROR("Failed to create VPN server");
        return 1;
    }
    
    /* Start server */
    if (vpn_server_start(g_server) != VPN_SUCCESS) {
        LOG_ERROR("Failed to start VPN server");
        vpn_server_destroy(g_server);
        return 1;
    }
    
    /* Run server (blocking) */
    LOG_INFO("\n=== VPN Server is running ===");
    LOG_INFO("Listening on port %u", port);
    LOG_INFO("Press Ctrl+C to stop\n");
    
    if (vpn_server_run(g_server) != VPN_SUCCESS) {
        LOG_ERROR("Server error occurred");
    }
    
    /* Cleanup */
    vpn_server_destroy(g_server);
    
    LOG_INFO("Server stopped");
    return 0;
}
