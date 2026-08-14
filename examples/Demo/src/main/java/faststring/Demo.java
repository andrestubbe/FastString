package faststring;

public class Demo {

    public static void main(String[] args) {
        System.out.println("==========================================================================");
        System.out.println("⚡ FastString SIMD Vector Engine — Demo");
        System.out.println("==========================================================================");
        System.out.println();

        String corpus = "LOG_TRACE [2026-08-13] PAYLOAD_ID=98471 STATUS=CRITICAL_ALERT USER=AGENT_PROT_99 TARGET=KERNEL_BUFFER_OVERFLOW";
        FastString fastStr = new FastString(corpus);

        System.out.println("Target Payload: " + corpus);
        System.out.println("Buffer Length : " + fastStr.byteLength() + " bytes (UTF-8)");
        System.out.println();

        System.out.println("🔍 [1/3] FastString Pattern Search (SIMD AVX2)");
        String[] targets = {"CRITICAL_ALERT", "KERNEL_BUFFER", "AGENT_PROT"};
        for (String target : targets) {
            long start = System.nanoTime();
            int index = fastStr.indexOf(target);
            long elapsed = System.nanoTime() - start;
            System.out.printf("   Finding '%s': Index %2d | Time: %d ns%n", target, index, elapsed);
        }

        System.out.println();
        System.out.println("✂️ [2/3] Zero-Copy Substring Slicing vs Standard Java String");
        long t0 = System.nanoTime();
        FastString subFast = fastStr.substring(10, 30);
        long t1 = System.nanoTime();

        long t2 = System.nanoTime();
        String subJava = corpus.substring(10, 30);
        long t3 = System.nanoTime();

        System.out.println("   FastString Zero-Copy Slice : \"" + subFast.toString() + "\" (" + (t1 - t0) + " ns)");
        System.out.println("   java.lang.String Heap Copy : \"" + subJava + "\" (" + (t3 - t2) + " ns)");

        System.out.println();
        System.out.println("🚀 [3/3] Search Benchmark (100,000 Iterations)");

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

        System.out.println("   FastString Time : " + durationFast + " ms");
        System.out.println("   Java String Time: " + durationJava + " ms");
        System.out.println();

        System.out.println("==========================================================================");
        System.out.println("✅ FastString Demo Completed Successfully! (Check count: " + dummyCount + ")");
        System.out.println("==========================================================================");
    }
}
