#include "server_cross.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std; 

void printUsage() {
      cout << "Usage: ./server <port>" <<   endl;
      cout << "Example: ./server 8080" <<   endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage();
        return 1;
    }

    int port =   stoi(argv[1]);

    try {
        Server server(port);
        
        if (!server.start()) {
              cerr << "Failed to start server" <<   endl;
            return 1;
        }
        
          cout << "Server running. Press Enter to stop..." <<   endl;
          cin.get();
        server.stop();
        
    } catch (const   exception& e) {
          cerr << "Error: " << e.what() <<   endl;
        return 1;
    }

    return 0;
}