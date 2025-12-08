#include "client_cross.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
using namespace std; 
void printUsage() {
      cout << "Usage: ./client <server_ip> <server_port> <client_id> [--guardian]" <<   endl;
      cout << "Example: ./client 192.168.1.100 8080 bscs24009" <<   endl;
      cout << "         ./client 192.168.1.100 8080 bscs24009 --guardian" <<   endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 5) {
        printUsage();
        return 1;
    }

      string server_ip = argv[1];
    int port =   stoi(argv[2]);
      string client_id = argv[3];
    
    // Check if launched by guardian (skip consent prompts)
    bool skip_consent = false;
    if (argc == 5 &&   strcmp(argv[4], "--guardian") == 0) {
        skip_consent = true;
    } 

    try {
        Client client(server_ip, port, client_id, skip_consent);
        
        if (!client.start()) {
              cerr << "Failed to start client" <<   endl;
            return 1;
        }
        
        if (!skip_consent) {
              cout << "Client running. Press Enter to stop..." <<   endl;
              cin.get();
            client.stop();
        } else {
            // In guardian mode, run indefinitely until killed
              cout << "Client running under guardian protection..." <<   endl;
            while (true) {
                  this_thread::sleep_for(  chrono::seconds(1));
            }
        }
        
    } catch (const   exception& e) {
          cerr << "Error: " << e.what() <<   endl;
        return 1;
    }

    return 0;
}  