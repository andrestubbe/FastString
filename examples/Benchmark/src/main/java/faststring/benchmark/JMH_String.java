package faststring.benchmark;

import faststring.FastString;
import org.openjdk.jmh.annotations.*;
import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
@State(Scope.Thread)
@Fork(1)
@Warmup(iterations = 2, time = 1)
@Measurement(iterations = 3, time = 1)
public class JMH_String {

    private String text;
    private FastString fastString;

    @Setup
    public void setup() {
        text = "FastJava Agentic Ecosystem — High-Performance SIMD String Search and Manipulation Engine for Java.";
        fastString = new FastString(text);
    }

    @Benchmark
    public int testFastStringIndexOf() {
        return fastString.indexOf("SIMD");
    }

    @Benchmark
    public int testJavaStringIndexOf() {
        return text.indexOf("SIMD");
    }
}
