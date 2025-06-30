

print_banner() {
    echo -e "\n    ███╗   ██╗███╗   ███╗ █████╗ ██████╗     ██████╗ ██████╗ ███████╗███╗   ███╗██╗██╗   ██╗███╗   ███╗"
    echo -e "    ████╗  ██║████╗ ████║██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗██╔════╝████╗ ████║██║██║   ██║████╗ ████║"
    echo -e "    ██╔██╗ ██║██╔████╔██║███████║██████╔╝    ██████╔╝██████╔╝█████╗  ██╔████╔██║██║██║   ██║██╔████╔██║"
    echo -e "    ██║╚██╗██║██║╚██╔╝██║██╔══██║██╔═══╝     ██╔═══╝ ██╔══██╗██╔══╝  ██║╚██╔╝██║██║██║   ██║██║╚██╔╝██║"
    echo -e "    ██║ ╚████║██║ ╚═╝ ██║██║  ██║██║         ██║     ██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╔╝██║ ╚═╝ ██║"
    echo -e "    ╚═╝  ╚═══╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝ ╚═╝     ╚═╝"
    echo -e "\n    Advanced Port Scanner [Premium Version]"
    echo -e "    Developed in Bash |  https://github.com/X2X0/Nmap-Premium"
    echo -e "    $(printf '=%.0s' {1..75})"
}

get_banner() {
    local ip=$1
    local port=$2
    local timeout=$3
    timeout $timeout bash -c "</dev/tcp/$ip/$port" && echo "Banner: Open" || echo "No banner"
}

scan_port() {
    local ip=$1
    local port=$2
    local verbose=$3
    local timeout=$4

    if timeout $timeout bash -c "</dev/tcp/$ip/$port"; then
        service="Unknown"
        banner=$(get_banner $ip $port $timeout)

        if [ "$verbose" -gt 0 ]; then
            echo "[+] Port $port/tcp open - $service"
            if [ "$banner" != "No banner" ]; then
                echo "    Banner: $banner"
            fi
        fi

        echo $port $service $banner
    else
        if [ "$verbose" -gt 1 ]; then
            echo "[-] Port $port/tcp closed"
        fi
        echo $port closed ""
    fi
}

port_scan() {
    local target=$1
    local ports=$2
    local num_threads=$3
    local verbose=$4
    local timeout=$5

    echo -e "\n[*] Starting scan on $target"
    echo -e "[*] Scanning ${#ports[@]} ports with $num_threads threads"
    echo -e "[*] Start time: $(date "+%Y-%m-%d %H:%M:%S")"
    
    ip=$(dig +short $target)
    echo "[*] Target IP: $ip"

    open_ports=()
    start_time=$(date +%s)

    for port in "${ports[@]}"; do
        scan_port $ip $port $verbose $timeout &
        ((counter++))
        if ((counter % num_threads == 0)); then
            wait
        fi
    done
    wait

    end_time=$(date +%s)
    elapsed_time=$((end_time - start_time))

    echo -e "\n$(printf '=%.0s' {1..60})"
    echo -e "Scan completed in $elapsed_time seconds"
    echo -e "Found ${#open_ports[@]} open ports out of ${#ports[@]} scanned on $target ($ip)"
    echo -e "$(printf '=%.0s' {1..60})"

    if [ ${#open_ports[@]} -gt 0 ]; then
        echo -e "\nOpen ports:"
        echo -e "$(printf '-%.0s' {1..80})"
        echo -e "PORT        SERVICE             BANNER"
        echo -e "$(printf '-%.0s' {1..80})"
        for port_info in "${open_ports[@]}"; do
            port=$(echo $port_info | awk '{print $1}')
            service=$(echo $port_info | awk '{print $2}')
            banner=$(echo $port_info | awk '{print $3}')
            echo -e "$port        $service            $banner"
        done
    fi
}

main() {
    print_banner

    target="example.com"
    ports_range="1-1000"
    num_threads=100
    verbose=1
    timeout=1

    IFS=','
    ports_to_scan=()
    for range in $(echo $ports_range | tr -s '-' ','); do
        for port in $(seq ${range[0]} ${range[1]}); do
            ports_to_scan+=($port)
        done
    done

    port_scan $target $ports_to_scan $num_threads $verbose $timeout
}

main
