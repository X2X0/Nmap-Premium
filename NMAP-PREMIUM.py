#!/usr/bin/env python3
# by X2X0/X4X0
import socket
import sys
import threading
import time
import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime

def print_banner():
    banner = """
    ███╗   ██╗███╗   ███╗ █████╗ ██████╗     ██████╗ ██████╗ ███████╗███╗   ███╗██╗██╗   ██╗███╗   ███╗
    ████╗  ██║████╗ ████║██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗██╔════╝████╗ ████║██║██║   ██║████╗ ████║
    ██╔██╗ ██║██╔████╔██║███████║██████╔╝    ██████╔╝██████╔╝█████╗  ██╔████╔██║██║██║   ██║██╔████╔██║
    ██║╚██╗██║██║╚██╔╝██║██╔══██║██╔═══╝     ██╔═══╝ ██╔══██╗██╔══╝  ██║╚██╔╝██║██║██║   ██║██║╚██╔╝██║
    ██║ ╚████║██║ ╚═╝ ██║██║  ██║██║         ██║     ██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╔╝██║ ╚═╝ ██║
    ╚═╝  ╚═══╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝ ╚═╝     ╚═╝
                                                                                        
    """
    print(banner)
    print("    Advanced Port Scanner [Premium Version]")
    print("    Developed in Python  |  https://github.com/X2X0/Nmap-Premium")
    print("    " + "=" * 75)

def get_banner(ip, port, timeout=1):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(timeout)
            s.connect((ip, port))
            s.send(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
            banner = s.recv(1024).strip()
            return banner.decode('utf-8', errors='ignore') if banner else "No banner"
    except:
        return "No banner"

def scan_port(ip, port, verbose=False, timeout=1):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(timeout)
            result = s.connect_ex((ip, port))
            if result == 0:
                service = "Unknown"
                try:
                    service = socket.getservbyport(port)
                except:
                    pass
                
                banner = get_banner(ip, port, timeout)
                if verbose:
                    print(f"[+] Port {port}/tcp open - {service}")
                    if banner != "No banner":
                        print(f"    Banner: {banner}")
                return port, True, service, banner
            else:
                if verbose > 1:
                    print(f"[-] Port {port}/tcp closed")
                return port, False, "", ""
    except KeyboardInterrupt:
        print("\n[!] Scan interrupted by user")
        sys.exit(1)
    except socket.gaierror:
        print(f"\n[!] Hostname {ip} could not be resolved")
        sys.exit(1)
    except socket.error:
        print(f"\n[!] Could not connect to server {ip}")
        sys.exit(1)

def port_scan(target, ports, num_threads=100, verbose=False, timeout=1):
    print(f"\n[*] Starting scan on {target}")
    print(f"[*] Scanning {len(ports)} ports with {num_threads} threads")
    print(f"[*] Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    try:
        ip = socket.gethostbyname(target)
    except socket.gaierror:
        print(f"\n[!] Hostname {target} could not be resolved")
        sys.exit(1)
    
    print(f"[*] Target IP: {ip}\n")
    
    open_ports = []
    start_time = time.time()
    
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        results = list(executor.map(
            lambda port: scan_port(ip, port, verbose, timeout), 
            ports
        ))
    
    for port, is_open, service, banner in results:
        if is_open:
            open_ports.append((port, service, banner))
    
    elapsed_time = time.time() - start_time
    
    print("\n" + "=" * 60)
    print(f"Scan completed in {elapsed_time:.2f} seconds")
    print(f"Found {len(open_ports)} open ports out of {len(ports)} scanned on {target} ({ip})")
    print("=" * 60)
    
    if open_ports:
        print("\nOpen ports:")
        print("-" * 80)
        print(f"{'PORT':<10}{'SERVICE':<20}{'BANNER'}")
        print("-" * 80)
        for port, service, banner in sorted(open_ports):
            banner_short = banner[:50] + "..." if len(banner) > 50 else banner
            print(f"{port:<10}{service:<20}{banner_short}")
    
    return open_ports

def main():
    print_banner()
    
    parser = argparse.ArgumentParser(description='Advanced Port Scanner in Python')
    parser.add_argument('-t', '--target', required=True, help='Target IP or domain')
    parser.add_argument('-p', '--ports', default='1-1000', help='Ports to scan (e.g.: 22,80,443 or 1-1000)')
    parser.add_argument('-T', '--threads', type=int, default=100, help='Number of threads (default: 100)')
    parser.add_argument('-v', '--verbose', action='count', default=0, help='Verbosity level')
    parser.add_argument('-o', '--output', help='Output file to save results')
    parser.add_argument('--timeout', type=float, default=1, help='Timeout in seconds (default: 1)')
    
    args = parser.parse_args()
    
    ports_to_scan = []
    for port_range in args.ports.split(','):
        if '-' in port_range:
            start, end = map(int, port_range.split('-'))
            ports_to_scan.extend(range(start, end + 1))
        else:
            ports_to_scan.append(int(port_range))
    
    ports_to_scan = sorted(list(set(ports_to_scan)))
    
    open_ports = port_scan(
        args.target, 
        ports_to_scan, 
        num_threads=args.threads, 
        verbose=args.verbose,
        timeout=args.timeout
    )
    
    if args.output and open_ports:
        try:
            with open(args.output, 'w') as f:
                f.write(f"Scan results for {args.target} ({socket.gethostbyname(args.target)})\n")
                f.write(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"Ports scanned: {args.ports}\n\n")
                f.write(f"{'PORT':<10}{'SERVICE':<20}{'BANNER'}\n")
                f.write("-" * 80 + "\n")
                for port, service, banner in sorted(open_ports):
                    f.write(f"{port:<10}{service:<20}{banner}\n")
            print(f"\n[+] Results saved to {args.output}")
        except Exception as e:
            print(f"\n[!] Error saving results: {e}")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[!] Scan interrupted by user")
        sys.exit(1)
