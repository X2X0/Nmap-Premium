import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;
import java.text.SimpleDateFormat;

public class PortScanner {

    public static void printBanner() {
        String banner = """
        ███╗   ██╗███╗   ███╗ █████╗ ██████╗     ██████╗ ██████╗ ███████╗███╗   ███╗██╗██╗   ██╗███╗   ███╗
        ████╗  ██║████╗ ████║██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗██╔════╝████╗ ████║██║██║   ██║████╗ ████║
        ██╔██╗ ██║██╔████╔██║███████║██████╔╝    ██████╔╝██████╔╝█████╗  ██╔████╔██║██║██║   ██║██╔████╔██║
        ██║╚██╗██║██║╚██╔╝██║██╔══██║██╔═══╝     ██╔═══╝ ██╔══██╗██╔══╝  ██║╚██╔╝██║██║██║   ██║██║╚██╔╝██║
        ██║ ╚████║██║ ╚═╝ ██║██║  ██║██║         ██║     ██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╔╝██║ ╚═╝ ██║
        ╚═╝  ╚═══╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝         ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝ ╚═╝     ╚═╝
        """;
        System.out.println(banner);
        System.out.println("    Advanced Port Scanner [Premium Version]");
        System.out.println("    Developed in Java  |  https://github.com/X2X0/Nmap-Premium");
        System.out.println("    " + "=".repeat(75));
    }

    public static String getBanner(String ip, int port, int timeout) {
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(ip, port), timeout);
            socket.getOutputStream().write("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n".getBytes());
            byte[] banner = new byte[1024];
            int length = socket.getInputStream().read(banner);
            return new String(banner, 0, length).trim();
        } catch (IOException e) {
            return "No banner";
        }
    }

    public static class ScanResult {
        int port;
        boolean isOpen;
        String service;
        String banner;

        public ScanResult(int port, boolean isOpen, String service, String banner) {
            this.port = port;
            this.isOpen = isOpen;
            this.service = service;
            this.banner = banner;
        }
    }

    public static ScanResult scanPort(String ip, int port, boolean verbose, int timeout) {
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(ip, port), timeout);
            String service = "Unknown";
            try {
                service = InetAddress.getByName("localhost").getCanonicalHostName();
            } catch (UnknownHostException ignored) {}

            String banner = getBanner(ip, port, timeout);
            if (verbose) {
                System.out.println("[+] Port " + port + "/tcp open - " + service);
                if (!"No banner".equals(banner)) {
                    System.out.println("    Banner: " + banner);
                }
            }
            return new ScanResult(port, true, service, banner);
        } catch (IOException e) {
            if (verbose) {
                System.out.println("[-] Port " + port + "/tcp closed");
            }
            return new ScanResult(port, false, "", "");
        }
    }

    public static List<ScanResult> portScan(String target, List<Integer> ports, int numThreads, boolean verbose, int timeout) {
        System.out.println("\n[*] Starting scan on " + target);
        System.out.println("[*] Scanning " + ports.size() + " ports with " + numThreads + " threads");
        System.out.println("[*] Start time: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));

        String ip;
        try {
            ip = InetAddress.getByName(target).getHostAddress();
        } catch (UnknownHostException e) {
            System.out.println("\n[!] Hostname " + target + " could not be resolved");
            return new ArrayList<>();
        }

        System.out.println("[*] Target IP: " + ip + "\n");

        List<ScanResult> openPorts = new ArrayList<>();
        long startTime = System.nanoTime();

        ExecutorService executor = Executors.newFixedThreadPool(numThreads);
        List<Future<ScanResult>> futures = new ArrayList<>();
        for (int port : ports) {
            futures.add(executor.submit(() -> scanPort(ip, port, verbose, timeout)));
        }

        for (Future<ScanResult> future : futures) {
            try {
                ScanResult result = future.get();
                if (result.isOpen) {
                    openPorts.add(result);
                }
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
        executor.shutdown();

        long elapsedTime = (System.nanoTime() - startTime) / 1_000_000;
        System.out.println("\n" + "=".repeat(60));
        System.out.println("Scan completed in " + elapsedTime + " ms");
        System.out.println("Found " + openPorts.size() + " open ports out of " + ports.size() + " scanned on " + target);
        System.out.println("=" .repeat(60));

        if (!openPorts.isEmpty()) {
            System.out.println("\nOpen ports:");
            System.out.println("-".repeat(80));
            System.out.printf("%-10s%-20s%-50s%n", "PORT", "SERVICE", "BANNER");
            System.out.println("-".repeat(80));
            for (ScanResult result : openPorts) {
                String bannerShort = result.banner.length() > 50 ? result.banner.substring(0, 50) + "..." : result.banner;
                System.out.printf("%-10d%-20s%-50s%n", result.port, result.service, bannerShort);
            }
        }

        return openPorts;
    }

    public static void main(String[] args) {
        printBanner();

        String target = args[0];
        String portRange = args[1];
        int threads = Integer.parseInt(args[2]);
        boolean verbose = args.length > 3 && args[3].equals("-v");
        int timeout = 1000;

        List<Integer> portsToScan = new ArrayList<>();
        for (String port : portRange.split(",")) {
            if (port.contains("-")) {
                String[] range = port.split("-");
                int start = Integer.parseInt(range[0]);
                int end = Integer.parseInt(range[1]);
                for (int i = start; i <= end; i++) {
                    portsToScan.add(i);
                }
            } else {
                portsToScan.add(Integer.parseInt(port));
            }
        }

        portsToScan = new ArrayList<>(new HashSet<>(portsToScan));
        Collections.sort(portsToScan);

        List<ScanResult> openPorts = portScan(target, portsToScan, threads, verbose, timeout);

        if (args.length > 4) {
            String outputFile = args[4];
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputFile))) {
                writer.write("Scan results for " + target + " (" + InetAddress.getByName(target).getHostAddress() + ")\n");
                writer.write("Date: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()) + "\n");
                writer.write("Ports scanned: " + portRange + "\n\n");
                writer.write(String.format("%-10s%-20s%-50s%n", "PORT", "SERVICE", "BANNER"));
                writer.write("-".repeat(80) + "\n");
                for (ScanResult result : openPorts) {
                    writer.write(String.format("%-10d%-20s%-50s%n", result.port, result.service, result.banner));
                }
                System.out.println("\n[+] Results saved to " + outputFile);
            } catch (IOException e) {
                System.out.println("\n[!] Error saving results: " + e.getMessage());
            }
        }
    }
                             }
