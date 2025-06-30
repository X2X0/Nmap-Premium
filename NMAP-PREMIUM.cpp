#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <iomanip>
#include <mutex>

void print_banner() {
    std::string banner = R"(
    ███╗   ██╗███╗   ███╗ █████╗ ██████╗     ██████╗ ██████╗ ███████╗███╗   ███╗██╗██╗   ██╗███╗   ███╗
    ████╗  ██║████╗ ████║██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗██╔════╝████╗ ████║██║██║   ██║████╗ ████║
    ██╔██╗ ██║██╔████╔██║███████║██████╔╝    ██████╔╝██████╔╝█████╗  ██╔████╔██║██║██║   ██║██╔████╔██║
    ██║╚██╗██║██║╚██╔╝██║██╔══██║██╔═══╝     ██╔═══╝ ██╔══██╗██╔══╝  ██║╚██╔╝██║██║██║   ██║██║╚██╔╝██║
    ██║ ╚████║██║ ╚═╝ ██║██║  ██║██║         ██║     ██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╔╝██║ ╚═╝ ██║
    ╚═╝  ╚═══╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝ ╚═╝     ╚═╝
    )";
    std::cout << banner << "\n";
    std::cout << "    Advanced Port Scanner [Premium Version]\n";
    std::cout << "    Developed in C++ |  https://github.com/X2X0/Nmap-Premium\n";
    std::cout << "    " << std::string(75, '=') << "\n";
}

std::string get_banner(const std::string& ip, int port, int timeout = 1) {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket(io_context);
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip), port);
        socket.connect(endpoint);
        std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
        asio::write(socket, asio::buffer(request));
        char buffer[1024];
        size_t length = socket.read_some(asio::buffer(buffer));
        return std::string(buffer, length);
    } catch (...) {
        return "No banner";
    }
}

std::tuple<int, bool, std::string, std::string> scan_port(const std::string& ip, int port, bool verbose = false, int timeout = 1) {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket(io_context);
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip), port);
        asio::system_timer timer(io_context);
        timer.expires_from_now(std::chrono::seconds(timeout));
        socket.async_connect(endpoint, [](const std::error_code& error) {
            if (error) {
                throw std::runtime_error("Connection failed");
            }
        });
        io_context.run();

        std::string service = "Unknown";
        std::string banner = get_banner(ip, port, timeout);
        if (verbose) {
            std::cout << "[+] Port " << port << "/tcp open - " << service << "\n";
            if (banner != "No banner") {
                std::cout << "    Banner: " << banner << "\n";
            }
        }
        return std::make_tuple(port, true, service, banner);
    } catch (...) {
        if (verbose) {
            std::cout << "[-] Port " << port << "/tcp closed\n";
        }
        return std::make_tuple(port, false, "", "");
    }
}

void port_scan(const std::string& target, const std::vector<int>& ports, int num_threads = 100, bool verbose = false, int timeout = 1) {
    std::cout << "\n[*] Starting scan on " << target << "\n";
    std::cout << "[*] Scanning " << ports.size() << " ports with " << num_threads << " threads\n";
    std::cout << "[*] Start time: " << std::chrono::system_clock::now() << "\n";

    std::string ip;
    try {
        ip = asio::ip::address::from_string(target).to_string();
    } catch (...) {
        std::cout << "\n[!] Hostname " << target << " could not be resolved\n";
        return;
    }

    std::vector<std::tuple<int, bool, std::string, std::string>> open_ports;
    std::mutex open_ports_mutex;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int port : ports) {
        threads.push_back(std::thread([&, port]() {
            auto result = scan_port(ip, port, verbose, timeout);
            if (std::get<1>(result)) {
                std::lock_guard<std::mutex> lock(open_ports_mutex);
                open_ports.push_back(result);
            }
        }));
    }
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count();
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Scan completed in " << elapsed_time << " seconds\n";
    std::cout << "Found " << open_ports.size() << " open ports out of " << ports.size() << " scanned on " << target << " (" << ip << ")\n";
    std::cout << std::string(60, '=') << "\n";

    if (!open_ports.empty()) {
        std::cout << "\nOpen ports:\n";
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::left << std::setw(10) << "PORT" << std::setw(20) << "SERVICE" << "BANNER\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto& result : open_ports) {
            std::string banner_short = std::get<3>(result).substr(0, 50) + "...";
            std::cout << std::setw(10) << std::get<0>(result) << std::setw(20) << std::get<2>(result) << banner_short << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    print_banner();

    // Prompt user for target and port range
    std::string target;
    std::string ports_range;
    int num_threads = 100;
    bool verbose = false;
    int timeout = 1;

    std::cout << "Enter target IP or hostname: ";
    std::getline(std::cin, target);

    std::cout << "Enter port range (e.g., 1-1000): ";
    std::getline(std::cin, ports_range);

    std::cout << "Enter number of threads [default 100]: ";
    std::string threads_input;
    std::getline(std::cin, threads_input);
    if (!threads_input.empty()) {
        num_threads = std::stoi(threads_input);
    }

    std::cout << "Enable verbose output? (y/N): ";
    std::string verbose_input;
    std::getline(std::cin, verbose_input);
    if (!verbose_input.empty() && (verbose_input[0] == 'y' || verbose_input[0] == 'Y')) {
        verbose = true;
    }

    std::cout << "Set timeout in seconds [default 1]: ";
    std::string timeout_input;
    std::getline(std::cin, timeout_input);
    if (!timeout_input.empty()) {
        timeout = std::stoi(timeout_input);
    }

    // Parse port range into vector<int>
    std::vector<int> ports_to_scan;
    size_t dash_pos = ports_range.find('-');
    if (dash_pos != std::string::npos) {
        int start = std::stoi(ports_range.substr(0, dash_pos));
        int end = std::stoi(ports_range.substr(dash_pos + 1));
        for (int p = start; p <= end; ++p) {
            ports_to_scan.push_back(p);
        }
    } else {
        ports_to_scan.push_back(std::stoi(ports_range));
    }

    port_scan(target, ports_to_scan, num_threads, verbose, timeout);

    return 0;
}
