package faststring;

import java.util.Random;

public class Demo {

    private static final String ANSI_RESET  = "\u001B[0m";
    private static final String ANSI_CYAN   = "\u001B[36m";
    private static final String ANSI_GREEN  = "\u001B[32m";
    private static final String ANSI_YELLOW = "\u001B[33m";
    private static final String ANSI_MAGENTA= "\u001B[35m";
    private static final String ANSI_BOLD   = "\u001B[1m";

    public static void main(String[] args) throws Exception {
        System.out.println(ANSI_CYAN + ANSI_BOLD + "==========================================================================" + ANSI_RESET);
        System.out.println(ANSI_GREEN + ANSI_BOLD + "  ⚡ FastString SIMD Vector Engine — Visual Performance Demo ⚡" + ANSI_RESET);
        System.out.println(ANSI_CYAN + ANSI_BOLD + "==========================================================================" + ANSI_RESET);
        System.out.println();

        System.out.println(ANSI_YELLOW + ">>> Initializing AVX2 256-bit Vector String Search..." + ANSI_RESET);
        Thread.sleep(300);

        String corpus = "LOG_TRACE [2026-08-13] PAYLOAD_ID=98471 STATUS=CRITICAL_ALERT USER=AGENT_PROT_99 TARGET=KERNEL_BUFFER_OVERFLOW";
        FastString fastStr = new FastString(corpus);

        System.out.println(ANSI_BOLD + "Target Payload: " + ANSI_RESET + corpus);
        System.out.println(ANSI_BOLD + "Buffer Length : " + ANSI_RESET + fastStr.byteLength() + " bytes (Zero-Copy UTF-8 Allocation)");
        System.out.println();

        System.out.println(ANSI_MAGENTA + ANSI_BOLD + "[1/3] Real-Time SIMD Pattern Search (AVX2 32-Byte Parallel Scans)" + ANSI_RESET);
        String[] targets = {"CRITICAL_ALERT", "KERNEL_BUFFER", "AGENT_PROT"};
        
        for (String target : targets) {
            long start = System.nanoTime();
            int index = fastStr.indexOf(target);
            long elapsed = System.nanoTime() - start;

            // Visual scan bar
            StringBuilder bar = new StringBuilder("[");
            int fill = (index * 30) / corpus.length();
            for (int i = 0; i < 30; i++) {
                if (i == fill) bar.append("🎯");
                else if (i < fill) bar.append("=");
                else bar.append(" ");
            }
            bar.append("]");

            System.out.printf("   Finding '%s%s%s': %s Index %2d | Time: %s%d ns%s%n",
                    ANSI_GREEN, target, ANSI_RESET,
                    bar.toString(), index, ANSI_YELLOW, elapsed, ANSI_RESET);
            Thread.sleep(200);
        }

        System.out.println();
        System.out.println(ANSI_MAGENTA + ANSI_BOLD + "[2/3] Zero-Copy Substring Slicing vs Standard Java String" + ANSI_RESET);
        long t0 = System.nanoTime();
        FastString subFast = fastStr.substring(10, 30);
        long t1 = System.nanoTime();

        long t2 = System.nanoTime();
        String subJava = corpus.substring(10, 30);
        long t3 = System.nanoTime();

        System.out.println("   FastString Zero-Copy Slice : \"" + subFast.toString() + "\" (" + (t1 - t0) + " ns, 0 bytes allocated)");
        System.out.println("   java.lang.String Heap Copy : \"" + subJava + "\" (" + (t3 - t2) + " ns, object created)");

        System.out.println();
        System.out.println(ANSI_MAGENTA + ANSI_BOLD + "[3/3] High-Throughput Stress Comparison (100,000 Iterations)" + ANSI_RESET);

        int iterations = 100000;
        long startFast = System.currentTimeMillis();
        int dummyCount = 0;
        for (int i = 0; i < iterations; i++) {
            dummyCount += fastStr.indexOf("CRITICAL_ALERT");
        }
        long durationFast = System.currentTimeMillis() - startFast;

        long startJava = System.currentTimeMillis();
        for (int i = 0; i < iterations; i++) {
            dummyCount += corpus.indexOf("CRITICAL_ALERT");
        }
        long durationJava = System.currentTimeMillis() - startJava;

        System.out.println("   ⚡ FastString SIMD Search Time : " + ANSI_GREEN + durationFast + " ms" + ANSI_RESET);
        System.out.println("   🐢 Standard Java String Time  : " + ANSI_YELLOW + durationJava + " ms" + ANSI_RESET);
        System.out.println();

        System.out.println(ANSI_CYAN + ANSI_BOLD + "==========================================================================" + ANSI_RESET);
        System.out.println(ANSI_GREEN + ANSI_BOLD + "  ✅ FastString Demo Completed Successfully! (Check count: " + dummyCount + ")" + ANSI_RESET);
        System.out.println(ANSI_CYAN + ANSI_BOLD + "==========================================================================" + ANSI_RESET);
    }
}
