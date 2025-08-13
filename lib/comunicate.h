// comunicate.h
#ifndef COMUNICATE_H
#define COMUNICATE_H

#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#define FIFO_PATH "/tmp/stepper_fifo"
#define BUF_SIZE 128

class FifoServer {
private:
    int fd;
    int lastX;
    int lastY;
    bool connected;
    std::string buffer; // Buffer lưu dữ liệu còn dở

    void ensureConnected() {
        if (connected) return;
        
        fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            std::cerr << "Error opening FIFO: " << strerror(errno) << std::endl;
            connected = false;
        } else {
            connected = true;
        }
    }

    void processLines() {
        // Tìm vị trí xuống dòng cuối cùng
        size_t last_newline = buffer.find_last_of('\n');
        
        if (last_newline == std::string::npos) return; // Không có dòng hoàn chỉnh
        
        // Tách phần dữ liệu có thể xử lý
        std::string complete_data = buffer.substr(0, last_newline);
        buffer = buffer.substr(last_newline + 1); // Giữ lại phần dư
        
        // Tách từng dòng
        std::istringstream iss(complete_data);
        std::string line;
        while (std::getline(iss, line)) {
            processSingleLine(line);
        }
    }

    void processSingleLine(const std::string& line) {
        std::istringstream iss(line);
        int x, y;
        if (!(iss >> x >> y)) {
            std::cerr << "Invalid line format: " << line << std::endl;
            return;
        }
        
        lastX = x;
        lastY = y;
    }

public:
    FifoServer() : lastX(0), lastY(0), connected(false) {
        ensureConnected();
    }

    ~FifoServer() {
        if (connected) close(fd);
    }

    void run(int &x, int &y) {
        ensureConnected();
        if (!connected) {
            x = lastX;
            y = lastY;
            return;
        }

        char temp_buffer[BUF_SIZE];
        ssize_t bytes_read = read(fd, temp_buffer, sizeof(temp_buffer));
        
        if (bytes_read > 0) {
            buffer.append(temp_buffer, bytes_read);
            processLines();
        } else if (bytes_read == -1 && errno != EAGAIN) {
            std::cerr << "FIFO read error: " << strerror(errno) << std::endl;
            close(fd);
            connected = false;
        }
        
        x = lastX;
        y = lastY;
    }
};

#endif // COMUNICATE_H
